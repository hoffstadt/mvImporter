#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>
#include "mvLights.h"
#include "mvOffscreenPass.h"
#include "mvGraphics.h"
#include "mvImporter.h"
#include "mvTimer.h"
#include "gltf_scene_info.h"
#include "mvAnimation.h"
#include "mvAssetLoader.h"
#include "mvViewport.h"

#define MV_ENVIRONMENT_CACHE 3
#define MV_MODEL_CACHE 5

mvViewport* window = nullptr;
ImVec2 oldContentRegion = ImVec2(500, 500);


// misc
float currentTime = 0.0f;

// flags
bool showSkybox = true;
bool blur = true;

// per frame dirty flags
bool reloadMaterials = false;
bool recreatePrimary = false;
bool recreateEnvironment = false;
bool changeScene = true;

int main()
{    
    // environment caching
    int envMapIndex = 5;
    int currentEnvironment = 0;
    int nextEnvironmentCacheIndex = 1;
    mvEnvironment environmentCache[MV_ENVIRONMENT_CACHE];
    int environmentIDCache[MV_ENVIRONMENT_CACHE];

    // model caching
    int modelIndex = 27;
    int currentModel = 0;
    int nextModelCacheIndex = 1;
    mvModel modelCache[MV_MODEL_CACHE];
    int modelIDCache[MV_MODEL_CACHE];

    for (int i = 0; i < MV_ENVIRONMENT_CACHE; i++)
        environmentIDCache[i] = -1;
    environmentIDCache[0] = envMapIndex;

    for (int i = 0; i < MV_MODEL_CACHE; i++)
        modelIDCache[i] = -1;
    modelIDCache[0] = modelIndex;
    
    window = initialize_viewport(1850, 900);
    mvGraphics graphics = setup_graphics(*window, "../sandbox_d3d11/shaders/");

    // setup imgui
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui_ImplWin32_Init(window->hWnd);
    ImGui_ImplDX11_Init(graphics.device.Get(), graphics.imDeviceContext.Get());
    ImGui::StyleColorsClassic();

    ID3D11DeviceContext* ctx = graphics.imDeviceContext.Get();

    environmentCache[0] = create_environment(graphics, "../data/glTF-Sample-Environments/" + std::string(env_maps[envMapIndex]) + ".hdr", 1024, 1024, 1.0f, 7);
    mvGLTFModel gltfmodel0 = mvLoadGLTF(gltf_directories[modelIndex], gltf_models[modelIndex]);
    modelCache[0] = load_gltf_assets(graphics, gltfmodel0);
    
    mvRendererContext renderCtx = create_renderer_context(graphics);

    // main camera
    mvCamera camera = create_perspective_camera({ 0.0f, 0.0f, 5.0f }, (float)PI/4.0f, 1.0f, 0.1f, 400.0f);
    renderCtx.camera = &camera;

    // lights
    mvPointLight pointlight = create_point_light(graphics);
    pointlight.info.viewLightPos = mvVec4{ -15.0f, 15.0f, 10.0f, 0.0f };
    mvDirectionalLight directionalLight = create_directional_light(graphics);

    // passes
    mvOffscreenPass offscreen = mvOffscreenPass(graphics, 500.0f, 500.0f);

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
            recreate_swapchain(graphics, window->width, window->height);
            window->resized = false;
        }

        if (reloadMaterials)
        {
            reload_materials(graphics, &modelCache[currentModel].materialManager);
            reloadMaterials = false;
        }

        if (recreatePrimary)
        {
            offscreen.recreate(graphics);
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
                bool cacheFound = false;
                for (int i = 0; i < MV_ENVIRONMENT_CACHE; i++)
                {
                    if (envMapIndex == environmentIDCache[i])
                    {
                        currentEnvironment = i;
                        cacheFound = true;
                        break;
                    }
                }

                if (!cacheFound)
                {
                    currentEnvironment = nextEnvironmentCacheIndex;
                    std::string newMap = "../data/glTF-Sample-Environments/" + std::string(env_maps[envMapIndex]) + ".hdr";
                    cleanup_environment(environmentCache[nextEnvironmentCacheIndex]);
                    environmentIDCache[nextEnvironmentCacheIndex] = envMapIndex;
                    environmentCache[nextEnvironmentCacheIndex] = create_environment(graphics, newMap, 1024, 1024, 1.0f, 7);
                    nextEnvironmentCacheIndex++;
                    if (nextEnvironmentCacheIndex >= MV_ENVIRONMENT_CACHE)
                        nextEnvironmentCacheIndex = 0;
                }
            }
        }
        if (changeScene)
        {
            changeScene = false;


            bool cacheFound = false;
            for (int i = 0; i < MV_MODEL_CACHE; i++)
            {
                if (modelIndex == modelIDCache[i])
                {
                    currentModel = i;
                    cacheFound = true;
                    break;
                }
            }

            if (!cacheFound)
            {
                currentModel = nextModelCacheIndex;
                unload_gltf_assets(modelCache[nextModelCacheIndex]);
                modelIDCache[nextModelCacheIndex] = modelIndex;
                gltfmodel0 = mvLoadGLTF(gltf_directories[modelIndex], gltf_models[modelIndex]);
                modelCache[nextModelCacheIndex] = load_gltf_assets(graphics, gltfmodel0);
                mvCleanupGLTF(gltfmodel0);
                nextModelCacheIndex++;
                if (nextModelCacheIndex >= MV_MODEL_CACHE)
                    nextModelCacheIndex = 0;
            }

            camera.minBound = mvVec3{ modelCache[currentModel].minBoundary[0], modelCache[currentModel].minBoundary[1], modelCache[currentModel].minBoundary[2] };
            camera.maxBound = mvVec3{ modelCache[currentModel].maxBoundary[0], modelCache[currentModel].maxBoundary[1], modelCache[currentModel].maxBoundary[2] };

            camera.target.x = (camera.minBound.x + camera.maxBound.x) / 2.0f;
            camera.target.y = (camera.minBound.y + camera.maxBound.y) / 2.0f;
            camera.target.z = (camera.minBound.z + camera.maxBound.z) / 2.0f;

            // fit radius
            float maxAxisLength = get_max(camera.maxBound.x - camera.minBound.x, camera.maxBound.y - camera.minBound.y);
            float yfov = camera.fieldOfView;
            float xfov = camera.fieldOfView * camera.aspectRatio;
            float yZoom = maxAxisLength / 2.0f / tan(yfov / 2.0f);
            float xZoom = maxAxisLength / 2.0f / tan(xfov / 2.0f);
            camera.distance = get_max(xZoom, yZoom);
            camera.baseDistance = camera.distance;

            // fit camera planes
            float longestDistance = 10.0f * sqrt(camera.minBound.x * camera.minBound.x + camera.minBound.y * camera.minBound.y + camera.minBound.z * camera.minBound.z);
            camera.yaw = 0.0f;
            camera.pitch = 0.0f;
            camera.nearZ = camera.distance - longestDistance * 0.6;
            camera.farZ = camera.distance + longestDistance * 0.6;
            camera.nearZ = get_max(camera.nearZ, camera.farZ / 10000.0f);
            camera.panSpeed = longestDistance / 3500;

            // fit camera target
            float target[3] = { 0.0f, 0.0f, 0.0f };
            for (int i = 0; i < 3; i++)
                target[i] = (camera.maxBound[i] + camera.minBound[i]) / 2.0f;
            camera.pos = mvVec3{ target[0], target[1], target[2] + camera.distance };
        }

        //-----------------------------------------------------------------------------
        // clear targets
        //-----------------------------------------------------------------------------
        static float backgroundColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
        static float backgroundColor2[] = { 0.2f, 0.2f, 0.2f, 1.0f };
        ctx->ClearRenderTargetView(*graphics.target.GetAddressOf(), backgroundColor);
        ctx->ClearRenderTargetView(offscreen.targetView.Get(), backgroundColor2);
        ctx->ClearDepthStencilView(offscreen.depthView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0u);

        //-----------------------------------------------------------------------------
        // update animations
        //-----------------------------------------------------------------------------
        for (int i = 0; i < modelCache[currentModel].animations.size(); i++)
            advance_animations(modelCache[currentModel], modelCache[currentModel].animations[i], currentTime);

        //-----------------------------------------------------------------------------
        // begin frame
        //-----------------------------------------------------------------------------
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        ID3D11ShaderResourceView* const pSRV[12] = { NULL };
        for(int i = 0; i < 12; i++)
            ctx->PSSetShaderResources(i, 6, pSRV);

        int activeScene = modelCache[currentModel].defaultScene;

        //-----------------------------------------------------------------------------
        // offscreen pass
        //-----------------------------------------------------------------------------
        ctx->OMSetRenderTargets(1, offscreen.targetView.GetAddressOf(), offscreen.depthView.Get());
        ctx->RSSetViewports(1u, &offscreen.viewport);

        mvMat4 viewMatrix = create_arcball_view(camera);
        mvMat4 projMatrix = create_projection(camera);

        renderCtx.globalInfo.camPos = camera.pos;

        // update constant buffers
        update_const_buffer(graphics, pointlight.buffer, &pointlight.info);
        update_const_buffer(graphics, directionalLight.buffer, &directionalLight.info);
        update_const_buffer(graphics, renderCtx.globalInfoBuffer, &renderCtx.globalInfo);

        // pixel constant buffers
        ctx->PSSetConstantBuffers(0u, 1u, pointlight.buffer.buffer.GetAddressOf());
        ctx->PSSetConstantBuffers(2u, 1u, directionalLight.buffer.buffer.GetAddressOf());
        ctx->PSSetConstantBuffers(3u, 1u, renderCtx.globalInfoBuffer.buffer.GetAddressOf());

        if (envMapIndex > -1)
        {
            ctx->PSSetSamplers(12u, 1, environmentCache[currentEnvironment].sampler.GetAddressOf());
            ctx->PSSetSamplers(13u, 1, environmentCache[currentEnvironment].sampler.GetAddressOf());
            ctx->PSSetSamplers(14u, 1, environmentCache[currentEnvironment].brdfSampler.GetAddressOf());
            ctx->PSSetShaderResources(12u, 1, environmentCache[currentEnvironment].irradianceMap.textureView.GetAddressOf());
            ctx->PSSetShaderResources(13u, 1, environmentCache[currentEnvironment].specularMap.textureView.GetAddressOf());
            ctx->PSSetShaderResources(14u, 1, environmentCache[currentEnvironment].brdfLUT.textureView.GetAddressOf());
        }

        render_mesh_solid(graphics, renderCtx, modelCache[currentModel], pointlight.mesh, translate(identity_mat4(), pointlight.info.viewLightPos.xyz()), viewMatrix, projMatrix);

        if (activeScene > -1)
        {
            submit_scene(graphics, modelCache[currentModel], renderCtx, modelCache[currentModel].scenes[activeScene]);
            //-----------------------------------------------------------------------------
            // update skins
            //-----------------------------------------------------------------------------
            for (int i = 0; i < modelCache[currentModel].nodes.size(); i++)
            {
                mvNode& node = modelCache[currentModel].nodes[i];
                if (node.skin != -1 && node.mesh != -1)
                {
                    unsigned int skeleton = modelCache[currentModel].skins[node.skin].skeleton;
                    if(skeleton != -1)
                        compute_joints(graphics, modelCache[currentModel], modelCache[currentModel].nodes[skeleton].inverseWorldTransform, modelCache[currentModel].skins[node.skin]);
                    else
                        compute_joints(graphics, modelCache[currentModel], viewMatrix, modelCache[currentModel].skins[node.skin]);
                }
            }

            render_scenes(graphics, modelCache[currentModel], renderCtx, viewMatrix, projMatrix);
        }

        if (showSkybox && envMapIndex > -1)
           render_skybox(graphics, renderCtx, modelCache[currentModel], blur ? environmentCache[currentEnvironment].specularMap : environmentCache[currentEnvironment].skyMap, environmentCache[currentEnvironment].sampler.Get(), viewMatrix, projMatrix);

        //-----------------------------------------------------------------------------
        // ui
        //-----------------------------------------------------------------------------
        ctx->OMSetRenderTargets(1, graphics.target.GetAddressOf(), *graphics.targetDepth.GetAddressOf());
        ctx->RSSetViewports(1u, &graphics.viewport);

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

            struct mvCallbackData
            {
                ID3D11BlendState* blendState;
                mvGraphics& graphics;
            };

            auto blendCallback = [](const ImDrawList* parent_list, const ImDrawCmd* cmd) {
                // Setup blend state
                const float blend_factor[4] = { 0.f, 0.f, 0.f, 0.f };
                ((mvCallbackData*)cmd->UserCallbackData)->graphics.imDeviceContext->OMSetBlendState(((mvCallbackData*)cmd->UserCallbackData)->blendState, blend_factor, 0xffffffff);
            };

            static mvCallbackData cdata{renderCtx.finalBlendState, graphics};

            ImGui::GetWindowDrawList()->AddCallback(blendCallback, &cdata);
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
            if (ImGui::Checkbox("Punctual Lighting", (bool*)&graphics.punctualLighting))reloadMaterials = true;
            if (ImGui::Checkbox("Image Based", (bool*)&graphics.imageBasedLighting)) reloadMaterials = true;

            ImGui::Dummy(ImVec2(50.0f, 25.0f));
            ImGui::Text("%s", "Background");
            ImGui::Checkbox("Blur##skybox", &blur);

            ImGui::Dummy(ImVec2(50.0f, 25.0f));
            ImGui::Text("%s", "Directional Light:");
            static float dangle = 0.0f;
            if (ImGui::SliderFloat("X Angle##d", &dangle, -45.0f, 45.0f))
            {
                float zcomponent = sinf(PI * dangle / 180.0f);
                float ycomponent = cosf(PI * dangle / 180.0f);
                directionalLight.info.viewLightDir = { 0.0f, -ycomponent, zcomponent };
            }

            ImGui::Dummy(ImVec2(50.0f, 25.0f));
            ImGui::Text("%s", "Point Light:");
            ImGui::SliderFloat3("Position##o", &pointlight.info.viewLightPos.x, -25.0f, 50.0f);

            ImGui::Dummy(ImVec2(50.0f, 25.0f));
            ImGui::Text("%s", "Extensions");
            if (ImGui::Checkbox("KHR_materials_clearcoat", (bool*)&graphics.clearcoat)) reloadMaterials = true;

            ImGui::Unindent(14.0f);
            ImGui::EndTable();

            ImGui::End();
            ImGui::PopStyleVar(4);
            ImGui::PopStyleColor(2);

            // render imgui
            ImGui::Render();
            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

            // present
            graphics.swapChain->Present(1, 0);
        }
    }

    offscreen.cleanup();

    // Cleanup
    renderCtx.finalBlendState->Release();
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

