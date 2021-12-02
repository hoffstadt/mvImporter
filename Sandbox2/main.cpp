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

struct ShadowMap
{
    ID3D11Texture2D*          texture;
    ID3D11DepthStencilView*   shadowDepthView;
    ID3D11ShaderResourceView* shadowResourceView;
};

struct ShadowCubeMap
{
    ID3D11Texture2D*          texture;
    ID3D11DepthStencilView*   shadowDepthView[6];
    ID3D11ShaderResourceView* shadowResourceView;
    mvVec3                    cameraDirections[6];
    mvVec3                    cameraUps[6];
};

ShadowMap create_directional_shadowmap(u32 resolution)
{
    ShadowMap shadowmap{};

    D3D11_TEXTURE2D_DESC shadowMapDesc;
    ZeroMemory(&shadowMapDesc, sizeof(D3D11_TEXTURE2D_DESC));
    shadowMapDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
    shadowMapDesc.MipLevels = 1;
    shadowMapDesc.ArraySize = 1;
    shadowMapDesc.SampleDesc.Count = 1;
    shadowMapDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
    shadowMapDesc.Height = static_cast<UINT>(resolution);
    shadowMapDesc.Width = static_cast<UINT>(resolution);

    HRESULT hr = GContext->graphics.device->CreateTexture2D(
        &shadowMapDesc,
        nullptr,
        &shadowmap.texture
    );

    D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
    ZeroMemory(&depthStencilViewDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
    depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    depthStencilViewDesc.Texture2D.MipSlice = 0;

    D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
    ZeroMemory(&shaderResourceViewDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
    shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    shaderResourceViewDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
    shaderResourceViewDesc.Texture2D.MipLevels = 1;

    GContext->graphics.device->CreateDepthStencilView(
        shadowmap.texture,
        &depthStencilViewDesc,
        &shadowmap.shadowDepthView
    );

    GContext->graphics.device->CreateShaderResourceView(
        shadowmap.texture,
        &shaderResourceViewDesc,
        &shadowmap.shadowResourceView
    );

    return shadowmap;
}

ShadowCubeMap create_omni_shadowmap(u32 resolution)
{
    ShadowCubeMap shadowCubeMap{};

    // texture descriptor
    D3D11_TEXTURE2D_DESC textureDesc = {};
    textureDesc.Width = resolution;
    textureDesc.Height = resolution;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 6;
    textureDesc.Format = DXGI_FORMAT_R32_TYPELESS;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
    textureDesc.CPUAccessFlags = 0;
    textureDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

    // create the texture resource
    mvComPtr<ID3D11Texture2D> pTexture;
    GContext->graphics.device->CreateTexture2D(&textureDesc, nullptr, &pTexture);

    // create the resource view on the texture
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = 1;
    GContext->graphics.device->CreateShaderResourceView(pTexture.Get(), &srvDesc, &shadowCubeMap.shadowResourceView);

    // make render target resources for capturing shadow map
    for (u32 face = 0; face < 6; face++)
    {
        // create target view of depth stensil texture
        D3D11_DEPTH_STENCIL_VIEW_DESC descView = {};
        descView.Format = DXGI_FORMAT_D32_FLOAT;
        descView.Flags = 0;
        descView.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
        descView.Texture2DArray.MipSlice = 0;
        descView.Texture2DArray.ArraySize = 1;
        descView.Texture2DArray.FirstArraySlice = face;
        GContext->graphics.device->CreateDepthStencilView(pTexture.Get(), &descView, &(shadowCubeMap.shadowDepthView[face]));
    }

    shadowCubeMap.cameraDirections[0] = { 0.0f,  0.0f,  -1.0f };
    shadowCubeMap.cameraDirections[1] = { 0.0f,  0.0f,  1.0f };
    shadowCubeMap.cameraDirections[2] = { 0.0f,  1.0f,  0.0f };
    shadowCubeMap.cameraDirections[3] = { 0.0f,  -1.0f,  0.0f };
    shadowCubeMap.cameraDirections[4] = { 1.0f,  0.0f,  0.0f };
    shadowCubeMap.cameraDirections[5] = { -1.0f,  0.0f,   0.0f };

    shadowCubeMap.cameraUps[0] = { 0.0f,  1.0f,  0.0f };
    shadowCubeMap.cameraUps[1] = { 0.0f,  1.0f,  0.0f };
    shadowCubeMap.cameraUps[2] = { 1.0f, 0.0f,   0.0f };
    shadowCubeMap.cameraUps[3] = { 1.0f, 0.0f,   0.0f };
    shadowCubeMap.cameraUps[4] = { 0.0f,  1.0f,  0.0f };
    shadowCubeMap.cameraUps[5] = { 0.0f,  1.0f,  0.0f };

    return shadowCubeMap;
}

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

    GContext->graphics.device->CreateDepthStencilState(&dsDesc, &pipeline.depthStencilState);
    GContext->graphics.device->CreateRasterizerState(&skyboxRasterDesc, &pipeline.rasterizationState);
    GContext->graphics.device->CreateBlendState(&blendDesc, &pipeline.blendState);

    mvPixelShader pixelShader = mvCreatePixelShader(GContext->IO.shaderDirectory + "Skybox_PS.hlsl");
    mvVertexShader vertexShader = mvCreateVertexShader(GContext->IO.shaderDirectory + "Skybox_VS.hlsl", mvCreateVertexLayout({ mvVertexElement::Position3D }));

    pipeline.pixelShader = pixelShader.shader;
    pipeline.pixelBlob = pixelShader.blob;
    pipeline.vertexShader = vertexShader.shader;
    pipeline.vertexBlob = vertexShader.blob;
    pipeline.inputLayout = vertexShader.inputLayout;
    pipeline.topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    return pipeline;
}

mvPipeline create_light_pipeline()
{
    mvPipeline pipeline{};

    pipeline.topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    D3D11_RASTERIZER_DESC rasterDesc = CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT{});
    rasterDesc.CullMode = D3D11_CULL_BACK;
    rasterDesc.FrontCounterClockwise = TRUE;

    GContext->graphics.device->CreateRasterizerState(&rasterDesc, &pipeline.rasterizationState);

    D3D11_DEPTH_STENCIL_DESC dsDesc = CD3D11_DEPTH_STENCIL_DESC{ CD3D11_DEFAULT{} };

    // Depth test parameters
    dsDesc.DepthEnable = true;
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    dsDesc.DepthFunc = D3D11_COMPARISON_LESS;

    // Stencil test parameters
    dsDesc.StencilEnable = true;
    dsDesc.StencilReadMask = 0xFF;
    dsDesc.StencilWriteMask = 0xFF;

    // Stencil operations if pixel is front-facing
    dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
    dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    // Stencil operations if pixel is back-facing
    dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
    dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    GContext->graphics.device->CreateDepthStencilState(&dsDesc, &pipeline.depthStencilState);

    D3D11_BLEND_DESC blendDesc = CD3D11_BLEND_DESC{ CD3D11_DEFAULT{} };
    auto& brt = blendDesc.RenderTarget[0];
    brt.BlendEnable = TRUE;
    brt.SrcBlend = D3D11_BLEND_SRC_ALPHA;
    brt.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    GContext->graphics.device->CreateBlendState(&blendDesc, &pipeline.blendState);

    mvPixelShader pixelShader = mvCreatePixelShader(GContext->IO.shaderDirectory + "Solid_PS.hlsl");
    mvVertexShader vertexShader = mvCreateVertexShader(GContext->IO.shaderDirectory + "Solid_VS.hlsl", mvCreateVertexLayout({ mvVertexElement::Position3D }));

    pipeline.pixelShader = pixelShader.shader;
    pipeline.pixelBlob = pixelShader.blob;
    pipeline.vertexShader = vertexShader.shader;
    pipeline.vertexBlob = vertexShader.blob;
    pipeline.inputLayout = vertexShader.inputLayout;
    pipeline.topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

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
    ID3D11DeviceContext* ctx = GContext->graphics.imDeviceContext.Get();

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

    // shadows
    ShadowMap shadowmap = create_directional_shadowmap(4096);
    ShadowCubeMap shadowcubemap = create_omni_shadowmap(2048);
    mvOrthoCamera directionalShadowCamera{};
    directionalShadowCamera.dir = { 0.0f, 1.0f, 0.0f };
    directionalShadowCamera.up = { 1.0f, 0.0f, 0.0f };
    directionalShadowCamera.pos.y = shadowWidth / 2.0f;
    directionalShadowCamera.left = -shadowWidth;
    directionalShadowCamera.right = shadowWidth;
    directionalShadowCamera.bottom = -shadowWidth;
    directionalShadowCamera.top = shadowWidth;
    directionalShadowCamera.nearZ = -shadowWidth - 1.0f;
    directionalShadowCamera.farZ = shadowWidth + 1.0f;

    ID3D11RasterizerState* shadowRasterizationState = nullptr;
    {
        D3D11_RASTERIZER_DESC shadowRenderStateDesc;
        ZeroMemory(&shadowRenderStateDesc, sizeof(D3D11_RASTERIZER_DESC));
        shadowRenderStateDesc.CullMode = D3D11_CULL_FRONT;
        shadowRenderStateDesc.FrontCounterClockwise = TRUE;
        shadowRenderStateDesc.FillMode = D3D11_FILL_SOLID;
        shadowRenderStateDesc.DepthClipEnable = true;
        shadowRenderStateDesc.DepthBias = 50;
        shadowRenderStateDesc.DepthBiasClamp = 0.1f;
        shadowRenderStateDesc.SlopeScaledDepthBias = 2.0f;

        GContext->graphics.device->CreateRasterizerState(&shadowRenderStateDesc, &shadowRasterizationState);
    }

    ID3D11SamplerState* shadowSampler = nullptr;
    {
        D3D11_SAMPLER_DESC comparisonSamplerDesc;
        ZeroMemory(&comparisonSamplerDesc, sizeof(D3D11_SAMPLER_DESC));
        comparisonSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
        comparisonSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
        comparisonSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        comparisonSamplerDesc.BorderColor[0] = 1.0f;
        comparisonSamplerDesc.BorderColor[1] = 1.0f;
        comparisonSamplerDesc.BorderColor[2] = 1.0f;
        comparisonSamplerDesc.BorderColor[3] = 1.0f;
        comparisonSamplerDesc.MinLOD = 0.f;
        comparisonSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
        comparisonSamplerDesc.MipLODBias = 0.f;
        comparisonSamplerDesc.MaxAnisotropy = 0;
        comparisonSamplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
        comparisonSamplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;

        GContext->graphics.device->CreateSamplerState(
            &comparisonSamplerDesc,
            &shadowSampler
        );
    }

    ID3D11SamplerState* omniShadowSampler = nullptr;
    {
        D3D11_SAMPLER_DESC comparisonSamplerDesc;
        ZeroMemory(&comparisonSamplerDesc, sizeof(D3D11_SAMPLER_DESC));
        comparisonSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
        comparisonSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
        comparisonSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        comparisonSamplerDesc.BorderColor[0] = 1.0f;
        comparisonSamplerDesc.BorderColor[1] = 1.0f;
        comparisonSamplerDesc.BorderColor[2] = 1.0f;
        comparisonSamplerDesc.BorderColor[3] = 1.0f;
        comparisonSamplerDesc.MinLOD = 0.f;
        comparisonSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
        comparisonSamplerDesc.MipLODBias = 0.f;
        comparisonSamplerDesc.MaxAnisotropy = 0;
        comparisonSamplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
        comparisonSamplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
        GContext->graphics.device->CreateSamplerState(
            &comparisonSamplerDesc,
            &omniShadowSampler
        );
    }

    struct ShadowCameraInfo
    {
        mvMat4 pointShadowView = mvIdentityMat4();
        mvMat4 directShadowView = mvIdentityMat4();
        mvMat4 directShadowProjection = mvIdentityMat4();
    } shadowCameraInfo;

    struct PointLightInfo
    {

        mvVec4 viewLightPos = { 0.0f, 15.0f, 0.0f, 1.0f };
        //-------------------------- ( 16 bytes )

        mvVec3 diffuseColor = { 1.0f, 1.0f, 1.0f };
        f32    diffuseIntensity = 1.0f;
        //-------------------------- ( 16 bytes )

        f32  attConst = 1.0f;
        f32  attLin = 0.045f;
        f32  attQuad = 0.0075f;
        char _pad1[4];
        //-------------------------- ( 16 bytes )

        //-------------------------- ( 4*16 = 64 bytes )
    } pointLightInfo;

    struct DirectionLightInfo
    {

        f32    diffuseIntensity = 1.0f;
        mvVec3 viewLightDir = { 0.0f, -1.0f, 0.0f };
        //-------------------------- ( 16 bytes )

        mvVec3 diffuseColor = { 1.0f, 1.0f, 1.0f };
        f32    padding = 0.0f;
        //-------------------------- ( 16 bytes )

        //-------------------------- ( 2*16 = 32 bytes )
    } directionLightInfo;

    mvConstBuffer shadowCameraBuffer = mvCreateConstBuffer(&shadowCameraInfo, sizeof(ShadowCameraInfo));
    mvConstBuffer directionLightBuffer = mvCreateConstBuffer(&directionLightInfo, sizeof(DirectionLightInfo));
    mvConstBuffer pointLightBuffer = mvCreateConstBuffer(&pointLightInfo, sizeof(PointLightInfo));

    // viewports
    D3D11_VIEWPORT viewport = { 0.0f, 0.0f, (f32)initialWidth, (f32)initialHeight, 0.0f, 1.0f };
    D3D11_VIEWPORT shadowviewport = { 0.0f, 0.0f, (f32)4096, (f32)4096, 0.0f, 1.0f };
    D3D11_VIEWPORT oshadowviewport = { 0.0f, 0.0f, (f32)2048, (f32)2048, 0.0f, 1.0f };

    mvCamera lightCamera{};
    lightCamera.aspect = 1.0f;
    lightCamera.yaw = 0.0f;
    lightCamera.pitch = 0.0f;
    lightCamera.pos = pointLightInfo.viewLightPos.xyz();

    mvMesh lightMesh = mvCreateCube(am, 0.25f);
    lightMesh.phongMaterialID = mvGetPhongMaterialAsset(&am, "Phong_VS.hlsl", "Solid_PS.hlsl", true, false, false, false);
    am.phongMaterials[lightMesh.phongMaterialID].material.data.materialColor = { 1.0f, 1.0f, 1.0f, 1.0f };

    mvPipeline lightPipeline = create_light_pipeline();

    mvPipeline skyboxPipeline = create_skybox_pipeline();
    Skybox skybox = create_skybox_resources();

    shadowCameraInfo.directShadowView = mvLookAtRH(directionalShadowCamera.pos, directionalShadowCamera.pos + directionalShadowCamera.dir, directionalShadowCamera.up);
    shadowCameraInfo.directShadowProjection = mvOrthoRH(directionalShadowCamera.left, directionalShadowCamera.right, directionalShadowCamera.bottom, directionalShadowCamera.top, directionalShadowCamera.nearZ, directionalShadowCamera.farZ);

    
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
        ctx->ClearRenderTargetView(*GContext->graphics.target.GetAddressOf(), backgroundColor);
        ctx->ClearDepthStencilView(*GContext->graphics.targetDepth.GetAddressOf(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0u);
        ctx->ClearDepthStencilView(shadowmap.shadowDepthView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0u);
        for (u32 i = 0; i < 6; i++)
            ctx->ClearDepthStencilView(shadowcubemap.shadowDepthView[i], D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0u);

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

        ImGui::GetForegroundDrawList()->AddText(ImVec2(45, 45),
            ImColor(0.0f, 1.0f, 0.0f), std::string(std::to_string(ImGui::GetIO().Framerate) + " FPS").c_str());

        // controls
        ImGui::Begin("Direction Light");
        if (ImGui::SliderFloat3("DLight", &directionalShadowCamera.dir.x, -1.0f, 1.0f))
        {
            shadowCameraInfo.directShadowView = mvLookAtRH(directionalShadowCamera.pos, directionalShadowCamera.pos + directionalShadowCamera.dir, directionalShadowCamera.up);
        }
        ImGui::End();

        ImGui::Begin("Point Light");
        if (ImGui::SliderFloat3("Position", &pointLightInfo.viewLightPos.x, -25.0f, 50.0f))
        {
            lightCamera.pos = { pointLightInfo.viewLightPos.x, pointLightInfo.viewLightPos.y, pointLightInfo.viewLightPos.z };
        }
        ImGui::End();

        ImGui::Begin("Scene");
        ImGui::ColorEdit3("Ambient Color", &scene.info.ambientColor.x);
        ImGui::Checkbox("Use Shadows", (bool*)&scene.info.useShadows);
        ImGui::Checkbox("Use Skybox", (bool*)&scene.info.useSkybox);
        ImGui::End();

        //-----------------------------------------------------------------------------
        // directional shadow pass
        //-----------------------------------------------------------------------------
        ctx->OMSetRenderTargets(0, nullptr, shadowmap.shadowDepthView);
        ctx->RSSetViewports(1u, &shadowviewport);
        ctx->RSSetState(shadowRasterizationState);

        for (int i = 0; i < am.sceneCount; i++)
            Renderer::mvRenderSceneShadows(am, am.scenes[i].scene, mvCreateOrthoView(directionalShadowCamera), mvCreateOrthoProjection(directionalShadowCamera));

        //-----------------------------------------------------------------------------
        // omni shadow pass
        //-----------------------------------------------------------------------------
        for (u32 i = 0; i < 6; i++)
        {
            ctx->OMSetRenderTargets(0, nullptr, shadowcubemap.shadowDepthView[i]);
            ctx->RSSetViewports(1u, &oshadowviewport);
            ctx->RSSetState(shadowRasterizationState);

            mvVec3 look_target = lightCamera.pos + shadowcubemap.cameraDirections[i];
            mvMat4 camera_matrix = mvLookAtLH(lightCamera.pos, look_target, shadowcubemap.cameraUps[i]);

            for (int i = 0; i < am.sceneCount; i++)
                Renderer::mvRenderSceneShadows(am, am.scenes[i].scene, camera_matrix, mvPerspectiveLH(M_PI_2, 1.0f, 0.5f, 100.0f));
        }

        //-----------------------------------------------------------------------------
        // main pass
        //-----------------------------------------------------------------------------
        ctx->OMSetRenderTargets(1, GContext->graphics.target.GetAddressOf(), *GContext->graphics.targetDepth.GetAddressOf());
        ctx->RSSetViewports(1u, &viewport);

        shadowCameraInfo.pointShadowView = mvCreateLookAtView(lightCamera);
        shadowCameraInfo.directShadowView = mvCreateOrthoView(directionalShadowCamera);
        shadowCameraInfo.directShadowProjection = mvCreateOrthoProjection(directionalShadowCamera);

        mvUpdateCameraFPSCamera(camera, dt, 12.0f, 0.004f);
        mvMat4 viewMatrix = mvCreateFPSView(camera);
        mvMat4 projMatrix = mvCreateLookAtProjection(camera);

        {
            mvVec4 posCopy = pointLightInfo.viewLightPos;

            mvVec4 out = viewMatrix * pointLightInfo.viewLightPos;
            pointLightInfo.viewLightPos.x = out.x;
            pointLightInfo.viewLightPos.y = out.y;
            pointLightInfo.viewLightPos.z = out.z;

            mvUpdateConstBuffer(pointLightBuffer, &pointLightInfo);
            pointLightInfo.viewLightPos = posCopy;
        }

        {
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
        mvUpdateConstBuffer(scene.buffer, &scene.info);
        mvUpdateConstBuffer(shadowCameraBuffer, &shadowCameraInfo);

        // vertex constant buffers
        ctx->VSSetConstantBuffers(1u, 1u, &shadowCameraBuffer.buffer);

        // pixel constant buffers
        ctx->PSSetConstantBuffers(0u, 1u, &pointLightBuffer.buffer);
        ctx->PSSetConstantBuffers(2u, 1u, &directionLightBuffer.buffer);
        ctx->PSSetConstantBuffers(3u, 1u, &scene.buffer.buffer);
        
        // samplers
        ctx->PSSetSamplers(1u, 1, &shadowSampler);
        ctx->PSSetSamplers(2u, 1, &omniShadowSampler);

        // textures
        ctx->PSSetShaderResources(3u, 1, &shadowmap.shadowResourceView);
        ctx->PSSetShaderResources(4u, 1, &shadowcubemap.shadowResourceView);

        //Renderer::mvRenderMesh(am, lightMesh, mvTranslate(mvIdentityMat4(), lightCamera.pos), viewMatrix, projMatrix);
        // render light mesh
        {
            mvSetPipelineState(lightPipeline);

            mvTransforms transforms{};
            transforms.model = mvTranslate(mvIdentityMat4(), lightCamera.pos);
            transforms.modelView = viewMatrix * transforms.model;
            transforms.modelViewProjection = projMatrix * viewMatrix * transforms.model;

            D3D11_MAPPED_SUBRESOURCE mappedSubresource;
            ctx->Map(GContext->graphics.tranformCBuf.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &mappedSubresource);
            memcpy(mappedSubresource.pData, &transforms, sizeof(mvTransforms));
            ctx->Unmap(GContext->graphics.tranformCBuf.Get(), 0u);

            // mesh
            mv_local_persist const UINT offset = 0u;
            ctx->VSSetConstantBuffers(0u, 1u, GContext->graphics.tranformCBuf.GetAddressOf());
            ctx->IASetIndexBuffer(am.buffers[lightMesh.indexBuffer].buffer.buffer, DXGI_FORMAT_R32_UINT, 0u);
            static mvVertexLayout lightvertexlayout = mvCreateVertexLayout({ mvVertexElement::Position3D });
            ctx->IASetVertexBuffers(0u, 1u,
                &am.buffers[lightMesh.vertexBuffer].buffer.buffer,
                &lightvertexlayout.size, &offset);

            // draw
            ctx->DrawIndexed(am.buffers[lightMesh.indexBuffer].buffer.size / sizeof(u32), 0u, 0u);
        }

        for (int i = 0; i < am.sceneCount; i++)
        {
            Renderer::mvRenderScene(am, am.scenes[i].scene, viewMatrix, projMatrix);
        }

        //-----------------------------------------------------------------------------
        // skybox pass
        //-----------------------------------------------------------------------------
        ctx->OMSetRenderTargets(1, GContext->graphics.target.GetAddressOf(), *GContext->graphics.targetDepth.GetAddressOf());
        ctx->RSSetViewports(1u, &viewport);
        ctx->PSSetConstantBuffers(0u, 1u, &scene.buffer.buffer);
        {

            // pipeline
            mvSetPipelineState(skyboxPipeline);
            ctx->PSSetSamplers(0, 1, &skybox.cubeSampler);
            ctx->PSSetShaderResources(0, 1, skybox.cubeTexture.textureView.GetAddressOf());

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

