#include "mv3D.h"
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>

mv_internal const char* sponzaPath = "C:/dev/MarvelAssets/Sponza/";
mv_internal b8 loadSponza = true;
mv_internal f32 shadowWidth = 100.0f;

struct Skybox
{
    ID3D11SamplerState* cubeSampler = nullptr;
    mvBuffer vertexBuffer;
    mvBuffer indexBuffer;
    mvVertexLayout vertexLayout;
    mvCubeTexture cubeTexture;
};

struct Mesh
{
    mvBuffer vertexBuffer;
    mvBuffer indexBuffer;
};

Skybox create_skybox_resources()
{
    Skybox skybox{};

    const float side = 1.0f / 2.0f;
    auto vertices = std::vector<f32>{
        -side, -side, -side,
         side, -side, -side,
        -side,  side, -side,
         side,  side, -side,
        -side, -side,  side,
         side, -side,  side,
        -side,  side,  side,
         side,  side,  side
    };

    static auto indices = std::vector<u32>{
        0, 2, 1, 2, 3, 1,
        1, 3, 5, 3, 7, 5,
        2, 6, 3, 3, 6, 7,
        4, 5, 7, 4, 7, 6,
        0, 4, 2, 2, 4, 6,
        0, 1, 4, 1, 5, 4
    };

    skybox.vertexBuffer = mvCreateBuffer(vertices.data(), vertices.size() * sizeof(f32), D3D11_BIND_VERTEX_BUFFER);
    skybox.indexBuffer = mvCreateBuffer(indices.data(), indices.size() * sizeof(u32), D3D11_BIND_INDEX_BUFFER);
    skybox.vertexLayout = mvCreateVertexLayout({ mvVertexElement::Position3D });
    skybox.cubeTexture = mvCreateCubeTexture("../../Resources/Skybox");

    D3D11_SAMPLER_DESC samplerDesc{};
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
    samplerDesc.BorderColor[0] = 0.0f;
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.MaxAnisotropy = D3D11_REQ_MAXANISOTROPY;
    GContext->graphics.device->CreateSamplerState(&samplerDesc, &skybox.cubeSampler);

    return skybox;
}

mvPipeline create_skybox_pipeline()
{
    mvPipeline pipeline{};

    D3D11_DEPTH_STENCIL_DESC dsDesc = CD3D11_DEPTH_STENCIL_DESC{ CD3D11_DEFAULT{} };
    dsDesc.DepthEnable = true;
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

    D3D11_RASTERIZER_DESC skyboxRasterDesc = CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT{});
    skyboxRasterDesc.CullMode = D3D11_CULL_NONE;
    skyboxRasterDesc.FrontCounterClockwise = TRUE;

    D3D11_BLEND_DESC blendDesc = CD3D11_BLEND_DESC{ CD3D11_DEFAULT{} };
    auto& brt = blendDesc.RenderTarget[0];
    brt.BlendEnable = TRUE;
    brt.SrcBlend = D3D11_BLEND_SRC_ALPHA;
    brt.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;

    GContext->graphics.device->CreateDepthStencilState(&dsDesc, pipeline.depthStencilState.GetAddressOf());
    GContext->graphics.device->CreateRasterizerState(&skyboxRasterDesc, pipeline.rasterizationState.GetAddressOf());
    GContext->graphics.device->CreateBlendState(&blendDesc, pipeline.blendState.GetAddressOf());

    mvPixelShader pixelShader = mvCreatePixelShader(GContext->IO.shaderDirectory + "Skybox_PS.hlsl");
    mvVertexShader vertexShader = mvCreateVertexShader(GContext->IO.shaderDirectory + "Skybox_VS.hlsl", mvCreateVertexLayout({ mvVertexElement::Position3D }));

    pipeline.pixelShader = pixelShader.shader;
    pipeline.pixelBlob = pixelShader.blob;
    pipeline.vertexShader = vertexShader.shader;
    pipeline.vertexBlob = vertexShader.blob;
    pipeline.inputLayout = vertexShader.inputLayout;

    return pipeline;
}

