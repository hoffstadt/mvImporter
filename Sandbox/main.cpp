#include "mv3D.h"

mv_internal const char* gltfModel = "FlightHelmet";
mv_internal const char* sponzaPath = "C:/dev/MarvelAssets/Sponza/";
mv_internal const char* gltfPath = "C://dev//glTF-Sample-Models//2.0//";
mv_internal b8 loadGLTF = false;
mv_internal b8 loadSponza = false;
mv_internal f32 shadowWidth = 100.0f;

mvGLTFModel LoadTestModel(const char* name);

int main()
{
    if (loadSponza)
        shadowWidth = 150.0f;

    mvCreateContext();
    GContext->IO.shaderDirectory = "../../mv3D/shaders/";
    GContext->IO.resourceDirectory = "../../Resources/";

    mvRenderer_StartRenderer();

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
    camera.pitch = 0.0f;
    camera.yaw = M_PI;
    camera.aspect = GContext->viewport.width / GContext->viewport.height;

    // lights
    mvPointLight light = mvCreatePointLight(&am, { 0.0f, 15.0f, 0.0f });
    mvDirectionLight dlight = mvCreateDirectionLight({ 0.0f, -1.0f, 0.0f });
    mvMat4 lightTransform = mvTranslate(mvIdentityMat4(), mvVec3{ 0.0f, 15.0f, 0.0f });

    // shadows
    mvOrthoCamera orthoCamera{};
    orthoCamera.dir = { 0.0f, 1.0f, 0.0f };
    orthoCamera.up = { 1.0f, 0.0f, 0.0f };
    orthoCamera.pos.y = shadowWidth/2.0f;
    orthoCamera.left = -shadowWidth;
    orthoCamera.right = shadowWidth;
    orthoCamera.bottom = -shadowWidth;
    orthoCamera.top = shadowWidth;
    orthoCamera.nearZ = -shadowWidth-1.0f;
    orthoCamera.farZ = shadowWidth+1.0f;

    mvCamera perspecCamera{};
    perspecCamera.pos = { light.info.viewLightPos.x, light.info.viewLightPos.y, light.info.viewLightPos.z };
    perspecCamera.aspect = 1.0f;
    perspecCamera.yaw = 0.0f;

    mvShadowCamera dshadowCamera = mvCreateShadowCamera();

    mvShadowMap directionalShadowMap = mvCreateShadowMap(4000);
    mvShadowCubeMap omniShadowMap = mvCreateShadowCubeMap(2000);
    
    // passes
    mvPass lambertian = mvCreateMainPass();
    mvPass directionalShadowPass = mvCreateShadowPass(directionalShadowMap);
    mvPass omniShadowPasses[6];
    for (u32 i = 0; i < 6; i++)
        omniShadowPasses[i] = mvCreateShadowPass(omniShadowMap, i);

    mvSkyboxPass skyboxPass = mvCreateSkyboxPass(&am, "../../Resources/Skybox");

    mvMesh texturedQuad = mvCreateTexturedQuad(am, 30.0f);
    mvVec3 texturedQuad_pos = { 5.0f, -3.0f, 5.0f };
    mvMat4 texturedQuadTrans = mvTranslate(mvIdentityMat4(), texturedQuad_pos) * mvRotate(mvIdentityMat4(), -M_PI_2, mvVec3{1.0f, 0.0f, 0.0f});
    //mvRegistryMeshAsset(&am, texturedQuad);

    mvMesh room = mvCreateRoom(am, 15.0f);
    mvMat4 roomTransform = mvTranslate(mvIdentityMat4(), mvVec3{-5.0f, -3.0f, -5.0f});

    mvMesh texturedCube = mvCreateTexturedCube(am, 1.0f);
    mvVec3 texturedCube_pos = { 5.0f, 5.0f, 5.0f };
    mvVec3 texturedCube_rot = { M_PI_4, M_PI_4, M_PI_4 };
    mvMat4 texturedCubeTrans = mvTranslate(mvIdentityMat4(), texturedCube_pos) * mvRotate(mvIdentityMat4(), M_PI_4, mvVec3{ 1.0f, 1.0f, 1.0f });
    texturedCube.diffuseTexture = mvGetTextureAsset(&am, "../../Resources/test_image.png");

    dshadowCamera.info.directShadowView = mvLookAtRH(orthoCamera.pos, orthoCamera.pos + orthoCamera.dir, orthoCamera.up);
    dshadowCamera.info.directShadowProjection = mvOrthoRH(orthoCamera.left, orthoCamera.right, orthoCamera.bottom, orthoCamera.top, orthoCamera.nearZ, orthoCamera.farZ);

    mvTimer timer;
    while (true)
    {
        const auto dt = timer.mark() * 1.0f;

        texturedCube_rot.x += dt;
        texturedCube_rot.y += dt;
        texturedCubeTrans = mvTranslate(mvIdentityMat4(), texturedCube_pos) 
            * mvRotate(mvIdentityMat4(), texturedCube_rot.x, mvVec3{ 1.0f, 0.0f, 0.0f }) 
            * mvRotate(mvIdentityMat4(), texturedCube_rot.y, mvVec3{ 0.0f, 1.0f, 0.0f });

        if (const auto ecode = mvProcessViewportEvents()) break;

        if (GContext->viewport.resized)
        {
            mvRenderer_Resize();
            GContext->viewport.resized = false;
            camera.aspect = (float)GContext->viewport.width / (float)GContext->viewport.height;
            lambertian.viewport.Width = GContext->viewport.width;
            lambertian.viewport.Height = GContext->viewport.height;
            skyboxPass.basePass.viewport.Width = GContext->viewport.width;
            skyboxPass.basePass.viewport.Height = GContext->viewport.height;
        }

        // for now, we will just use imgui's input
        if (ImGui::GetIO().KeysDown['W']) mvTranslateCamera(camera, 0.0f, 0.0f, dt);
        if (ImGui::GetIO().KeysDown['S']) mvTranslateCamera(camera, 0.0f, 0.0f, -dt);
        if (ImGui::GetIO().KeysDown['D']) mvTranslateCamera(camera, dt, 0.0f, 0.0f);
        if (ImGui::GetIO().KeysDown['A']) mvTranslateCamera(camera, -dt, 0.0f, 0.0f);
        if (ImGui::GetIO().KeysDown['R']) mvTranslateCamera(camera, 0.0f, dt, 0.0f);
        if (ImGui::GetIO().KeysDown['F']) mvTranslateCamera(camera, 0.0f, -dt, 0.0f);
        if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && !ImGui::GetIO().WantCaptureMouse) mvRotateCamera(camera, ImGui::GetIO().MouseDelta.x, ImGui::GetIO().MouseDelta.y);

        mvMat4 viewMatrix = mvBuildCameraMatrix(camera);
        mvMat4 projMatrix = mvBuildProjectionMatrix(camera);
        mv_local_persist mvMat4 identityMat = mvIdentityMat4();

        //-----------------------------------------------------------------------------
        // clear passes
        //-----------------------------------------------------------------------------
        mvRenderer_ClearPass(lambertian);
        mvRenderer_ClearPass(directionalShadowPass);
        for (u32 i = 0; i < 6; i++)
            mvRenderer_ClearPass(omniShadowPasses[i]);

        //-----------------------------------------------------------------------------
        // begin frame
        //-----------------------------------------------------------------------------
        mvRenderer_BeginFrame();

        // controls
        ImGui::Begin("Light Controls");
        if (ImGui::SliderFloat3("Position", &light.info.viewLightPos.x, -25.0f, 50.0f))
        {
            perspecCamera.pos = { light.info.viewLightPos.x, light.info.viewLightPos.y, light.info.viewLightPos.z };
            lightTransform = mvTranslate(mvIdentityMat4(), perspecCamera.pos);
        }
        if (ImGui::SliderFloat3("DLight", &orthoCamera.dir.x, -1.0f, 1.0f))
        {
            dshadowCamera.info.directShadowView = mvLookAtRH(orthoCamera.pos, orthoCamera.pos + orthoCamera.dir, orthoCamera.up);
        }
        ImGui::End();

        //-----------------------------------------------------------------------------
        // directional shadow pass
        //-----------------------------------------------------------------------------
        mvRenderer_BeginPass(directionalShadowPass);

        //mvRenderer_RenderMeshShadows(am, room, roomTransform, dshadowCamera.info.directShadowView, dshadowCamera.info.directShadowProjection);
        mvRenderer_RenderMeshShadows(am, texturedQuad, texturedQuadTrans, mvBuildCameraMatrix(orthoCamera), mvBuildProjectionMatrix(orthoCamera));
        mvRenderer_RenderMeshShadows(am, texturedCube, texturedCubeTrans, mvBuildCameraMatrix(orthoCamera), mvBuildProjectionMatrix(orthoCamera));

        for (int i = 0; i < am.sceneCount; i++)
             mvRenderer_RenderSceneShadows(am, am.scenes[i].scene, mvBuildCameraMatrix(orthoCamera), mvBuildProjectionMatrix(orthoCamera));

        mvRenderer_EndPass();

        //-----------------------------------------------------------------------------
        // omni shadow pass
        //-----------------------------------------------------------------------------
        for (u32 i = 0; i < 6; i++)
        {
            mvRenderer_BeginPass(omniShadowPasses[i]);

            mvVec3 look_target = perspecCamera.pos + omniShadowMap.cameraDirections[i];
            mvMat4 camera_matrix = mvLookAtLH(perspecCamera.pos, look_target, omniShadowMap.cameraUps[i]);

            //mvRenderer_RenderMeshShadows(am, room, roomTransform, camera_matrix, mvPerspectiveLH(M_PI_2, 1.0f, 0.5f, 100.0f));
            mvRenderer_RenderMeshShadows(am, texturedQuad, texturedQuadTrans, camera_matrix, mvPerspectiveLH(M_PI_2, 1.0f, 0.5f, 100.0f));
            mvRenderer_RenderMeshShadows(am, texturedCube, texturedCubeTrans, camera_matrix, mvPerspectiveLH(M_PI_2, 1.0f, 0.5f, 100.0f));

            for (int i = 0; i < am.sceneCount; i++)
                    mvRenderer_RenderSceneShadows(am, am.scenes[i].scene, camera_matrix, mvPerspectiveLH(M_PI_2, 1.0f, 0.5f, 100.0f));

            mvRenderer_EndPass();
        }

        //-----------------------------------------------------------------------------
        // main pass
        //-----------------------------------------------------------------------------
        mvRenderer_BeginPass(lambertian);

        mvBindSlot_bVS(1u, dshadowCamera, mvBuildCameraMatrix(perspecCamera), mvBuildCameraMatrix(orthoCamera), mvBuildProjectionMatrix(orthoCamera));

        mvBindSlot_bPS(0u, light, viewMatrix);        
        mvBindSlot_bPS(2u, dlight, viewMatrix);       
        mvBindSlot_bPS(3u, scene);                    
                                                    
        mvBindSlot_tsPS(directionalShadowMap, 3u, 1u);
        mvBindSlot_tsPS(omniShadowMap, 4u, 2u);       

        //mvRenderer_RenderMesh(am, room, roomTransform, viewMatrix, projMatrix);
        mvRenderer_RenderMesh(am, texturedQuad, texturedQuadTrans, viewMatrix, projMatrix);
        mvRenderer_RenderMesh(am, texturedCube, texturedCubeTrans, viewMatrix, projMatrix);
        mvRenderer_RenderMesh(am, *light.mesh, lightTransform, viewMatrix, projMatrix);

        for (int i = 0; i < am.sceneCount; i++)
                mvRenderer_RenderScene(am, am.scenes[i].scene, viewMatrix, projMatrix);

        mvRenderer_EndPass();


        //-----------------------------------------------------------------------------
        // skybox pass
        //-----------------------------------------------------------------------------
        mvRenderer_BeginPass(skyboxPass.basePass);
        mvBindSlot_bPS(0u, scene);
        mvRenderer_RenderSkybox(am, skyboxPass, viewMatrix, projMatrix);
        mvRenderer_EndPass();

        //-----------------------------------------------------------------------------
        // end frame & present
        //-----------------------------------------------------------------------------
        mvRenderer_EndFrame();
        mvRenderer_Present();
    }

    mvCleanupAssetManager(&am);
    mvRenderer_StopRenderer();
    mvDestroyContext();
}

mvGLTFModel LoadTestModel(const char* name)
{
    std::string root = gltfPath + std::string(name) + "//glTF//";
    std::string file = root + std::string(name) + ".gltf";
    return mvLoadGLTF(root.c_str(), file.c_str());
}
