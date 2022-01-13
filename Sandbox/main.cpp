#include "main.h"

int main()
{    
    UISession session = create_ui_session();

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

    ID3D11DeviceContext* ctx = GContext->graphics.imDeviceContext.Get();

    // setup asset manager
    mvAssetManager am{};
    initialize_asset_manager(&am);
    Renderer::setup_common_assets(am);
    
    mvRendererContext renderCtx = Renderer::create_renderer_context(am);

    // main camera
    mvCamera camera = create_perspective_camera({ -13.5f, 6.0f, 3.5f }, (f32)M_PI_4, 1.0f, 0.1f, 400.0f);
    //camera.yaw = (f32)M_PI;
    renderCtx.camera = &camera;

    // lights
    mvPointLight pointlight = create_point_light(am);
    mvDirectionalLight directionalLight = create_directional_light(am);

    // passes
    mvOffscreenPass offscreen = mvOffscreenPass(500.0f, 500.0f);
    mvDirectionalShadowPass directionalShadowMap = mvDirectionalShadowPass(am, 4096, 95.0f);
    mvOmniShadowPass omniShadowMap = mvOmniShadowPass(am, 2048);
    omniShadowMap.info.view = create_lookat_view(pointlight.camera);
    //omniShadowMap.info.view = translate(identity_mat4(), { pointlight.info.viewLightPos.x, pointlight.info.viewLightPos.y, pointlight.info.viewLightPos.z });

    mvTimer timer;
    while (true)
    {
        
        if (const auto ecode = process_viewport_events()) break;

        const auto dt = timer.mark() * 1.0f;

        //-----------------------------------------------------------------------------
        // updates
        //-----------------------------------------------------------------------------

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
            }

            else
            {
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
        }
        if (session.changeScene)
        {
            session.changeScene = false;
            b8 cacheFound = false;
            for (int i = 0; i < MODEL_CACHE_SIZE; i++)
            {
                if (session.cachedModelIndex[i] == session.modelIndex)
                {
                    session.cacheIndex = session.cachedModelIndex[i];
                    cacheFound = true;
                    break;
                }
            }

            if (!cacheFound)
            {

                i32 modelToRemove = session.cachedModelIndex[session.cacheRingIndex];

                if (modelToRemove > -1)
                    unload_gltf_assets(am, session.cachedModel[modelToRemove]);

                mvGLTFModel gltfmodel0 = mvLoadGLTF(gltf_directories[session.modelIndex], gltf_models[session.modelIndex]);
                session.cachedModel[session.cacheRingIndex] = load_gltf_assets(am, gltfmodel0);
                mvCleanupGLTF(gltfmodel0);
                session.cachedModelIndex[session.cacheRingIndex] = modelToRemove;
                session.cacheIndex = session.cacheRingIndex;
                session.cacheRingIndex++;
                if (session.cacheRingIndex == MODEL_CACHE_SIZE) session.cacheRingIndex = 0;

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
        // update animations
        //-----------------------------------------------------------------------------
        for (i32 i = 0; i < am.animationCount; i++)
            advance_animations(am, am.animations[i].asset, session.currentTime);

        //-----------------------------------------------------------------------------
        // begin frame
        //-----------------------------------------------------------------------------
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        ID3D11ShaderResourceView* const pSRV[12] = { NULL };
        for(int i = 0; i < 12; i++)
            ctx->PSSetShaderResources(i, 6, pSRV);

        //-----------------------------------------------------------------------------
        // directional shadow pass
        //-----------------------------------------------------------------------------
        ctx->OMSetRenderTargets(0, nullptr, directionalShadowMap.depthView);
        ctx->RSSetViewports(1u, &directionalShadowMap.viewport);
        ctx->RSSetState(directionalShadowMap.rasterizationState);

        i32 activeScene = -1;
        if (session.cacheIndex > -1)
        {
            activeScene = session.cachedModel[session.cacheIndex].defaultScene;
        }

        if (activeScene > -1)
            Renderer::render_scene_shadows(am, am.scenes[activeScene].asset,
                directionalShadowMap.getViewMatrix(), directionalShadowMap.getProjectionMatrix(),
                session.scaleTransform, session.translationTransform);

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
                Renderer::render_scene_shadows(am, am.scenes[activeScene].asset, 
                    camera_matrix, perspective(M_PI_2, 1.0f, 0.5f, 100.0f), 
                    session.scaleTransform, session.translationTransform);
        }

        //-----------------------------------------------------------------------------
        // main pass
        //-----------------------------------------------------------------------------
        ctx->OMSetRenderTargets(1, &offscreen.targetView, offscreen.depthView);
        ctx->RSSetViewports(1u, &offscreen.viewport);

        mvMat4 viewMatrix = create_fps_view(camera);
        mvMat4 projMatrix = create_projection(camera);

        renderCtx.globalInfo.camPos = camera.pos;
        directionalLight.info.viewLightDir = directionalShadowMap.camera.dir;

        // update constant buffers
        update_const_buffer(pointlight.buffer, &pointlight.info);
        update_const_buffer(directionalLight.buffer, &directionalLight.info);
        update_const_buffer(renderCtx.globalInfoBuffer, &renderCtx.globalInfo);
        update_const_buffer(directionalShadowMap.buffer, &directionalShadowMap.info);
        update_const_buffer(omniShadowMap.buffer, &omniShadowMap.info);

        // vertex constant buffers
        ctx->VSSetConstantBuffers(1u, 1u, &directionalShadowMap.buffer.buffer);
        ctx->VSSetConstantBuffers(2u, 1u, &omniShadowMap.buffer.buffer);

        // pixel constant buffers
        ctx->PSSetConstantBuffers(0u, 1u, &pointlight.buffer.buffer);
        ctx->PSSetConstantBuffers(2u, 1u, &directionalLight.buffer.buffer);
        ctx->PSSetConstantBuffers(3u, 1u, &renderCtx.globalInfoBuffer.buffer);

        // samplers
        ctx->PSSetSamplers(5u, 1, &directionalShadowMap.sampler);
        ctx->PSSetSamplers(6u, 1, &omniShadowMap.sampler);

        if (session.activeEnv > -1)
        {
            ctx->PSSetSamplers(7u, 1, &session.cachedEnvironments[session.activeEnv].sampler);
            ctx->PSSetSamplers(8u, 1, &session.cachedEnvironments[session.activeEnv].sampler);
            ctx->PSSetSamplers(9u, 1, &session.cachedEnvironments[session.activeEnv].brdfSampler);
        }

        // textures
        ctx->PSSetShaderResources(5u, 1, &directionalShadowMap.resourceView);
        ctx->PSSetShaderResources(6u, 1, &omniShadowMap.resourceView);

        if (session.activeEnv > -1)
        {
            ctx->PSSetShaderResources(7u, 1, &session.cachedEnvironments[session.activeEnv].irradianceMap.textureView);
            ctx->PSSetShaderResources(8u, 1, &session.cachedEnvironments[session.activeEnv].specularMap.textureView);
        }

        if (session.activeEnv > -1)
        {
            ctx->PSSetShaderResources(9u, 1, &session.cachedEnvironments[session.activeEnv].brdfLUT.textureView);
        }

        Renderer::render_mesh_solid(am, pointlight.mesh, translate(identity_mat4(), pointlight.camera.pos), viewMatrix, projMatrix);

        if (activeScene > -1)
        {
            Renderer::submit_scene(am, renderCtx, am.scenes[activeScene].asset, session.scaleTransform, session.translationTransform);
            //-----------------------------------------------------------------------------
            // update skins
            //-----------------------------------------------------------------------------
            for (i32 i = 0; i < am.nodeCount; i++)
            {
                mvNode& node = am.nodes[i].asset;
                if (node.skin != -1 && node.mesh != -1)
                {
                    u32 skeleton = am.skins[node.skin].asset.skeleton;
                    if(skeleton != -1)
                        compute_joints(am, am.nodes[skeleton].asset.inverseWorldTransform, am.skins[node.skin].asset);
                    else
                        compute_joints(am, identity_mat4(), am.skins[node.skin].asset);
                }
            }

            Renderer::render_scenes(am, renderCtx, viewMatrix, projMatrix);
        }

        if (session.showSkybox && session.activeEnv > -1)
            Renderer::render_skybox(am, session.blur ? session.cachedEnvironments[session.activeEnv].specularMap : session.cachedEnvironments[session.activeEnv].skyMap, session.cachedEnvironments[session.activeEnv].sampler, viewMatrix, projMatrix);

        //-----------------------------------------------------------------------------
        // ui
        //-----------------------------------------------------------------------------
        draw_ui(session, dt, renderCtx, directionalShadowMap, omniShadowMap, offscreen,
            pointlight, directionalLight);
    }

    offscreen.cleanup();
    directionalShadowMap.cleanup();
    omniShadowMap.cleanup();

    // Cleanup
    renderCtx.finalBlendState->Release();
    cleanup_asset_manager(&am);

    for (int i = 0; i < ENV_CACHE_SIZE; i++)
        cleanup_environment(session.cachedEnvironments[i]);

    for (int i = 0; i < MODEL_CACHE_SIZE; i++)
    {
        unload_gltf_assets(am, session.cachedModel[i]);
    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
    destroy_context();
}

