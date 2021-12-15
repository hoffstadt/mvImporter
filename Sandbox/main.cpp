#include <imgui.h>
#include <implot.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>
#include "mvLights.h"
#include "mvSkybox.h"
#include "mvShadows.h"
#include "mvOffscreenPass.h"
#include "mvRenderer.h"
#include "mvImporter.h"
#include "mvTimer.h"

// TODO: make most of these runtime options
static const char* gltfAssetDirectory0 = "../../data/glTF-Sample-Models/2.0/Sponza/glTF/";
static const char* gltfModel0 = "../../data/glTF-Sample-Models/2.0/Sponza/glTF/Sponza.gltf";
static f32         shadowWidth = 95.0f;
static int         initialWidth = 1850;
static int         initialHeight = 900;
static ImVec2      oldContentRegion = ImVec2(500, 500);

int main()
{
    mvCreateContext();
    GContext->IO.shaderDirectory = "../../Sandbox/shaders/";
    GContext->IO.resourceDirectory = "../../Resources/";

    mvViewport* window = initialize_viewport(initialWidth, initialHeight);
    setup_graphics(*window);
    ID3D11DeviceContext* ctx = GContext->graphics.imDeviceContext.Get();

    // setup imgui
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui_ImplWin32_Init(window->hWnd);
    ImGui_ImplDX11_Init(GContext->graphics.device.Get(), GContext->graphics.imDeviceContext.Get());

    mvAssetManager am{};
    mvInitializeAssetManager(&am);
    Renderer::mvSetupCommonAssets(am);

    // assets & meshes
    mvGLTFModel gltfmodel0 = mvLoadGLTF(gltfAssetDirectory0, gltfModel0);
    load_gltf_assets(am, gltfmodel0);
    mvCleanupGLTF(gltfmodel0);

    // main camera
    mvCamera camera = create_perspective_camera({ -13.5f, 6.0f, 3.5f }, (f32)M_PI_4, 1.0f, 0.1f, 400.0f);

    // lights
    mvPointLight pointlight = create_point_light(am);
    mvDirectionalLight directionalLight = create_directional_light(am);

    // passes
    mvOffscreenPass offscreen = mvOffscreenPass(500.0f, 500.0f);
    mvDirectionalShadowPass directionalShadowMap = mvDirectionalShadowPass(am, 4096, shadowWidth);
    mvOmniShadowPass omniShadowMap = mvOmniShadowPass(am, 2048);
    mvSkybox skybox = create_skybox(am);
    
    omniShadowMap.info.view = create_lookat_view(pointlight.camera);

    GlobalInfo globalInfo{};
    mvConstBuffer globalInfoBuffer = create_const_buffer(&globalInfo, sizeof(GlobalInfo));
    register_asset(&am, "global_constant_buffer", globalInfoBuffer);

    mvTimer timer;

    while (true)
    {
        const auto dt = timer.mark() * 1.0f;

        static f32 scale0 = 1.0f;
        static mvVec3 translate0 = { 0.0f, 0.0f, 0.0f };
        mvVec3 scaleVec0 = { scale0, scale0, scale0 };
        mvMat4 stransform0 = scale(identity_mat4(), scaleVec0);
        mvMat4 ttransform0 = translate(identity_mat4(), translate0);

        if (const auto ecode = process_viewport_events()) break;

        if (window->resized)
        {
            recreate_swapchain(window->width, window->height);
            window->resized = false;
        }

        static bool recreatePrimary = false;
        if (recreatePrimary)
        {
            offscreen.recreate();
            recreatePrimary = false;
        }

        static bool recreateShadowMapRS = false;
        if (recreateShadowMapRS)
        {
            directionalShadowMap.recreate();
            recreateShadowMapRS = false;
        }

        static bool recreateOShadowMapRS = false;
        if (recreateOShadowMapRS)
        {
            omniShadowMap.recreate();
            recreateOShadowMapRS = false;
        }

        //-----------------------------------------------------------------------------
        // clear targets
        //-----------------------------------------------------------------------------
        static float backgroundColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
        static float backgroundColor2[] = { 0.2f, 0.2f, 0.2f, 1.0f };
        ctx->ClearRenderTargetView(*GContext->graphics.target.GetAddressOf(), backgroundColor);
        ctx->ClearRenderTargetView(offscreen.targetView, backgroundColor2);
        ctx->ClearDepthStencilView(directionalShadowMap.depthView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0u);
        ctx->ClearDepthStencilView(offscreen.depthView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0u);
        for (u32 i = 0; i < 6; i++)
            ctx->ClearDepthStencilView(omniShadowMap.depthView[i], D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0u);

        //-----------------------------------------------------------------------------
        // begin frame
        //-----------------------------------------------------------------------------
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        ID3D11ShaderResourceView* const pSRV[6] = { NULL };
        ctx->PSSetShaderResources(0, 1, pSRV);
        ctx->PSSetShaderResources(1, 1, pSRV);
        ctx->PSSetShaderResources(2, 1, pSRV);
        ctx->PSSetShaderResources(3, 6, pSRV); // depth map
        ctx->PSSetShaderResources(4, 6, pSRV); // depth map
        ctx->PSSetShaderResources(5, 6, pSRV); // depth map
        ctx->PSSetShaderResources(6, 1, pSRV); // depth map

        //-----------------------------------------------------------------------------
        // directional shadow pass
        //-----------------------------------------------------------------------------
        ctx->OMSetRenderTargets(0, nullptr, directionalShadowMap.depthView);
        ctx->RSSetViewports(1u, &directionalShadowMap.viewport);
        ctx->RSSetState(directionalShadowMap.rasterizationState);

        for (int i = 0; i < am.sceneCount; i++)
            Renderer::render_scene_shadows(am, am.scenes[i].asset, directionalShadowMap.getViewMatrix(), directionalShadowMap.getProjectionMatrix(), stransform0, ttransform0);

        //-----------------------------------------------------------------------------
        // omni shadow pass
        //-----------------------------------------------------------------------------
        ctx->RSSetViewports(1u, &omniShadowMap.viewport);
        ctx->RSSetState(omniShadowMap.rasterizationState);

        for (u32 i = 0; i < 6; i++)
        {
            ctx->OMSetRenderTargets(0, nullptr, omniShadowMap.depthView[i]);
            mvVec3 look_target = pointlight.camera.pos + omniShadowMap.cameraDirections[i];
            mvMat4 camera_matrix = lookat(pointlight.camera.pos, look_target, omniShadowMap.cameraUps[i]);

            for (int i = 0; i < am.sceneCount; i++)
                Renderer::render_scene_shadows(am, am.scenes[i].asset, camera_matrix, perspective(M_PI_2, 1.0f, 0.5f, 100.0f), stransform0, ttransform0);
        }

        //-----------------------------------------------------------------------------
        // offscreen pass
        //-----------------------------------------------------------------------------
        ctx->OMSetRenderTargets(1, &offscreen.targetView, offscreen.depthView);
        ctx->RSSetViewports(1u, &offscreen.viewport);

        mvMat4 viewMatrix = create_fps_view(camera);
        mvMat4 projMatrix = create_projection(camera);

        globalInfo.camPos = camera.pos;
        directionalLight.info.viewLightDir = directionalShadowMap.camera.dir;

        // update constant buffers
        update_const_buffer(pointlight.buffer, &pointlight.info);
        update_const_buffer(directionalLight.buffer, &directionalLight.info);
        update_const_buffer(globalInfoBuffer, &globalInfo);
        update_const_buffer(directionalShadowMap.buffer, &directionalShadowMap.info);
        update_const_buffer(omniShadowMap.buffer, &omniShadowMap.info);

        // vertex constant buffers
        ctx->VSSetConstantBuffers(1u, 1u, &directionalShadowMap.buffer.buffer);
        ctx->VSSetConstantBuffers(2u, 1u, &omniShadowMap.buffer.buffer);

        // pixel constant buffers
        ctx->PSSetConstantBuffers(0u, 1u, &pointlight.buffer.buffer);
        ctx->PSSetConstantBuffers(2u, 1u, &directionalLight.buffer.buffer);
        ctx->PSSetConstantBuffers(3u, 1u, &globalInfoBuffer.buffer);

        // samplers
        ctx->PSSetSamplers(1u, 1, &directionalShadowMap.sampler);
        ctx->PSSetSamplers(2u, 1, &omniShadowMap.sampler);
        ctx->PSSetSamplers(3u, 1, &skybox.cubeSampler);

        // textures
        ctx->PSSetShaderResources(5u, 1, &directionalShadowMap.resourceView);
        ctx->PSSetShaderResources(6u, 1, &omniShadowMap.resourceView);
        ctx->PSSetShaderResources(7u, 1, &skybox.cubeTexture.textureView);

        Renderer::render_mesh_solid(am, pointlight.mesh, translate(identity_mat4(), pointlight.camera.pos), viewMatrix, projMatrix);

        for (int i = 0; i < am.sceneCount; i++)
            Renderer::render_scene(am, am.scenes[i].asset, viewMatrix, projMatrix, stransform0, ttransform0);

        if (globalInfo.useSkybox) render_skybox(skybox, viewMatrix, projMatrix);

        //-----------------------------------------------------------------------------
        // main pass
        //-----------------------------------------------------------------------------
        ctx->OMSetRenderTargets(1, GContext->graphics.target.GetAddressOf(), *GContext->graphics.targetDepth.GetAddressOf());
        ctx->RSSetViewports(1u, &GContext->graphics.viewport);

        ImGui::SetNextWindowBgAlpha(1.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f); // to prevent main window corners from showing
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(14.0f, 5.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_TableBorderLight, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_TableBorderStrong, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        
        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
        ImGui::SetNextWindowSize(ImVec2((float)window->width, (float)window->height));

        static ImGuiWindowFlags windowFlags =
            ImGuiWindowFlags_NoBringToFrontOnFocus |
            ImGuiWindowFlags_NoDecoration;

        ImGui::Begin("Main Window", 0, windowFlags);

        static ImGuiTableFlags tableflags = 
            ImGuiTableFlags_Borders |
            ImGuiTableFlags_Resizable | 
            ImGuiTableFlags_SizingStretchProp |
            ImGuiTableFlags_NoHostExtendX;

        if (ImGui::BeginTable("Main Table", 3, tableflags))
        {

            ImGui::TableSetupColumn("Scene Controls", ImGuiTableColumnFlags_WidthFixed, 300.0f);
            ImGui::TableSetupColumn("Primary Scene");
            ImGui::TableSetupColumn("Light Controls", ImGuiTableColumnFlags_WidthFixed, 300.0f);
            ImGui::TableNextColumn();

            //-----------------------------------------------------------------------------
            // left panel
            //-----------------------------------------------------------------------------

            ImGui::TableSetColumnIndex(0);
            ImGui::Dummy(ImVec2(50.0f, 25.0f));

            ImGui::Text("%s", "Scene:");
            ImGui::DragFloat("Scale", &scale0, 0.1f, 0.0f);
            ImGui::DragFloat3("Translate", &translate0.x, 1.0f, -200.0f, 200.0f);
            ImGui::ColorEdit3("Ambient Color", &globalInfo.ambientColor.x);
            ImGui::Checkbox("Use Skybox", (bool*)&globalInfo.useSkybox);

            ImGui::Dummy(ImVec2(50.0f, 25.0f));
            ImGui::Text("%s", "Directional Shadows:");
            ImGui::Checkbox("Use Direct Shadows", (bool*)&globalInfo.useShadows);
            if (globalInfo.useShadows)
            {
                if (ImGui::Checkbox("Backface Cull", &directionalShadowMap.backface)) recreateShadowMapRS = true;
                if(ImGui::DragInt("Depth Bias##d", &directionalShadowMap.depthBias, 1.0f, 0, 100)) recreateShadowMapRS=true;
                if(ImGui::DragFloat("Slope Bias##d", &directionalShadowMap.slopeBias, 0.1f, 0.1f, 10.0f)) recreateShadowMapRS=true;
            }

            ImGui::Checkbox("Use PCF", (bool*)&globalInfo.usePCF);
            if (globalInfo.usePCF)
            {
                ImGui::SliderInt("PCF Range", &globalInfo.pcfRange, 1, 5);
            }

            ImGui::Dummy(ImVec2(50.0f, 25.0f));
            ImGui::Text("%s", "OmniDirectional Shadows:");
            ImGui::Checkbox("Use Omni Shadows", (bool*)&globalInfo.useOmniShadows);
            if (globalInfo.useOmniShadows)
            {
                if (ImGui::DragInt("Depth Bias", &omniShadowMap.depthBias, 1.0f, 0, 100)) recreateOShadowMapRS = true;
                if (ImGui::DragFloat("Slope Bias", &omniShadowMap.slopeBias, 0.1f, 0.1f, 10.0f)) recreateOShadowMapRS = true;
            }

            ImGui::Dummy(ImVec2(50.0f, 25.0f));
            ImGui::Text("%s", "Textures:");
            ImGui::Checkbox("Use Albedo", (bool*)&globalInfo.useAlbedo);
            ImGui::Checkbox("Use Normal", (bool*)&globalInfo.useNormalMap);
            ImGui::Checkbox("Use Metalness", (bool*)&globalInfo.useMetalness);
            ImGui::Checkbox("Use Roughness", (bool*)&globalInfo.useRoughness);
            ImGui::Checkbox("Use Irradiance", (bool*)&globalInfo.useIrradiance);
            ImGui::Checkbox("Use Reflection", (bool*)&globalInfo.useReflection);
            ImGui::Checkbox("Use Emissive", (bool*)&globalInfo.useEmissiveMap);
            ImGui::Checkbox("Use Occlusion", (bool*)&globalInfo.useOcclusionMap);

            //-----------------------------------------------------------------------------
            // center panel
            //-----------------------------------------------------------------------------
            ImGui::TableSetColumnIndex(1);
            ImGui::GetForegroundDrawList()->AddText(ImVec2(15, 7),
                ImColor(0.0f, 1.0f, 0.0f), std::string(std::to_string(ImGui::GetIO().Framerate) + " FPS").c_str());

            if (ImGui::IsWindowHovered())
                update_fps_camera(camera, dt, 12.0f, 0.004f);

            ImVec2 contentSize = ImGui::GetContentRegionAvail();
            offscreen.viewport = { 0.0f, 0.0f, contentSize.x, contentSize.y, 0.0f, 1.0f };
            camera.aspectRatio = offscreen.viewport.Width / offscreen.viewport.Height;

            ImGui::Image(offscreen.resourceView, contentSize);
            if (!(contentSize.x == oldContentRegion.x && contentSize.y == oldContentRegion.y))
                recreatePrimary = true;
            oldContentRegion = contentSize;

            //-----------------------------------------------------------------------------
            // right panel
            //-----------------------------------------------------------------------------
            ImGui::TableSetColumnIndex(2);
            ImGui::Dummy(ImVec2(50.0f, 25.0f));
            ImGui::Indent(14.0f);

            ImGui::Text("%s", "Directional Light:");
            bool directionalCameraChange = false;
            if (ImGui::SliderFloat3("Position##d", &directionalShadowMap.camera.pos.x, -200.0f, 200.0f))
                directionalCameraChange = true;

            if (ImGui::SliderFloat("Angle (x)", &directionalShadowMap.angle, -45.0f, 45.0f))
                directionalCameraChange = true;

            if (ImGui::SliderFloat("Width", &directionalShadowMap.width, 1.0f, 200.0f))
                directionalCameraChange = true;

            if (ImGui::SliderFloat("Near Z", &directionalShadowMap.camera.nearZ, 1.0f, 200.0f))
                directionalCameraChange = true;

            if (ImGui::SliderFloat("Far Z", &directionalShadowMap.camera.farZ, 1.0f, 200.0f))
                directionalCameraChange = true;

            if (directionalCameraChange)
            {
                directionalShadowMap.camera.width = directionalShadowMap.width;
                directionalShadowMap.camera.height = directionalShadowMap.width;

                f32 zcomponent = sinf(M_PI * directionalShadowMap.angle / 180.0f);
                f32 ycomponent = cosf(M_PI * directionalShadowMap.angle / 180.0f);

                directionalShadowMap.camera.dir = { 0.0f, -ycomponent, zcomponent };
                directionalShadowMap.info.view = lookat(
                    directionalShadowMap.camera.pos, directionalShadowMap.camera.pos - directionalShadowMap.camera.dir, directionalShadowMap.camera.up);
                directionalShadowMap.info.projection = ortho(
                    -directionalShadowMap.camera.width/2.0f,
                    directionalShadowMap.camera.width / 2.0f,
                    -directionalShadowMap.camera.height / 2.0f,
                    directionalShadowMap.camera.height / 2.0f,
                    directionalShadowMap.camera.nearZ, 
                    directionalShadowMap.camera.farZ);

                //directionalLight.info.viewLightDir = directionalShadowMap.camera.dir;
            }

            ImGui::Dummy(ImVec2(50.0f, 25.0f));
            ImGui::Text("%s", "Point Light:");
            if (ImGui::SliderFloat3("Position##o", &pointlight.info.viewLightPos.x, -25.0f, 50.0f))
            {
                pointlight.camera.pos = { pointlight.info.viewLightPos.x, pointlight.info.viewLightPos.y, pointlight.info.viewLightPos.z };
                omniShadowMap.info.view = create_lookat_view(pointlight.camera);
            }

            ImGui::Unindent(14.0f);
            ImGui::EndTable();

            ImGui::End();
            ImGui::PopStyleVar(3);
            ImGui::PopStyleColor(2);

            // render imgui
            ImGui::Render();
            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

            // present
            GContext->graphics.swapChain->Present(1, 0);
        }
    }

    offscreen.cleanup();
    directionalShadowMap.cleanup();
    omniShadowMap.cleanup();

    // Cleanup
    mvCleanupAssetManager(&am);
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
    mvDestroyContext();
}

