#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>
#include "mvLights.h"
#include "mvEnvironment.h"
#include "mvOffscreenPass.h"
#include "mvRenderer.h"
#include "mvImporter.h"
#include "mvTimer.h"
#include "gltf_scene_info.h"
#include "mvAnimation.h"
#include "mvSandbox.h"
#include "mvAssetLoader.h"
#include "mvViewport.h"

mvViewport* window = nullptr;
ImVec2 oldContentRegion = ImVec2(500, 500);

// scenes & models
i32 modelIndex = 27;   // current model from list
i32 envMapIndex = 5;

// misc
f32    currentTime = 0.0f;

// flags
b8 showSkybox = true;
b8 blur = true;

// per frame dirty flags
bool reloadMaterials = false;
bool recreatePrimary = false;
bool recreateEnvironment = false;
bool changeScene = true;

int main()
{    
    create_context();
    GContext->IO.shaderDirectory = "../../Sandbox/shaders/";
    GContext->IO.resourceDirectory = "../../Resources/";

    window = initialize_viewport(1850, 900);
    setup_graphics(*window);

    // setup imgui
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui_ImplWin32_Init(window->hWnd);
    ImGui_ImplDX11_Init(GContext->graphics.device.Get(), GContext->graphics.imDeviceContext.Get());
    ImGui::StyleColorsClassic();

    ID3D11DeviceContext* ctx = GContext->graphics.imDeviceContext.Get();

    mvEnvironment environment = create_environment("../../data/glTF-Sample-Environments/" + std::string(env_maps[envMapIndex]) + ".hdr", 1024, 1024, 1.0f, 7);
    mvGLTFModel gltfmodel0 = mvLoadGLTF(gltf_directories[modelIndex], gltf_models[modelIndex]);
    mvModel model = load_gltf_assets(gltfmodel0);
    
    mvRendererContext renderCtx = Renderer::create_renderer_context();

    // main camera
    mvCamera camera = create_perspective_camera({ 0.0f, 0.0f, 5.0f }, (f32)PI/4.0f, 1.0f, 0.1f, 400.0f);
    renderCtx.camera = &camera;

    // lights
    mvPointLight pointlight = create_point_light();
    pointlight.info.viewLightPos = mvVec4{ -15.0f, 15.0f, 10.0f, 0.0f };
    mvDirectionalLight directionalLight = create_directional_light();

    // passes
    mvOffscreenPass offscreen = mvOffscreenPass(500.0f, 500.0f);

    mvTimer timer;
    while (true)
    {
        
        if (const auto ecode = process_viewport_events()) break;

        const auto dt = timer.mark() * 1.0f;

        //-----------------------------------------------------------------------------
        // updates
        //-----------------------------------------------------------------------------

        currentTime += dt;

        if (window->resized)
        {
            recreate_swapchain(window->width, window->height);
            window->resized = false;
        }

        if (reloadMaterials)
        {
            reload_materials(&model.materialManager);
            reloadMaterials = false;
        }

        if (recreatePrimary)
        {
            offscreen.recreate();
            recreatePrimary = false;
        }

        if (recreateEnvironment)
        {
            recreateEnvironment = false;
            if (envMapIndex == 0)
                showSkybox = false;

            else
            {
                showSkybox = true;
                std::string newMap = "../../data/glTF-Sample-Environments/" + std::string(env_maps[envMapIndex]) + ".hdr";
                cleanup_environment(environment);
                environment = create_environment(newMap, 1024, 1024, 1.0f, 7);
            }
        }
        if (changeScene)
        {
            changeScene = false;
            unload_gltf_assets(model);

            gltfmodel0 = mvLoadGLTF(gltf_directories[modelIndex], gltf_models[modelIndex]);
            model = load_gltf_assets(gltfmodel0);
            mvCleanupGLTF(gltfmodel0);

            camera.minBound = mvVec3{ model.minBoundary[0], model.minBoundary[1], model.minBoundary[2] };
            camera.maxBound = mvVec3{ model.maxBoundary[0], model.maxBoundary[1], model.maxBoundary[2] };

            camera.target.x = (camera.minBound.x + camera.maxBound.x) / 2.0f;
            camera.target.y = (camera.minBound.y + camera.maxBound.y) / 2.0f;
            camera.target.z = (camera.minBound.z + camera.maxBound.z) / 2.0f;

            // fit radius
            f32 maxAxisLength = get_max(camera.maxBound.x - camera.minBound.x, camera.maxBound.y - camera.minBound.y);
            f32 yfov = camera.fieldOfView;
            f32 xfov = camera.fieldOfView * camera.aspectRatio;
            f32 yZoom = maxAxisLength / 2.0f / tan(yfov / 2.0f);
            f32 xZoom = maxAxisLength / 2.0f / tan(xfov / 2.0f);
            camera.distance = get_max(xZoom, yZoom);
            camera.baseDistance = camera.distance;

            // fit camera planes
            f32 longestDistance = 10.0f * sqrt(camera.minBound.x * camera.minBound.x + camera.minBound.y * camera.minBound.y + camera.minBound.z * camera.minBound.z);
            camera.yaw = 0.0f;
            camera.pitch = 0.0f;
            camera.nearZ = camera.distance - longestDistance * 0.6;
            camera.farZ = camera.distance + longestDistance * 0.6;
            camera.nearZ = get_max(camera.nearZ, camera.farZ / 10000.0f);
            camera.panSpeed = longestDistance / 3500;

            // fit camera target
            f32 target[3] = { 0.0f, 0.0f, 0.0f };
            for (i32 i = 0; i < 3; i++)
                target[i] = (camera.maxBound[i] + camera.minBound[i]) / 2.0f;
            camera.pos = mvVec3{ target[0], target[1], target[2] + camera.distance };
        }

        //-----------------------------------------------------------------------------
        // clear targets
        //-----------------------------------------------------------------------------
        static float backgroundColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
        static float backgroundColor2[] = { 0.2f, 0.2f, 0.2f, 1.0f };
        ctx->ClearRenderTargetView(*GContext->graphics.target.GetAddressOf(), backgroundColor);
        ctx->ClearRenderTargetView(offscreen.targetView.Get(), backgroundColor2);
        ctx->ClearDepthStencilView(offscreen.depthView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0u);

        //-----------------------------------------------------------------------------
        // update animations
        //-----------------------------------------------------------------------------
        for (i32 i = 0; i < model.animations.size(); i++)
            advance_animations(model, model.animations[i], currentTime);

        //-----------------------------------------------------------------------------
        // begin frame
        //-----------------------------------------------------------------------------
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        ID3D11ShaderResourceView* const pSRV[12] = { NULL };
        for(int i = 0; i < 12; i++)
            ctx->PSSetShaderResources(i, 6, pSRV);

        i32 activeScene = model.defaultScene;

        //-----------------------------------------------------------------------------
        // offscreen pass
        //-----------------------------------------------------------------------------
        ctx->OMSetRenderTargets(1, offscreen.targetView.GetAddressOf(), offscreen.depthView.Get());
        ctx->RSSetViewports(1u, &offscreen.viewport);

        mvMat4 viewMatrix = create_arcball_view(camera);
        mvMat4 projMatrix = create_projection(camera);

        renderCtx.globalInfo.camPos = camera.pos;

        // update constant buffers
        update_const_buffer(pointlight.buffer, &pointlight.info);
        update_const_buffer(directionalLight.buffer, &directionalLight.info);
        update_const_buffer(renderCtx.globalInfoBuffer, &renderCtx.globalInfo);

        // pixel constant buffers
        ctx->PSSetConstantBuffers(0u, 1u, pointlight.buffer.buffer.GetAddressOf());
        ctx->PSSetConstantBuffers(2u, 1u, directionalLight.buffer.buffer.GetAddressOf());
        ctx->PSSetConstantBuffers(3u, 1u, renderCtx.globalInfoBuffer.buffer.GetAddressOf());

        if (envMapIndex > -1)
        {
            ctx->PSSetSamplers(12u, 1, environment.sampler.GetAddressOf());
            ctx->PSSetSamplers(13u, 1, environment.sampler.GetAddressOf());
            ctx->PSSetSamplers(14u, 1, environment.brdfSampler.GetAddressOf());
            ctx->PSSetShaderResources(12u, 1, environment.irradianceMap.textureView.GetAddressOf());
            ctx->PSSetShaderResources(13u, 1, environment.specularMap.textureView.GetAddressOf());
            ctx->PSSetShaderResources(14u, 1, environment.brdfLUT.textureView.GetAddressOf());
        }

        Renderer::render_mesh_solid(renderCtx, model, pointlight.mesh, translate(identity_mat4(), pointlight.info.viewLightPos.xyz()), viewMatrix, projMatrix);

        if (activeScene > -1)
        {
            Renderer::submit_scene(model, renderCtx, model.scenes[activeScene]);
            //-----------------------------------------------------------------------------
            // update skins
            //-----------------------------------------------------------------------------
            for (i32 i = 0; i < model.nodes.size(); i++)
            {
                mvNode& node = model.nodes[i];
                if (node.skin != -1 && node.mesh != -1)
                {
                    u32 skeleton = model.skins[node.skin].skeleton;
                    if(skeleton != -1)
                        compute_joints(model, model.nodes[skeleton].inverseWorldTransform, model.skins[node.skin]);
                    else
                        compute_joints(model, viewMatrix, model.skins[node.skin]);
                }
            }

            Renderer::render_scenes(model, renderCtx, viewMatrix, projMatrix);
        }

        if (showSkybox && envMapIndex > -1)
           Renderer::render_skybox(renderCtx, model, blur ? environment.specularMap : environment.skyMap, environment.sampler.Get(), viewMatrix, projMatrix);

        //-----------------------------------------------------------------------------
        // ui
        //-----------------------------------------------------------------------------
        ctx->OMSetRenderTargets(1, GContext->graphics.target.GetAddressOf(), *GContext->graphics.targetDepth.GetAddressOf());
        ctx->RSSetViewports(1u, &GContext->graphics.viewport);

        // styling to fill viewport 
        ImGui::SetNextWindowBgAlpha(1.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f); // to prevent main window corners from showing
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_TableBorderLight, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_TableBorderStrong, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
        ImGui::SetNextWindowSize(ImVec2((float)window->width, (float)window->height));
        static ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoDecoration;

        ImGui::Begin("Main Window", 0, windowFlags);

        static ImGuiTableFlags tableflags =
            ImGuiTableFlags_Borders |
            ImGuiTableFlags_Resizable |
            ImGuiTableFlags_SizingStretchProp |
            ImGuiTableFlags_NoHostExtendX;

        if (ImGui::BeginTable("Main Table", 2, tableflags))
        {

            ImGui::TableSetupColumn("Primary Scene");
            ImGui::TableSetupColumn("Scene Controls", ImGuiTableColumnFlags_WidthFixed, 400.0f);
            ImGui::TableNextColumn();

            //-----------------------------------------------------------------------------
            // render panel
            //-----------------------------------------------------------------------------
            ImGui::TableSetColumnIndex(0);

            ImGui::GetForegroundDrawList()->AddText(ImVec2(ImGui::GetWindowPos().x + 45, 15),
                ImColor(0.0f, 1.0f, 0.0f), std::string(std::to_string(io.Framerate) + " FPS").c_str());

            ImGui::GetForegroundDrawList()->AddText(ImVec2(ImGui::GetWindowPos().x + 45, 30),
                ImColor(0.0f, 1.0f, 0.0f), std::string(std::to_string(camera.pos.x) + ", "
                    + std::to_string(camera.pos.y) + ", " + std::to_string(camera.pos.z)).c_str());
            ImGui::GetForegroundDrawList()->AddText(ImVec2(ImGui::GetWindowPos().x + 45, 45),
                ImColor(0.0f, 1.0f, 0.0f), std::string(std::to_string(camera.pitch) + ", "
                    + std::to_string(camera.yaw)).c_str());

            ImVec2 contentSize = ImGui::GetContentRegionAvail();

            if (ImGui::IsWindowHovered())
                update_arcball_camera(*renderCtx.camera, dt);


            offscreen.viewport = { 0.0f, 0.0f, contentSize.x, contentSize.y, 0.0f, 1.0f };
            renderCtx.camera->aspectRatio = offscreen.viewport.Width / offscreen.viewport.Height;

            auto blendCallback = [](const ImDrawList* parent_list, const ImDrawCmd* cmd) {
                // Setup blend state
                const float blend_factor[4] = { 0.f, 0.f, 0.f, 0.f };
                GContext->graphics.imDeviceContext->OMSetBlendState((ID3D11BlendState*)cmd->UserCallbackData, blend_factor, 0xffffffff);
            };

            ImGui::GetWindowDrawList()->AddCallback(blendCallback, renderCtx.finalBlendState);
            ImGui::Image(offscreen.resourceView.Get(), contentSize, ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1), ImVec4(0, 0, 0, 0));
            if (!(contentSize.x == oldContentRegion.x && contentSize.y == oldContentRegion.y))
                recreatePrimary = true;
            oldContentRegion = contentSize;
            ImGui::GetWindowDrawList()->AddCallback(ImDrawCallback_ResetRenderState, nullptr);

            //-----------------------------------------------------------------------------
            // right panel
            //-----------------------------------------------------------------------------
            ImGui::TableSetColumnIndex(1);
            ImGui::Indent(14.0f);

            ImGui::Dummy(ImVec2(50.0f, 25.0f));
            ImGui::Text("%s", "Environments");
            if (ImGui::Combo("##environments", &envMapIndex, env_maps, 11, 11)) recreateEnvironment = true;

            ImGui::Dummy(ImVec2(50.0f, 25.0f));

            ImGui::Text("%s", "Models");
            if (ImGui::Combo("##models", &modelIndex, gltf_names, 80 + 36 + 59, 20)) changeScene = true;

            ImGui::Dummy(ImVec2(50.0f, 25.0f));
            ImGui::Text("%s", "Lighting");
            if (ImGui::Checkbox("Punctual Lighting", (bool*)&GContext->IO.punctualLighting))reloadMaterials = true;
            if (ImGui::Checkbox("Image Based", (bool*)&GContext->IO.imageBasedLighting)) reloadMaterials = true;

            ImGui::Dummy(ImVec2(50.0f, 25.0f));
            ImGui::Text("%s", "Background");
            ImGui::Checkbox("Blur##skybox", &blur);

            ImGui::Dummy(ImVec2(50.0f, 25.0f));
            ImGui::Text("%s", "Directional Light:");
            static float dangle = 0.0f;
            if (ImGui::SliderFloat("X Angle##d", &dangle, -45.0f, 45.0f))
            {
                f32 zcomponent = sinf(PI * dangle / 180.0f);
                f32 ycomponent = cosf(PI * dangle / 180.0f);
                directionalLight.info.viewLightDir = { 0.0f, -ycomponent, zcomponent };
            }

            ImGui::Dummy(ImVec2(50.0f, 25.0f));
            ImGui::Text("%s", "Point Light:");
            ImGui::SliderFloat3("Position##o", &pointlight.info.viewLightPos.x, -25.0f, 50.0f);

            ImGui::Dummy(ImVec2(50.0f, 25.0f));
            ImGui::Text("%s", "Extensions");
            if (ImGui::Checkbox("KHR_materials_clearcoat", (bool*)&GContext->IO.clearcoat)) reloadMaterials = true;

            ImGui::Unindent(14.0f);
            ImGui::EndTable();

            ImGui::End();
            ImGui::PopStyleVar(4);
            ImGui::PopStyleColor(2);

            // render imgui
            ImGui::Render();
            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

            // present
            GContext->graphics.swapChain->Present(1, 0);
        }
    }

    offscreen.cleanup();

    // Cleanup
    renderCtx.finalBlendState->Release();
    cleanup_environment(environment);
    unload_gltf_assets(model);
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    destroy_context();
}

