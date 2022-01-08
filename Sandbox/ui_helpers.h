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

#define MODEL_CACHE_SIZE 5
#define ENV_CACHE_SIZE 3

struct UISession
{

    mvViewport* window = nullptr;
    ImVec2 oldContentRegion = ImVec2(500, 500);

    // scenes & models
    i32       activeScene = -1;
    i32       cacheIndex = 0;
    i32       modelIndex = 0;
    i32       cachedModel[MODEL_CACHE_SIZE];
    mvAssetID cachedScenes[MODEL_CACHE_SIZE];

    // environments
    i32           activeEnv = -1;
    i32           envMapIndex = 0;
    i32           envCacheIndex = 0;
    mvEnvironment cachedEnvironments[ENV_CACHE_SIZE];
    i32           cachedEnvironmentsID[ENV_CACHE_SIZE];

    // misc
    f32    currentTime = 0.0f;
    f32    uniformScale = 1.0f;
    mvVec3 translation = { 0.0f, 0.0f, 0.0f };

    // prevent recalculating every frame
    mvMat4 scaleTransform = identity_mat4();
    mvMat4 translationTransform = identity_mat4();

    // flags
    b8 showSkybox = false;
    b8 blur       = true;

    // per frame dirty flags
    bool reloadMaterials      = false;
    bool recreatePrimary      = false;
    bool recreateShadowMapRS  = false;
    bool recreateOShadowMapRS = false;
    bool recreateEnvironment  = true;
    bool changeScene          = false;

};

UISession
create_ui_session()
{
    UISession session{};

    create_context();
    GContext->IO.shaderDirectory = "../../Sandbox/shaders/";
    GContext->IO.resourceDirectory = "../../Resources/";

    session.window = initialize_viewport(1850, 900);
    setup_graphics(*session.window);

    // setup imgui
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui_ImplWin32_Init(session.window->hWnd);
    ImGui_ImplDX11_Init(GContext->graphics.device.Get(), GContext->graphics.imDeviceContext.Get());

    for (int i = 0; i < MODEL_CACHE_SIZE; i++)
    {
        session.cachedModel[i] = -1;
        session.cachedScenes[i] = -1;
    }

    for (int i = 0; i < ENV_CACHE_SIZE; i++)
        session.cachedEnvironmentsID[i] = -1;

    return session;
}

void
end_ui_session(UISession& session)
{
    for (int i = 0; i < ENV_CACHE_SIZE; i++)
        cleanup_environment(session.cachedEnvironments[i]);

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
    destroy_context();
}

void 
blendCallback(const ImDrawList* parent_list, const ImDrawCmd* cmd)
{
    // Setup blend state
    const float blend_factor[4] = { 0.f, 0.f, 0.f, 0.f };
    GContext->graphics.imDeviceContext->OMSetBlendState((ID3D11BlendState*)cmd->UserCallbackData, blend_factor, 0xffffffff);
}

