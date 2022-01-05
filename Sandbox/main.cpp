#include <imgui.h>
#include <imgui_internal.h>
#include <implot.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>
#include "mvLights.h"
#include "mvEnvironment.h"
#include "mvShadows.h"
#include "mvOffscreenPass.h"
#include "mvRenderer.h"
#include "mvImporter.h"
#include "mvTimer.h"
#include "gltf_scene_info.h"
#include "mvAnimation.h"

// TODO: make most of these runtime options
#define MODEL_CACHE_SIZE 5
#define ENV_CACHE_SIZE 3
static f32    shadowWidth = 95.0f;
static int    initialWidth = 1850;
static int    initialHeight = 900;
static ImVec2 oldContentRegion = ImVec2(500, 500);

void blendCallback(const ImDrawList* parent_list, const ImDrawCmd* cmd)
{
    // Setup blend state
    const float blend_factor[4] = { 0.f, 0.f, 0.f, 0.f };
    GContext->graphics.imDeviceContext->OMSetBlendState((ID3D11BlendState*)cmd->UserCallbackData, blend_factor, 0xffffffff);
}

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

    mvRendererContext renderCtx{};
    {
        D3D11_BLEND_DESC desc{};
        desc.AlphaToCoverageEnable = false;
        desc.RenderTarget[0].BlendEnable = false;
        desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
        desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
        desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        GContext->graphics.device->CreateBlendState(&desc, &renderCtx.finalBlendState);
    }

    // main camera
    mvCamera camera = create_perspective_camera({ -13.5f, 6.0f, 3.5f }, (f32)M_PI_4, 1.0f, 0.1f, 400.0f);
    camera.yaw = (f32)M_PI;

    // lights
    mvPointLight pointlight = create_point_light(am);
    mvDirectionalLight directionalLight = create_directional_light(am);

    // passes
    mvOffscreenPass offscreen = mvOffscreenPass(500.0f, 500.0f);
    mvDirectionalShadowPass directionalShadowMap = mvDirectionalShadowPass(am, 4096, shadowWidth);
    mvOmniShadowPass omniShadowMap = mvOmniShadowPass(am, 2048);
    
    omniShadowMap.info.view = create_lookat_view(pointlight.camera);

    GlobalInfo globalInfo{};
    mvConstBuffer globalInfoBuffer = create_const_buffer(&globalInfo, sizeof(GlobalInfo));
    register_asset(&am, "global_constant_buffer", globalInfoBuffer);

    mvTimer timer;
    mvEnvironment cachedEnvironments[ENV_CACHE_SIZE];
    int cachedEnvironmentsID[ENV_CACHE_SIZE];
    int cacheModel[MODEL_CACHE_SIZE] = { -1, -1, -1 };
    mvAssetID cacheScenes[MODEL_CACHE_SIZE] = { -1, -1, -1 };
    int cacheIndex = 0;
    int activeScene = -1;
    int modelIndex = 0;
    int activeEnv = -1;
    int envMapIndex = 0;
    int envCacheIndex = 0;
    bool blur = true;
    f32 currentTime = 0.0f;
    while (true)
    {
        const auto dt = timer.mark() * 1.0f;
        currentTime += dt;

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

        static bool reloadMaterials = false;
        if (reloadMaterials)
        {
            reload_materials(&am);
            reloadMaterials = false;
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

        static bool recreateEnvironment = true;
        if (recreateEnvironment)
        {            
            if (envMapIndex == 0)
            {
                globalInfo.useSkybox = false;
            }
            else
            {
                globalInfo.useSkybox = true;

                b8 cacheFound = false;
                for (int i = 0; i < ENV_CACHE_SIZE; i++)
                {
                    if (cachedEnvironmentsID[i] == envMapIndex)
                    {
                        activeEnv = i;
                        cacheFound = true;
                        break;
                    }
                }

                if (!cacheFound)
                {
                    std::string newMap = "../../data/glTF-Sample-Environments/" + std::string(env_maps[envMapIndex]) + ".hdr";
                    cleanup_environment(cachedEnvironments[envCacheIndex]);
                    cachedEnvironments[envCacheIndex] = create_environment(newMap, 1024, 1024, 1.0f, 7);
                    cachedEnvironmentsID[envCacheIndex] = envMapIndex;
                    activeEnv = envCacheIndex;
                    envCacheIndex++;
                    if (envCacheIndex == ENV_CACHE_SIZE) envCacheIndex = 0;
                }
            }

            recreateEnvironment = false;
        }

        static bool changeScene = false;
        if (changeScene)
        {
            b8 cacheFound = false;
            for (int i = 0; i < MODEL_CACHE_SIZE; i++)
            {
                if (cacheModel[i] == modelIndex)
                {
                    activeScene = cacheScenes[i];
                    cacheFound = true;
                    break;
                }
            }

            if (!cacheFound)
            {

                mvAssetID sceneToRemove = cacheScenes[cacheIndex];

                if (sceneToRemove > -1)
                {
                    mvScene* previousScene = &am.scenes[sceneToRemove].asset;
                    for (int i = 0; i < previousScene->nodeCount; i++)
                    {
                        if (am.nodes[previousScene->nodes[i]].asset.mesh > -1)
                        {
                            mvMesh& mesh = am.meshes[am.nodes[previousScene->nodes[i]].asset.mesh].asset;
                            for (int i = 0; i < mesh.primitives.size(); i++)
                            {
                                unregister_buffer_asset(&am, mesh.primitives[i].indexBuffer);
                                unregister_buffer_asset(&am, mesh.primitives[i].vertexBuffer);
                                unregister_texture_asset(&am, mesh.primitives[i].normalTexture);
                                unregister_texture_asset(&am, mesh.primitives[i].specularTexture);
                                unregister_texture_asset(&am, mesh.primitives[i].albedoTexture);
                                unregister_texture_asset(&am, mesh.primitives[i].emissiveTexture);
                                unregister_texture_asset(&am, mesh.primitives[i].occlusionTexture);
                                unregister_texture_asset(&am, mesh.primitives[i].metalRoughnessTexture);
                                unregister_material_asset(&am, mesh.primitives[i].materialID);
                                unregister_material_asset(&am, mesh.primitives[i].shadowMaterialID);
                            }
                        }
                        unregister_mesh_asset(&am, am.nodes[previousScene->nodes[i]].asset.mesh); // maybe do mesh offset?
                        unregister_camera_asset(&am, am.nodes[previousScene->nodes[i]].asset.camera); // maybe do mesh offset?
                        unregister_node_asset(&am, previousScene->nodes[i]);
                    }
                    unregister_scene_asset(&am, sceneToRemove);
                }

                mvGLTFModel gltfmodel0 = mvLoadGLTF(gltf_directories[modelIndex], gltf_models[modelIndex]);
                activeScene = load_gltf_assets(am, gltfmodel0);
                mvCleanupGLTF(gltfmodel0);
                cacheScenes[cacheIndex] = activeScene;
                cacheModel[cacheIndex] = modelIndex;
                cacheIndex++;
                if (cacheIndex == MODEL_CACHE_SIZE) cacheIndex = 0;

            }

            changeScene = false;
        }

        //-----------------------------------------------------------------------------
        // update animations
        //-----------------------------------------------------------------------------
        for (i32 i = 0; i < am.animationCount; i++)
        {
            advance(am, am.animations[i].asset, currentTime);
        }

        for (i32 i = 0; i < am.nodeCount; i++)
        {
            mvNode& node = am.nodes[i].asset;
            if (node.animated)
            {
                mvVec3& trans = node.translationAnimated ? node.animationTranslation : node.translation;
                mvVec3& scal = node.scaleAnimated ? node.animationScale : node.scale;
                mvVec4& rot = node.rotationAnimated ? node.animationRotation : node.rotation;
                node.matrix = rotation_translation_scale(rot, trans, scal);
            }
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

        ID3D11ShaderResourceView* const pSRV[12] = { NULL };
        for(int i = 0; i < 12; i++)
            ctx->PSSetShaderResources(0, i, pSRV);

        //-----------------------------------------------------------------------------
        // directional shadow pass
        //-----------------------------------------------------------------------------
        ctx->OMSetRenderTargets(0, nullptr, directionalShadowMap.depthView);
        ctx->RSSetViewports(1u, &directionalShadowMap.viewport);
        ctx->RSSetState(directionalShadowMap.rasterizationState);

        if(activeScene > -1)
            Renderer::render_scene_shadows(am, am.scenes[activeScene].asset, directionalShadowMap.getViewMatrix(), directionalShadowMap.getProjectionMatrix(), stransform0, ttransform0);

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

            if (activeScene > -1)
                Renderer::render_scene_shadows(am, am.scenes[activeScene].asset, camera_matrix, perspective(M_PI_2, 1.0f, 0.5f, 100.0f), stransform0, ttransform0);
        }

        //-----------------------------------------------------------------------------
        // main pass
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
        ctx->PSSetSamplers(5u, 1, &directionalShadowMap.sampler);
        ctx->PSSetSamplers(6u, 1, &omniShadowMap.sampler);

        if (activeEnv > -1)
        {
            ctx->PSSetSamplers(7u, 1, &cachedEnvironments[activeEnv].sampler);
            ctx->PSSetSamplers(8u, 1, &cachedEnvironments[activeEnv].sampler);
            ctx->PSSetSamplers(9u, 1, &cachedEnvironments[activeEnv].brdfSampler);
        }

        // textures
        ctx->PSSetShaderResources(5u, 1, &directionalShadowMap.resourceView);
        ctx->PSSetShaderResources(6u, 1, &omniShadowMap.resourceView);

        if (activeEnv > -1)
        {
            ctx->PSSetShaderResources(7u, 1, &cachedEnvironments[activeEnv].irradianceMap.textureView);
            ctx->PSSetShaderResources(8u, 1, &cachedEnvironments[activeEnv].specularMap.textureView);
        }

        if (activeEnv > -1)
        {
            ctx->PSSetShaderResources(9u, 1, &cachedEnvironments[activeEnv].brdfLUT.textureView);
        }

        Renderer::render_mesh_solid(am, pointlight.mesh, translate(identity_mat4(), pointlight.camera.pos), viewMatrix, projMatrix);

        if (activeScene > -1)
        {
            Renderer::submit_scene(am, renderCtx, am.scenes[activeScene].asset, stransform0, ttransform0);
            Renderer::render_jobs(am, renderCtx, viewMatrix, projMatrix);
            //Renderer::render_scene(am, am.scenes[activeScene].asset, viewMatrix, projMatrix, stransform0, ttransform0);
        }

        if (globalInfo.useSkybox && activeEnv > -1)
        {
            Renderer::render_skybox(am, blur ? cachedEnvironments[activeEnv].specularMap : cachedEnvironments[activeEnv].skyMap, cachedEnvironments[activeEnv].sampler, viewMatrix, projMatrix);
        }

        //-----------------------------------------------------------------------------
        // ui pass
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
            if (ImGui::Checkbox("Punctual Lighting", (bool*)&GContext->IO.punctualLighting))reloadMaterials = true;
            if(ImGui::Checkbox("Image Based", (bool*)&GContext->IO.imageBasedLighting)) reloadMaterials = true;
            if(ImGui::Checkbox("KHR_materials_clearcoat", (bool*)&GContext->IO.clearcoat)) reloadMaterials = true;

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

            ImGui::GetWindowDrawList()->AddCallback(blendCallback, renderCtx.finalBlendState);
            ImGui::Image(offscreen.resourceView, contentSize, ImVec2(0,0), ImVec2(1, 1), ImVec4(1, 1, 1, 1), ImVec4(0, 0, 0, 0));
            if (!(contentSize.x == oldContentRegion.x && contentSize.y == oldContentRegion.y))
                recreatePrimary = true;
            oldContentRegion = contentSize;
            ImGui::GetWindowDrawList()->AddCallback(ImDrawCallback_ResetRenderState, nullptr);

            //-----------------------------------------------------------------------------
            // right panel
            //-----------------------------------------------------------------------------
            ImGui::TableSetColumnIndex(2);

            ImGui::Checkbox("Blur##skybox", &blur);
            if (ImGui::ListBox("Environment##separate", &envMapIndex, env_maps, 14, 14))
            {
                recreateEnvironment = true;
            }

            ImGui::Dummy(ImVec2(50.0f, 25.0f));
            ImGui::Indent(14.0f);

            if (ImGui::ListBox("Model##separate", &modelIndex, gltf_names, 80+36+59, 20))
            {
                changeScene = true;
            }

            ImGui::Dummy(ImVec2(50.0f, 25.0f));
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

    for(int i = 0; i < ENV_CACHE_SIZE; i++)
        cleanup_environment(cachedEnvironments[i]);

    // Cleanup
    renderCtx.finalBlendState->Release();
    mvCleanupAssetManager(&am);
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
    mvDestroyContext();
}

