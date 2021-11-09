#include "mv3D.h"

mv_internal const char* gltfModel = "FlightHelmet";
mv_internal const char* sponzaPath = "C:/dev/MarvelAssets/Sponza/";
mv_internal const char* gltfPath = "C://dev//glTF-Sample-Models//2.0//";
mv_internal b8 loadGLTF = false;
mv_internal b8 loadSponza = true;
mv_internal f32 shadowWidth = 100.0f;

mvGLTFModel LoadTestModel(const char* name);

int main()
{
    if (loadSponza)
        shadowWidth = 150.0f;

    mvCreateContext();
    GContext->IO.shaderDirectory = "../../mv3D/shaders/";
    GContext->IO.resourceDirectory = "../../Resources/";

    Renderer::mvStartRenderer();

    mvAssetManager am{};
    mvInitializeAssetManager(&am);

    // assets & meshes
    if (loadGLTF)
    {
        mvGLTFModel gltfmodel = LoadTestModel(gltfModel);
        mvLoadGLTFAssets(am, gltfmodel);
        mvCleanupGLTF(gltfmodel);
    }
    if (loadSponza) mvLoadOBJAssets(am, sponzaPath, "sponza");

    // scenes
    mvScene scene = mvCreateScene();

    // main camera
    mvCamera camera{};
    camera.pos = { -13.5f, 6.0f, 3.5f };
    camera.front = { 0.0f, 0.0f, -1.0f };
    camera.pitch = 0.0f;
    camera.yaw = 0.0f;
    camera.aspect = GContext->viewport.width / GContext->viewport.height;

    // lights
    mvPointLight light = mvCreatePointLight(&am, { 0.0f, 15.0f, 0.0f });
    mvDirectionLight dlight = mvCreateDirectionLight({ 0.0f, -1.0f, 0.0f });

    // shadows

    mvShadowCamera dshadowCamera = mvCreateShadowCamera();

    // passes
    mvPass lambertian
    {
        GContext->graphics.target.GetAddressOf(),
        GContext->graphics.targetDepth.GetAddressOf(),
        nullptr,
        {0.0f, 0.0f, (f32)GContext->viewport.width, (f32)GContext->viewport.height, 0.0f, 1.0f},
        nullptr
    };

    mvShadowMap directionalShadowMap = mvCreateShadowMap(4000, shadowWidth);
    mvPass directionalShadowPass
    {
        nullptr,
        directionalShadowMap.shadowDepthView.GetAddressOf(),
        nullptr,
        {0.0f, 0.0f, (f32)directionalShadowMap.shadowMapDimension, (f32)directionalShadowMap.shadowMapDimension, 0.0f, 1.0f},
        directionalShadowMap.shadowRasterizationState.GetAddressOf()
    };

    mvShadowCubeMap omniShadowMap = mvCreateShadowCubeMap(2000);
    mvPass omniShadowPasses[6];
    for (u32 i = 0; i < 6; i++)
    {
        omniShadowPasses[i].target = nullptr;
        omniShadowPasses[i].depthStencil = omniShadowMap.shadowDepthViews[i].GetAddressOf();
        omniShadowPasses[i].shaderResource = nullptr;
        omniShadowPasses[i].viewport = { 0.0f, 0.0f, (f32)omniShadowMap.shadowMapDimension, (f32)omniShadowMap.shadowMapDimension, 0.0f, 1.0f };
        omniShadowPasses[i].rasterizationState = omniShadowMap.shadowRasterizationState.GetAddressOf();
    }

    mvSkyboxPass skyboxPass = mvCreateSkyboxPass(&am, "../../Resources/Skybox");

    dshadowCamera.info.directShadowView = mvLookAtRH(directionalShadowMap.camera.pos, directionalShadowMap.camera.pos + directionalShadowMap.camera.dir, directionalShadowMap.camera.up);
    dshadowCamera.info.directShadowProjection = mvOrthoRH(directionalShadowMap.camera.left, directionalShadowMap.camera.right, directionalShadowMap.camera.bottom, directionalShadowMap.camera.top, directionalShadowMap.camera.nearZ, directionalShadowMap.camera.farZ);

    mvTimer timer;
    while (true)
    {
        const auto dt = timer.mark() * 1.0f;

        if (const auto ecode = mvProcessViewportEvents()) break;

        if (GContext->viewport.resized)
        {
            Renderer::mvResize();
            GContext->viewport.resized = false;
            camera.aspect = (float)GContext->viewport.width / (float)GContext->viewport.height;
            lambertian.viewport.Width = GContext->viewport.width;
            lambertian.viewport.Height = GContext->viewport.height;
            skyboxPass.basePass.viewport.Width = GContext->viewport.width;
            skyboxPass.basePass.viewport.Height = GContext->viewport.height;
        }

        //-----------------------------------------------------------------------------
        // clear passes
        //-----------------------------------------------------------------------------
        Renderer::mvClearPass(lambertian);
        Renderer::mvClearPass(directionalShadowPass);
        for (u32 i = 0; i < 6; i++)
            Renderer::mvClearPass(omniShadowPasses[i]);

        //-----------------------------------------------------------------------------
        // begin frame
        //-----------------------------------------------------------------------------
        Renderer::mvBeginFrame();

        // controls
        ImGui::Begin("Direction Light");
        if (ImGui::SliderFloat3("DLight", &directionalShadowMap.camera.dir.x, -1.0f, 1.0f))
        {
            dshadowCamera.info.directShadowView = mvLookAtRH(directionalShadowMap.camera.pos, directionalShadowMap.camera.pos + directionalShadowMap.camera.dir, directionalShadowMap.camera.up);
        }
        ImGui::End();

        mvShowControls(light);
        mvShowControls(scene);

        //-----------------------------------------------------------------------------
        // directional shadow pass
        //-----------------------------------------------------------------------------
        Renderer::mvBeginPass(directionalShadowPass);

        for (int i = 0; i < am.sceneCount; i++)
            Renderer::mvRenderSceneShadows(am, am.scenes[i].scene, mvCreateOrthoView(directionalShadowMap.camera), mvCreateOrthoProjection(directionalShadowMap.camera));

        Renderer::mvEndPass();

        //-----------------------------------------------------------------------------
        // omni shadow pass
        //-----------------------------------------------------------------------------
        for (u32 i = 0; i < 6; i++)
        {
            Renderer::mvBeginPass(omniShadowPasses[i]);

            mvVec3 look_target = light.camera.pos + omniShadowMap.cameraDirections[i];
            mvMat4 camera_matrix = mvLookAtLH(light.camera.pos, look_target, omniShadowMap.cameraUps[i]);

            for (int i = 0; i < am.sceneCount; i++)
                Renderer::mvRenderSceneShadows(am, am.scenes[i].scene, camera_matrix, mvPerspectiveLH(M_PI_2, 1.0f, 0.5f, 100.0f));

            Renderer::mvEndPass();
        }

        //-----------------------------------------------------------------------------
        // main pass
        //-----------------------------------------------------------------------------
        Renderer::mvBeginPass(lambertian);

        mvBindSlot_bVS(1u, dshadowCamera, mvCreateLookAtView(light.camera), mvCreateOrthoView(directionalShadowMap.camera), mvCreateOrthoProjection(directionalShadowMap.camera));

        mvUpdateCameraFPSCamera(camera, dt, 12.0f, 0.004f);
        mvMat4 viewMatrix = mvCreateFPSView(camera);
        mvMat4 projMatrix = mvCreateLookAtProjection(camera);

        mvBindSlot_bPS(0u, light, viewMatrix);
        mvBindSlot_bPS(2u, dlight, viewMatrix);
        mvBindSlot_bPS(3u, scene);

        mvBindSlot_tsPS(directionalShadowMap, 3u, 1u);
        mvBindSlot_tsPS(omniShadowMap, 4u, 2u);

        Renderer::mvRenderMesh(am, light.mesh, mvTranslate(mvIdentityMat4(), light.camera.pos), viewMatrix, projMatrix);

        for (int i = 0; i < am.sceneCount; i++)
            Renderer::mvRenderScene(am, am.scenes[i].scene, viewMatrix, projMatrix);

        Renderer::mvEndPass();


        //-----------------------------------------------------------------------------
        // skybox pass
        //-----------------------------------------------------------------------------
        Renderer::mvBeginPass(skyboxPass.basePass);
        mvBindSlot_bPS(0u, scene);
        Renderer::mvRenderSkybox(am, skyboxPass, viewMatrix, projMatrix);
        Renderer::mvEndPass();

        //-----------------------------------------------------------------------------
        // end frame & present
        //-----------------------------------------------------------------------------
        Renderer::mvEndFrame();
        Renderer::mvPresent();
    }

    mvCleanupAssetManager(&am);
    Renderer::mvStopRenderer();
    mvDestroyContext();
}

mvGLTFModel LoadTestModel(const char* name)
{
    std::string root = gltfPath + std::string(name) + "//glTF//";
    std::string file = root + std::string(name) + ".gltf";
    return mvLoadGLTF(root.c_str(), file.c_str());
}