void
do_updates(f32 dt, mvAssetManager& am, UISession& session, mvDirectionalShadowPass& directionalShadowMap, mvOmniShadowPass omniShadowMap,
    mvOffscreenPass& offscreen)
{
    session.currentTime += dt;

    if (session.window->resized)
    {
        recreate_swapchain(session.window->width, session.window->height);
        session.window->resized = false;
    }

    if (session.reloadMaterials)
    {
        reload_materials(&am);
        session.reloadMaterials = false;
    }

    if (session.recreatePrimary)
    {
        offscreen.recreate();
        session.recreatePrimary = false;
    }

    if (session.recreateShadowMapRS)
    {
        directionalShadowMap.recreate();
        session.recreateShadowMapRS = false;
    }

    if (session.recreateOShadowMapRS)
    {
        omniShadowMap.recreate();
        session.recreateOShadowMapRS = false;
    }

    if (session.recreateEnvironment)
    {
        session.recreateEnvironment = false;
        if (session.envMapIndex == 0)
        {
            session.showSkybox = false;
            return;
        }

        session.showSkybox = true;
        b8 cacheFound = false;
        for (int i = 0; i < ENV_CACHE_SIZE; i++)
        {
            if (session.cachedEnvironmentsID[i] == session.envMapIndex)
            {
                session.activeEnv = i;
                cacheFound = true;
                break;
            }
        }

        if (!cacheFound)
        {
            std::string newMap = "../../data/glTF-Sample-Environments/" + std::string(env_maps[session.envMapIndex]) + ".hdr";
            cleanup_environment(session.cachedEnvironments[session.envCacheIndex]);
            session.cachedEnvironments[session.envCacheIndex] = create_environment(newMap, 1024, 1024, 1.0f, 7);
            session.cachedEnvironmentsID[session.envCacheIndex] = session.envMapIndex;
            session.activeEnv = session.envCacheIndex;
            session.envCacheIndex++;
            if (session.envCacheIndex == ENV_CACHE_SIZE) session.envCacheIndex = 0;
        }
    }
    if (session.changeScene)
    {
        session.changeScene = false;
        b8 cacheFound = false;
        for (int i = 0; i < MODEL_CACHE_SIZE; i++)
        {
            if (session.cachedModel[i] == session.modelIndex)
            {
                session.activeScene = session.cachedScenes[i];
                cacheFound = true;
                break;
            }
        }

        if (!cacheFound)
        {

            mvAssetID sceneToRemove = session.cachedScenes[session.cacheIndex];

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

            mvGLTFModel gltfmodel0 = mvLoadGLTF(gltf_directories[session.modelIndex], gltf_models[session.modelIndex]);
            session.activeScene = load_gltf_assets(am, gltfmodel0);
            mvCleanupGLTF(gltfmodel0);
            session.cachedScenes[session.cacheIndex] = session.activeScene;
            session.cachedModel[session.cacheIndex] = session.modelIndex;
            session.cacheIndex++;
            if (session.cacheIndex == MODEL_CACHE_SIZE) session.cacheIndex = 0;

        }
    }
}

