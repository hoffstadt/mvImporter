#include "helpers.h"

// TODO: make most of these runtime options
static const char* gltfModel = "Cerberus";
//static const char* gltfModel = "Lantern";
static f32         shadowWidth = 75.0f;
static int         initialWidth = 1850;
static int         initialHeight = 900;
static ImVec2      oldContentRegion = ImVec2(500, 500);

int main()
{

    mvCreateContext();
    GContext->IO.shaderDirectory = "../../Sandbox/shaders/";
    GContext->IO.resourceDirectory = "../../Resources/";

    mvViewport* window = mvInitializeViewport(initialWidth, initialHeight);
    mvSetupGraphics(*window);
    ID3D11DeviceContext* ctx = GContext->graphics.imDeviceContext.Get();

    // setup imgui
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui_ImplWin32_Init(window->hWnd);
    ImGui_ImplDX11_Init(GContext->graphics.device.Get(), GContext->graphics.imDeviceContext.Get());

    mvAssetManager am{};
    mvInitializeAssetManager(&am);

    // assets & meshes
    mvGLTFModel gltfmodel = LoadTestModel(gltfModel);
    mvLoadGLTFAssets(am, gltfmodel);
    mvCleanupGLTF(gltfmodel);

    // main camera
    mvCamera camera{};
    camera.pos = { -13.5f, 6.0f, 3.5f };
    camera.front = { 0.0f, 0.0f, -1.0f };
    camera.pitch = 0.0f;
    camera.yaw = 0.0f;
    camera.aspect = 500.0f / 500.0f;



    // helpers
    mvShadowMap directionalShadowMap = mvShadowMap(4096, shadowWidth);
    mvShadowCubeMap omniShadowMap = mvShadowCubeMap(2048);
    mvSkybox skybox = mvSkybox();
    mvPointLight pointlight = mvPointLight(am);
    //omniShadowMap.info.view = mvCreateLookAtView(pointlight.camera);

    // framework constant buffers
    DirectionLightInfo directionLightInfo{};
    mvConstBuffer directionLightBuffer = mvCreateConstBuffer(&directionLightInfo, sizeof(DirectionLightInfo));

    GlobalInfo globalInfo{};
    mvConstBuffer globalInfoBuffer = mvCreateConstBuffer(&globalInfo, sizeof(GlobalInfo));

    mvOffscreen offscreen = mvOffscreen(500.0f, 500.0f);
    mvTimer timer;
    while (true)
    {
        const auto dt = timer.mark() * 1.0f;

        if (const auto ecode = mvProcessViewportEvents()) break;

        if (window->resized)
        {
            mvRecreateSwapChain(window->width, window->height);
            window->resized = false;
        }

        //-----------------------------------------------------------------------------
        // clear targets
        //-----------------------------------------------------------------------------
        static float backgroundColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
        static float backgroundColor2[] = { 0.1f, 0.1f, 0.1f, 1.0f };
        ctx->ClearRenderTargetView(*GContext->graphics.target.GetAddressOf(), backgroundColor);
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
            Renderer::mvRenderSceneShadows(am, am.scenes[i].scene, directionalShadowMap.getViewMatrix(), directionalShadowMap.getProjectionMatrix());

        //-----------------------------------------------------------------------------
        // omni shadow pass
        //-----------------------------------------------------------------------------
        ctx->RSSetViewports(1u, &omniShadowMap.viewport);
        ctx->RSSetState(directionalShadowMap.rasterizationState);

        for (u32 i = 0; i < 6; i++)
        {
            ctx->OMSetRenderTargets(0, nullptr, omniShadowMap.depthView[i]);
            mvVec3 look_target = pointlight.camera.pos + omniShadowMap.cameraDirections[i];
            mvMat4 camera_matrix = mvLookAtLH(pointlight.camera.pos, look_target, omniShadowMap.cameraUps[i]);

            for (int i = 0; i < am.sceneCount; i++)
                Renderer::mvRenderSceneShadows(am, am.scenes[i].scene, camera_matrix, mvPerspectiveLH(M_PI_2, 1.0f, 0.5f, 100.0f));
        }

        //-----------------------------------------------------------------------------
        // offscreen pass
        //-----------------------------------------------------------------------------
        ctx->OMSetRenderTargets(1, &offscreen.targetView, offscreen.depthView);
        ctx->RSSetViewports(1u, &offscreen.viewport);

        mvMat4 viewMatrix = mvCreateFPSView(camera);
        mvMat4 projMatrix = mvCreateLookAtProjection(camera);

        {
            mvVec4 posCopy = pointlight.info.viewLightPos;

            mvVec4 out = viewMatrix * pointlight.info.viewLightPos;
            pointlight.info.viewLightPos.x = out.x;
            pointlight.info.viewLightPos.y = out.y;
            pointlight.info.viewLightPos.z = out.z;

            mvUpdateConstBuffer(pointlight.buffer, &pointlight.info);
            pointlight.info.viewLightPos = posCopy;
        }

        {
            directionLightInfo.viewLightDir = directionalShadowMap.camera.dir;
            mvVec3 posCopy = directionLightInfo.viewLightDir;

            mvVec4 out = viewMatrix * mvVec4{
                directionLightInfo.viewLightDir.x,
                directionLightInfo.viewLightDir.y,
                directionLightInfo.viewLightDir.z,
                0.0f };
            directionLightInfo.viewLightDir.x = out.x;
            directionLightInfo.viewLightDir.y = out.y;
            directionLightInfo.viewLightDir.z = out.z;

            mvUpdateConstBuffer(directionLightBuffer, &directionLightInfo);
            directionLightInfo.viewLightDir = posCopy;
        }

        // update constant buffers
        mvUpdateConstBuffer(globalInfoBuffer, &globalInfo);
        mvUpdateConstBuffer(directionalShadowMap.buffer, &directionalShadowMap.info);
        mvUpdateConstBuffer(omniShadowMap.buffer, &omniShadowMap.info);

        // vertex constant buffers
        ctx->VSSetConstantBuffers(1u, 1u, &directionalShadowMap.buffer.buffer);
        ctx->VSSetConstantBuffers(2u, 1u, &omniShadowMap.buffer.buffer);

        // pixel constant buffers
        ctx->PSSetConstantBuffers(0u, 1u, &pointlight.buffer.buffer);
        ctx->PSSetConstantBuffers(2u, 1u, &directionLightBuffer.buffer);
        ctx->PSSetConstantBuffers(3u, 1u, &globalInfoBuffer.buffer);

        // samplers
        ctx->PSSetSamplers(1u, 1, &directionalShadowMap.sampler);
        ctx->PSSetSamplers(2u, 1, &omniShadowMap.sampler);

        // textures
        ctx->PSSetShaderResources(3u, 1, &directionalShadowMap.resourceView);
        ctx->PSSetShaderResources(4u, 1, &omniShadowMap.resourceView);

        // render light mesh
        {
            mvSetPipelineState(pointlight.pipeline);

            mvTransforms transforms{};
            transforms.model = mvTranslate(mvIdentityMat4(), pointlight.camera.pos);
            transforms.modelView = viewMatrix * transforms.model;
            transforms.modelViewProjection = projMatrix * viewMatrix * transforms.model;

            D3D11_MAPPED_SUBRESOURCE mappedSubresource;
            ctx->Map(GContext->graphics.tranformCBuf.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &mappedSubresource);
            memcpy(mappedSubresource.pData, &transforms, sizeof(mvTransforms));
            ctx->Unmap(GContext->graphics.tranformCBuf.Get(), 0u);

            // mesh
            static const UINT offset = 0u;
            ctx->VSSetConstantBuffers(0u, 1u, GContext->graphics.tranformCBuf.GetAddressOf());
            ctx->IASetIndexBuffer(am.buffers[pointlight.mesh.primitives.back().indexBuffer].buffer.buffer, DXGI_FORMAT_R32_UINT, 0u);
            static mvVertexLayout lightvertexlayout = mvCreateVertexLayout({ mvVertexElement::Position3D });
            ctx->IASetVertexBuffers(0u, 1u,
                &am.buffers[pointlight.mesh.primitives.back().vertexBuffer].buffer.buffer,
                &lightvertexlayout.size, &offset);

            // draw
            ctx->DrawIndexed(am.buffers[pointlight.mesh.primitives.back().indexBuffer].buffer.size / sizeof(u32), 0u, 0u);
        }

        for (int i = 0; i < am.sceneCount; i++)
        {
            Renderer::mvRenderScene(am, am.scenes[i].scene, viewMatrix, projMatrix);
        }

        //-----------------------------------------------------------------------------
        // skybox pass
        //-----------------------------------------------------------------------------
        if (globalInfo.useSkybox)
        {
            ctx->OMSetRenderTargets(1, &offscreen.targetView, offscreen.depthView);
            ctx->RSSetViewports(1u, &offscreen.viewport);
            {

                // pipeline
                mvSetPipelineState(skybox.pipeline);
                ctx->PSSetSamplers(0, 1, &skybox.cubeSampler);
                ctx->PSSetShaderResources(0, 1, &skybox.cubeTexture.textureView);

                mvTransforms transforms{};
                transforms.model = mvIdentityMat4() * mvScale(mvIdentityMat4(), mvVec3{ 1.0f, 1.0f, -1.0f });
                transforms.modelView = viewMatrix * transforms.model;
                transforms.modelViewProjection = projMatrix * viewMatrix * transforms.model;

                D3D11_MAPPED_SUBRESOURCE mappedSubresource;
                ctx->Map(GContext->graphics.tranformCBuf.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &mappedSubresource);
                memcpy(mappedSubresource.pData, &transforms, sizeof(mvTransforms));
                ctx->Unmap(GContext->graphics.tranformCBuf.Get(), 0u);

                // mesh
                static const UINT offset = 0u;
                ctx->VSSetConstantBuffers(0u, 1u, GContext->graphics.tranformCBuf.GetAddressOf());
                ctx->IASetIndexBuffer(skybox.indexBuffer.buffer, DXGI_FORMAT_R32_UINT, 0u);
                ctx->IASetVertexBuffers(0u, 1u, &skybox.vertexBuffer.buffer, &skybox.vertexLayout.size, &offset);

                // draw
                ctx->DrawIndexed(skybox.indexBuffer.size / sizeof(u32), 0u, 0u);
            }
        }

        //-----------------------------------------------------------------------------
        // main pass
        //-----------------------------------------------------------------------------
        ctx->OMSetRenderTargets(1, GContext->graphics.target.GetAddressOf(), *GContext->graphics.targetDepth.GetAddressOf());
        ctx->RSSetViewports(1u, &GContext->graphics.viewport);

        ImGui::DockSpaceOverViewport(0, ImGuiDockNodeFlags_AutoHideTabBar);

        ImGui::GetForegroundDrawList()->AddText(ImVec2(45, 45),
            ImColor(0.0f, 1.0f, 0.0f), std::string(std::to_string(ImGui::GetIO().Framerate) + " FPS").c_str());

        directionalShadowMap.showControls();
        if (pointlight.showControls())
            omniShadowMap.info.view = mvCreateLookAtView(pointlight.camera);

        ImGui::Begin("Global Settings", 0);
        ImGui::ColorEdit3("Ambient Color", &globalInfo.ambientColor.x);
        ImGui::Checkbox("Use Shadows", (bool*)&globalInfo.useShadows);
        ImGui::Checkbox("Use Skybox", (bool*)&globalInfo.useSkybox);
        ImGui::End();

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::Begin("Model", 0, ImGuiWindowFlags_NoDecoration);

        if (ImGui::IsWindowHovered())
            mvUpdateCameraFPSCamera(camera, dt, 12.0f, 0.004f);

        ImVec2 contentSize = ImGui::GetWindowContentRegionMax();
        offscreen.viewport = { 0.0f, 0.0f, contentSize.x, contentSize.y, 0.0f, 1.0f };
        camera.aspect = offscreen.viewport.Width / offscreen.viewport.Height;

        ImGui::Image(offscreen.resourceView, contentSize);
        if (contentSize.x == oldContentRegion.x && contentSize.y == oldContentRegion.y)
        {

        }
        else
        {
            offscreen.targetView->Release();
            offscreen.depthView->Release();
            offscreen.resourceView->Release();
            offscreen.texture->Release();
            offscreen.depthTexture->Release();
            offscreen.resize(offscreen.viewport.Width, offscreen.viewport.Height);
        }

        oldContentRegion = contentSize;

        ImGui::End();

        ImGui::PopStyleVar();
        ImGui::PopStyleVar();

        // render imgui
        ImGui::Render();
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        // present
        GContext->graphics.swapChain->Present(1, 0);
    }

    directionLightBuffer.buffer->Release();
    globalInfoBuffer.buffer->Release();

    offscreen.targetView->Release();
    offscreen.depthView->Release();
    offscreen.resourceView->Release();
    offscreen.texture->Release();
    offscreen.depthTexture->Release();

    pointlight.buffer.buffer->Release();
    pointlight.pipeline.pixelShader->Release();
    pointlight.pipeline.vertexShader->Release();
    pointlight.pipeline.pixelBlob->Release();
    pointlight.pipeline.vertexBlob->Release();
    pointlight.pipeline.inputLayout->Release();
    pointlight.pipeline.blendState->Release();
    pointlight.pipeline.depthStencilState->Release();
    pointlight.pipeline.rasterizationState->Release();

    skybox.cubeSampler->Release();
    skybox.vertexBuffer.buffer->Release();
    skybox.indexBuffer.buffer->Release();
    skybox.cubeTexture.textureView->Release();
    skybox.pipeline.pixelShader->Release();
    skybox.pipeline.vertexShader->Release();
    skybox.pipeline.pixelBlob->Release();
    skybox.pipeline.vertexBlob->Release();
    skybox.pipeline.inputLayout->Release();
    skybox.pipeline.blendState->Release();
    skybox.pipeline.depthStencilState->Release();
    skybox.pipeline.rasterizationState->Release();

    directionalShadowMap.texture->Release();
    directionalShadowMap.depthView->Release();
    directionalShadowMap.resourceView->Release();
    directionalShadowMap.rasterizationState->Release();
    directionalShadowMap.sampler->Release();
    directionalShadowMap.buffer.buffer->Release();

    for (int i = 0; i < 6; i++)
        omniShadowMap.depthView[i]->Release();
    omniShadowMap.resourceView->Release();
    omniShadowMap.sampler->Release();
    omniShadowMap.buffer.buffer->Release();

    mvCleanupAssetManager(&am);

    // Cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
    mvCleanupGraphics();

    mvDestroyContext();
}