int main()
{

    mvCreateContext();
    GContext->IO.shaderDirectory = "../../mv3D/shaders/";
    GContext->IO.resourceDirectory = "../../Resources/";

    int initialWidth = 1850;
    int initialHeight = 900;
    mvViewport* window = mvInitializeViewport(initialWidth, initialHeight);
    mvSetupGraphics(*window);

    // Setup Dear ImGui context
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui_ImplWin32_Init(window->hWnd);
    ImGui_ImplDX11_Init(GContext->graphics.device.Get(), GContext->graphics.imDeviceContext.Get());

    mvAssetManager am{};
    mvInitializeAssetManager(&am);

    

    // assets & meshes
    if (loadSponza) mvLoadOBJAssets(am, sponzaPath, "sponza");

    // scenes
    mvScene scene = mvCreateScene();

    // main camera
    mvCamera camera{};
    camera.pos = { -13.5f, 6.0f, 3.5f };
    camera.front = { 0.0f, 0.0f, -1.0f };
    camera.pitch = 0.0f;
    camera.yaw = 0.0f;
    camera.aspect = initialWidth / initialHeight;

    // lights
    mvPointLight light = mvCreatePointLight(&am, { 0.0f, 15.0f, 0.0f });
    mvDirectionLight dlight = mvCreateDirectionLight({ 0.0f, -1.0f, 0.0f });

    // shadows

    mvShadowCamera dshadowCamera = mvCreateShadowCamera();

    mvShadowMap directionalShadowMap = mvCreateShadowMap(4000, shadowWidth);
    mvShadowCubeMap omniShadowMap = mvCreateShadowCubeMap(2000);

    D3D11_VIEWPORT viewport = { 0.0f, 0.0f, (f32)initialWidth, (f32)initialHeight, 0.0f, 1.0f };
    D3D11_VIEWPORT shadowviewport = { 0.0f, 0.0f, (f32)directionalShadowMap.shadowMapDimension, (f32)directionalShadowMap.shadowMapDimension, 0.0f, 1.0f };
    D3D11_VIEWPORT oshadowviewport = { 0.0f, 0.0f, (f32)omniShadowMap.shadowMapDimension, (f32)omniShadowMap.shadowMapDimension, 0.0f, 1.0f };

    mvPipeline skyboxPipeline = create_skybox_pipeline();
    Skybox skybox = create_skybox_resources();

    dshadowCamera.info.directShadowView = mvLookAtRH(directionalShadowMap.camera.pos, directionalShadowMap.camera.pos + directionalShadowMap.camera.dir, directionalShadowMap.camera.up);
    dshadowCamera.info.directShadowProjection = mvOrthoRH(directionalShadowMap.camera.left, directionalShadowMap.camera.right, directionalShadowMap.camera.bottom, directionalShadowMap.camera.top, directionalShadowMap.camera.nearZ, directionalShadowMap.camera.farZ);

    mvTimer timer;
    while (true)
    {
        const auto dt = timer.mark() * 1.0f;

        if (const auto ecode = mvProcessViewportEvents()) break;

        if (window->resized)
        {
            mvRecreateSwapChain(window->width, window->height);
            window->resized = false;
            camera.aspect = (float)window->width / (float)window->height;
            viewport.Width = window->width;
            viewport.Height = window->height;
        }

        //-----------------------------------------------------------------------------
        // clear targets
        //-----------------------------------------------------------------------------
        static float backgroundColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
        GContext->graphics.imDeviceContext->ClearRenderTargetView(*GContext->graphics.target.GetAddressOf(), backgroundColor);
        GContext->graphics.imDeviceContext->ClearDepthStencilView(*GContext->graphics.targetDepth.GetAddressOf(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0u);
        GContext->graphics.imDeviceContext->ClearDepthStencilView(*directionalShadowMap.shadowDepthView.GetAddressOf(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0u);
        for (u32 i = 0; i < 6; i++)
            GContext->graphics.imDeviceContext->ClearDepthStencilView(*omniShadowMap.shadowDepthViews[i].GetAddressOf(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0u);

        //-----------------------------------------------------------------------------
        // begin frame
        //-----------------------------------------------------------------------------
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        ID3D11ShaderResourceView* const pSRV[6] = { NULL };
        GContext->graphics.imDeviceContext->PSSetShaderResources(0, 1, pSRV);
        GContext->graphics.imDeviceContext->PSSetShaderResources(1, 1, pSRV);
        GContext->graphics.imDeviceContext->PSSetShaderResources(2, 1, pSRV);
        GContext->graphics.imDeviceContext->PSSetShaderResources(3, 6, pSRV); // depth map
        GContext->graphics.imDeviceContext->PSSetShaderResources(4, 6, pSRV); // depth map
        GContext->graphics.imDeviceContext->PSSetShaderResources(5, 6, pSRV); // depth map
        GContext->graphics.imDeviceContext->PSSetShaderResources(6, 1, pSRV); // depth map

        ImGui::GetForegroundDrawList()->AddText(ImVec2(45, 45),
            ImColor(0.0f, 1.0f, 0.0f), std::string(std::to_string(ImGui::GetIO().Framerate) + " FPS").c_str());

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
        GContext->graphics.imDeviceContext->OMSetRenderTargets(0, nullptr, *directionalShadowMap.shadowDepthView.GetAddressOf());
        GContext->graphics.imDeviceContext->RSSetViewports(1u, &shadowviewport);
        GContext->graphics.imDeviceContext->RSSetState(*directionalShadowMap.shadowRasterizationState.GetAddressOf());

        for (int i = 0; i < am.sceneCount; i++)
            Renderer::mvRenderSceneShadows(am, am.scenes[i].scene, mvCreateOrthoView(directionalShadowMap.camera), mvCreateOrthoProjection(directionalShadowMap.camera));

        //-----------------------------------------------------------------------------
        // omni shadow pass
        //-----------------------------------------------------------------------------
        for (u32 i = 0; i < 6; i++)
        {
            GContext->graphics.imDeviceContext->OMSetRenderTargets(0, nullptr, *omniShadowMap.shadowDepthViews[i].GetAddressOf());
            GContext->graphics.imDeviceContext->RSSetViewports(1u, &oshadowviewport);
            GContext->graphics.imDeviceContext->RSSetState(*omniShadowMap.shadowRasterizationState.GetAddressOf());

            mvVec3 look_target = light.camera.pos + omniShadowMap.cameraDirections[i];
            mvMat4 camera_matrix = mvLookAtLH(light.camera.pos, look_target, omniShadowMap.cameraUps[i]);

            for (int i = 0; i < am.sceneCount; i++)
                Renderer::mvRenderSceneShadows(am, am.scenes[i].scene, camera_matrix, mvPerspectiveLH(M_PI_2, 1.0f, 0.5f, 100.0f));
        }

        //-----------------------------------------------------------------------------
        // main pass
        //-----------------------------------------------------------------------------
        GContext->graphics.imDeviceContext->OMSetRenderTargets(1, GContext->graphics.target.GetAddressOf(), *GContext->graphics.targetDepth.GetAddressOf());
        GContext->graphics.imDeviceContext->RSSetViewports(1u, &viewport);

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
        {
            Renderer::mvRenderScene(am, am.scenes[i].scene, viewMatrix, projMatrix);
        }

        //-----------------------------------------------------------------------------
        // skybox pass
        //-----------------------------------------------------------------------------
        GContext->graphics.imDeviceContext->OMSetRenderTargets(1, GContext->graphics.target.GetAddressOf(), *GContext->graphics.targetDepth.GetAddressOf());
        GContext->graphics.imDeviceContext->RSSetViewports(1u, &viewport);
        mvBindSlot_bPS(0u, scene);
        {

            auto device = GContext->graphics.imDeviceContext;

            // pipeline
            device->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            device->RSSetState(skyboxPipeline.rasterizationState.Get());
            device->OMSetBlendState(skyboxPipeline.blendState.Get(), nullptr, 0xFFFFFFFFu);
            device->OMSetDepthStencilState(skyboxPipeline.depthStencilState.Get(), 0xFF);;
            device->IASetInputLayout(skyboxPipeline.inputLayout.Get());
            device->VSSetShader(skyboxPipeline.vertexShader.Get(), nullptr, 0);
            device->PSSetShader(skyboxPipeline.pixelShader.Get(), nullptr, 0);
            device->HSSetShader(nullptr, nullptr, 0);
            device->DSSetShader(nullptr, nullptr, 0);
            device->GSSetShader(nullptr, nullptr, 0);
            device->PSSetSamplers(0, 1, &skybox.cubeSampler);
            device->PSSetShaderResources(0, 1, skybox.cubeTexture.textureView.GetAddressOf());

            mvTransforms transforms{};
            transforms.model = mvIdentityMat4() * mvScale(mvIdentityMat4(), mvVec3{ 1.0f, 1.0f, -1.0f });
            transforms.modelView = viewMatrix * transforms.model;
            transforms.modelViewProjection = projMatrix * viewMatrix * transforms.model;

            D3D11_MAPPED_SUBRESOURCE mappedSubresource;
            device->Map(GContext->graphics.tranformCBuf.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &mappedSubresource);
            memcpy(mappedSubresource.pData, &transforms, sizeof(mvTransforms));
            device->Unmap(GContext->graphics.tranformCBuf.Get(), 0u);

            // mesh
            static const UINT offset = 0u;
            device->VSSetConstantBuffers(0u, 1u, GContext->graphics.tranformCBuf.GetAddressOf());
            device->IASetIndexBuffer(skybox.indexBuffer.buffer, DXGI_FORMAT_R32_UINT, 0u);
            device->IASetVertexBuffers(0u, 1u, &skybox.vertexBuffer.buffer, &skybox.vertexLayout.size, &offset);

            // draw
            device->DrawIndexed(skybox.indexBuffer.size / sizeof(u32), 0u, 0u);
        }

        //-----------------------------------------------------------------------------
        // end frame & present
        //-----------------------------------------------------------------------------

        // render imgui
        ImGui::Render();
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        // present
        GContext->graphics.swapChain->Present(1, 0);
    }

    mvCleanupAssetManager(&am);

    // Cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
    mvCleanupGraphics();

    mvDestroyContext();
}