void 
draw_ui(UISession& session, f32 dt, mvRendererContext& renderCtx, 
    mvDirectionalShadowPass& directionalShadowMap, mvOmniShadowPass omniShadowMap, 
    mvOffscreenPass& offscreen, mvPointLight& pointlight, mvDirectionalLight& directionalLight)
{
    static ID3D11DeviceContext* ctx = GContext->graphics.imDeviceContext.Get();

    ctx->OMSetRenderTargets(1, GContext->graphics.target.GetAddressOf(), *GContext->graphics.targetDepth.GetAddressOf());
    ctx->RSSetViewports(1u, &GContext->graphics.viewport);

    // styling to fill viewport 
    ImGui::SetNextWindowBgAlpha(1.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f); // to prevent main window corners from showing
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(14.0f, 5.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_TableBorderLight, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_TableBorderStrong, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::SetNextWindowSize(ImVec2((float)session.window->width, (float)session.window->height));
    static ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoDecoration;

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
        if (ImGui::DragFloat("Scale", &session.uniformScale, 0.1f, 0.001f))
        {
            session.scaleTransform = scale(identity_mat4(), mvVec3{ session.uniformScale, session.uniformScale, session.uniformScale });
        }
        if (ImGui::DragFloat3("Translate", &session.translation.x, 1.0f, -200.0f, 200.0f))
        {
            session.translationTransform = translate(identity_mat4(), session.translation);
        }
        ImGui::ColorEdit3("Ambient Color", &renderCtx.globalInfo.ambientColor.x);

        ImGui::Dummy(ImVec2(50.0f, 25.0f));
        ImGui::Text("%s", "Directional Shadows:");
        if (ImGui::Checkbox("Use Direct Shadows", (bool*)&GContext->IO.directionalShadows))session.reloadMaterials = true;
        if (GContext->IO.directionalShadows && GContext->IO.punctualLighting)
        {
            if (ImGui::Checkbox("Backface Cull", &directionalShadowMap.backface)) session.recreateShadowMapRS = true;
            if (ImGui::DragInt("Depth Bias##d", &directionalShadowMap.depthBias, 1.0f, 0, 100)) session.recreateShadowMapRS = true;
            if (ImGui::DragFloat("Slope Bias##d", &directionalShadowMap.slopeBias, 0.1f, 0.1f, 10.0f)) session.recreateShadowMapRS = true;
        }

        ImGui::SliderInt("PCF Range", &renderCtx.globalInfo.pcfRange, 0, 5);

        ImGui::Dummy(ImVec2(50.0f, 25.0f));
        ImGui::Text("%s", "OmniDirectional Shadows:");
        if (ImGui::Checkbox("Use Omni Shadows", (bool*)&GContext->IO.omniShadows))session.reloadMaterials = true;
        if (GContext->IO.omniShadows && GContext->IO.punctualLighting)
        {
            if (ImGui::DragInt("Depth Bias", &omniShadowMap.depthBias, 1.0f, 0, 100)) session.recreateOShadowMapRS = true;
            if (ImGui::DragFloat("Slope Bias", &omniShadowMap.slopeBias, 0.1f, 0.1f, 10.0f)) session.recreateOShadowMapRS = true;
        }

        ImGui::Dummy(ImVec2(50.0f, 25.0f));
        ImGui::Text("%s", "Settings:");
        if (ImGui::Checkbox("Punctual Lighting", (bool*)&GContext->IO.punctualLighting))session.reloadMaterials = true;
        if (ImGui::Checkbox("Image Based", (bool*)&GContext->IO.imageBasedLighting)) session.reloadMaterials = true;
        if (ImGui::Checkbox("KHR_materials_clearcoat", (bool*)&GContext->IO.clearcoat)) session.reloadMaterials = true;

        //-----------------------------------------------------------------------------
        // center panel
        //-----------------------------------------------------------------------------
        ImGui::TableSetColumnIndex(1);
        ImGui::GetForegroundDrawList()->AddText(ImVec2(15, 7),
            ImColor(0.0f, 1.0f, 0.0f), std::string(std::to_string(ImGui::GetIO().Framerate) + " FPS").c_str());

        if (ImGui::IsWindowHovered())
            update_fps_camera(*renderCtx.camera, dt, 12.0f, 0.004f);

        ImVec2 contentSize = ImGui::GetContentRegionAvail();
        offscreen.viewport = { 0.0f, 0.0f, contentSize.x, contentSize.y, 0.0f, 1.0f };
        renderCtx.camera->aspectRatio = offscreen.viewport.Width / offscreen.viewport.Height;

        ImGui::GetWindowDrawList()->AddCallback(blendCallback, renderCtx.finalBlendState);
        ImGui::Image(offscreen.resourceView, contentSize, ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1), ImVec4(0, 0, 0, 0));
        if (!(contentSize.x == session.oldContentRegion.x && contentSize.y == session.oldContentRegion.y))
            session.recreatePrimary = true;
        session.oldContentRegion = contentSize;
        ImGui::GetWindowDrawList()->AddCallback(ImDrawCallback_ResetRenderState, nullptr);

        //-----------------------------------------------------------------------------
        // right panel
        //-----------------------------------------------------------------------------
        ImGui::TableSetColumnIndex(2);

        ImGui::Checkbox("Blur##skybox", &session.blur);
        if (ImGui::ListBox("Environment##separate", &session.envMapIndex, env_maps, 11, 11))
        {
            session.recreateEnvironment = true;
        }

        ImGui::Dummy(ImVec2(50.0f, 25.0f));
        ImGui::Indent(14.0f);

        if (ImGui::ListBox("Model##separate", &session.modelIndex, gltf_names, 80 + 36 + 59, 20))
        {
            session.changeScene = true;
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
                -directionalShadowMap.camera.width / 2.0f,
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