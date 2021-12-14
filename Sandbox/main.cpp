#include "helpers.h"

// TODO: make most of these runtime options
static const char* gltfAssetDirectory0 = "../../data/glTF-Sample-Models/2.0/Sponza/glTF/";
static const char* gltfModel0 = "../../data/glTF-Sample-Models/2.0/Sponza/glTF/Sponza.gltf";
//static const char* gltfAssetDirectory0 = "../../data/glTF-Sample-Models/2.0/Cameras/glTF/";
//static const char* gltfModel0 = "../../data/glTF-Sample-Models/2.0/Cameras/glTF/Cameras.gltf";
static f32         shadowWidth = 95.0f;
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
    ImGui_ImplWin32_Init(window->hWnd);
    ImGui_ImplDX11_Init(GContext->graphics.device.Get(), GContext->graphics.imDeviceContext.Get());

    mvAssetManager am{};
    mvInitializeAssetManager(&am);

    // assets & meshes
    mvGLTFModel gltfmodel0 = mvLoadGLTF(gltfAssetDirectory0, gltfModel0);
    mvLoadGLTFAssets(am, gltfmodel0);
    mvCleanupGLTF(gltfmodel0);

    // main camera
    mvCamera camera{};
    camera.pos = { -13.5f, 6.0f, 3.5f };
    camera.front = { 0.0f, 0.0f, 1.0f };
    camera.pitch = 0.0f;
    camera.yaw = 0.0f;
    camera.aspect = 500.0f / 500.0f;

    // helpers
    mvShadowMap directionalShadowMap = mvShadowMap(4096, shadowWidth);
    mvShadowCubeMap omniShadowMap = mvShadowCubeMap(2048);
    mvSkybox skybox = mvSkybox();
    mvPointLight pointlight = mvPointLight(am);
    omniShadowMap.info.view = mvCreateLookAtView(pointlight.camera);

    //mvMesh frustum = mvCreateFrustum(am, 5.0f, 5.0f, 0.5f, 100.0f);
    //mvMesh frustum = mvCreateFrustum2(am, M_PI_2, 1.0f, 0.5f, 100.0f);
    mvMesh frustum1 = mvCreateFrustum2(am, 45.0f, 1.0f, 0.5f, 50.0f);

    // framework constant buffers
    DirectionLightInfo directionLightInfo{};
    mvConstBuffer directionLightBuffer = mvCreateConstBuffer(&directionLightInfo, sizeof(DirectionLightInfo));

    GlobalInfo globalInfo{};
    mvConstBuffer globalInfoBuffer = mvCreateConstBuffer(&globalInfo, sizeof(GlobalInfo));

    mvOffscreen offscreen = mvOffscreen(500.0f, 500.0f);
    mvTimer timer;
    bool recreatePrimary = false;
    bool recreateShadowMapRS = false;
    bool recreateOShadowMapRS = false;
    f32 scale0 = 5.0f;
    mvVec3 translate0 = { 0.0f, 0.0f, 0.0f };
    while (true)
    {
        const auto dt = timer.mark() * 1.0f;

        mvVec3 scaleVec0 = { scale0, scale0, scale0 };
        mvMat4 stransform0 = mvScale(mvIdentityMat4(), scaleVec0);
        mvMat4 ttransform0 = mvTranslate(mvIdentityMat4(), translate0);

        if (const auto ecode = mvProcessViewportEvents()) break;

        if (window->resized)
        {
            mvRecreateSwapChain(window->width, window->height);
            window->resized = false;
        }

        if (recreatePrimary)
        {
            offscreen.targetView->Release();
            offscreen.depthView->Release();
            offscreen.resourceView->Release();
            offscreen.texture->Release();
            offscreen.depthTexture->Release();
            offscreen.resize(offscreen.viewport.Width, offscreen.viewport.Height);
            recreatePrimary = false;
        }

        if (recreateShadowMapRS)
        {
            directionalShadowMap.rasterizationState->Release();

            D3D11_RASTERIZER_DESC shadowRenderStateDesc;
            ZeroMemory(&shadowRenderStateDesc, sizeof(D3D11_RASTERIZER_DESC));
            shadowRenderStateDesc.CullMode = directionalShadowMap.backface ? D3D11_CULL_BACK : D3D11_CULL_FRONT;
            shadowRenderStateDesc.FrontCounterClockwise = true;
            shadowRenderStateDesc.FillMode = D3D11_FILL_SOLID;
            shadowRenderStateDesc.DepthClipEnable = true;
            shadowRenderStateDesc.DepthBias = directionalShadowMap.depthBias;
            shadowRenderStateDesc.DepthBiasClamp = 0.0f;
            shadowRenderStateDesc.SlopeScaledDepthBias = directionalShadowMap.slopeBias;

            GContext->graphics.device->CreateRasterizerState(&shadowRenderStateDesc, &directionalShadowMap.rasterizationState);
            recreateShadowMapRS = false;
        }

        if (recreateOShadowMapRS)
        {
            omniShadowMap.rasterizationState->Release();

            D3D11_RASTERIZER_DESC shadowRenderStateDesc;
            ZeroMemory(&shadowRenderStateDesc, sizeof(D3D11_RASTERIZER_DESC));
            shadowRenderStateDesc.CullMode = D3D11_CULL_BACK;
            shadowRenderStateDesc.FrontCounterClockwise = true;
            shadowRenderStateDesc.FillMode = D3D11_FILL_SOLID;
            shadowRenderStateDesc.DepthClipEnable = true;
            shadowRenderStateDesc.DepthBias = omniShadowMap.depthBias;
            shadowRenderStateDesc.DepthBiasClamp = 0.0f;
            shadowRenderStateDesc.SlopeScaledDepthBias = omniShadowMap.slopeBias;

            GContext->graphics.device->CreateRasterizerState(&shadowRenderStateDesc, &omniShadowMap.rasterizationState);
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
            Renderer::mvRenderSceneShadows(am, am.scenes[i].scene, directionalShadowMap.getViewMatrix(), directionalShadowMap.getProjectionMatrix(), stransform0, ttransform0);

        //-----------------------------------------------------------------------------
        // omni shadow pass
        //-----------------------------------------------------------------------------
        ctx->RSSetViewports(1u, &omniShadowMap.viewport);
        ctx->RSSetState(omniShadowMap.rasterizationState);

        for (u32 i = 0; i < 6; i++)
        {
            ctx->OMSetRenderTargets(0, nullptr, omniShadowMap.depthView[i]);
            mvVec3 look_target = pointlight.camera.pos + omniShadowMap.cameraDirections[i];
            mvMat4 camera_matrix = mvLookAtRH(pointlight.camera.pos, look_target, omniShadowMap.cameraUps[i]);

            for (int i = 0; i < am.sceneCount; i++)
                Renderer::mvRenderSceneShadows(am, am.scenes[i].scene, camera_matrix, mvPerspectiveRH(M_PI_2, 1.0f, 0.5f, 100.0f), stransform0, ttransform0);
        }

        //-----------------------------------------------------------------------------
        // offscreen pass
        //-----------------------------------------------------------------------------
        ctx->OMSetRenderTargets(1, &offscreen.targetView, offscreen.depthView);
        ctx->RSSetViewports(1u, &offscreen.viewport);

        mvMat4 viewMatrix = mvCreateFPSView(camera);
        mvMat4 projMatrix = mvCreateLookAtProjection(camera);

        globalInfo.camPos = camera.pos;
        directionLightInfo.viewLightDir = directionalShadowMap.camera.dir;

        // update constant buffers
        mvUpdateConstBuffer(pointlight.buffer, &pointlight.info);
        mvUpdateConstBuffer(directionLightBuffer, &directionLightInfo);
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
        ctx->PSSetSamplers(3u, 1, &skybox.cubeSampler);

        // textures
        ctx->PSSetShaderResources(5u, 1, &directionalShadowMap.resourceView);
        ctx->PSSetShaderResources(6u, 1, &omniShadowMap.resourceView);
        ctx->PSSetShaderResources(7u, 1, &skybox.cubeTexture.textureView);

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
            Renderer::mvRenderScene(am, am.scenes[i].scene, viewMatrix, projMatrix, stransform0, ttransform0);

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

        // render frustum mesh
        {
            mvSetPipelineState(pointlight.pipeline2);

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
            ctx->IASetIndexBuffer(am.buffers[frustum1.primitives.back().indexBuffer].buffer.buffer, DXGI_FORMAT_R32_UINT, 0u);
            static mvVertexLayout lightvertexlayout = mvCreateVertexLayout({ mvVertexElement::Position3D });
            ctx->IASetVertexBuffers(0u, 1u,
                &am.buffers[frustum1.primitives.back().vertexBuffer].buffer.buffer,
                &lightvertexlayout.size, &offset);

            // draw
            ctx->DrawIndexed(am.buffers[frustum1.primitives.back().indexBuffer].buffer.size / sizeof(u32), 0u, 0u);
        }

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
                mvUpdateCameraFPSCamera(camera, dt, 12.0f, 0.004f);

            ImVec2 contentSize = ImGui::GetContentRegionAvail();
            offscreen.viewport = { 0.0f, 0.0f, contentSize.x, contentSize.y, 0.0f, 1.0f };
            camera.aspect = offscreen.viewport.Width / offscreen.viewport.Height;

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
                directionalShadowMap.camera.left = -directionalShadowMap.width;
                directionalShadowMap.camera.right = directionalShadowMap.width;
                directionalShadowMap.camera.bottom = -directionalShadowMap.width;
                directionalShadowMap.camera.top = directionalShadowMap.width;

                f32 zcomponent = sinf(M_PI * directionalShadowMap.angle / 180.0f);
                f32 ycomponent = cosf(M_PI * directionalShadowMap.angle / 180.0f);

                directionalShadowMap.camera.dir = { 0.0f, -ycomponent, zcomponent };
                directionalShadowMap.info.view = mvLookAtRH(
                    directionalShadowMap.camera.pos, directionalShadowMap.camera.pos - directionalShadowMap.camera.dir, directionalShadowMap.camera.up);
                directionalShadowMap.info.projection = mvOrthoRH(
                    directionalShadowMap.camera.left, directionalShadowMap.camera.right, directionalShadowMap.camera.bottom,
                    directionalShadowMap.camera.top, directionalShadowMap.camera.nearZ, directionalShadowMap.camera.farZ);
            }

            ImGui::Dummy(ImVec2(50.0f, 25.0f));
            ImGui::Text("%s", "Point Light:");
            if (ImGui::SliderFloat3("Position##o", &pointlight.info.viewLightPos.x, -25.0f, 50.0f))
            {
                pointlight.camera.pos = { pointlight.info.viewLightPos.x, pointlight.info.viewLightPos.y, pointlight.info.viewLightPos.z };
                omniShadowMap.info.view = mvCreateLookAtView(pointlight.camera);
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

    pointlight.pipeline2.pixelShader->Release();
    pointlight.pipeline2.vertexShader->Release();
    pointlight.pipeline2.pixelBlob->Release();
    pointlight.pipeline2.vertexBlob->Release();
    pointlight.pipeline2.inputLayout->Release();
    pointlight.pipeline2.blendState->Release();
    pointlight.pipeline2.depthStencilState->Release();
    pointlight.pipeline2.rasterizationState->Release();

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

    omniShadowMap.rasterizationState->Release();

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

