#include "mvShadows.h"
#include "mv3D_internal.h"

mvShadowMap 
mvCreateShadowMap(u32 resolution)
{
	mvShadowMap shadow{};
    shadow.shadowMapDimension = resolution;

    D3D11_TEXTURE2D_DESC shadowMapDesc;
    ZeroMemory(&shadowMapDesc, sizeof(D3D11_TEXTURE2D_DESC));
    shadowMapDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
    shadowMapDesc.MipLevels = 1;
    shadowMapDesc.ArraySize = 1;
    shadowMapDesc.SampleDesc.Count = 1;
    shadowMapDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
    shadowMapDesc.Height = static_cast<UINT>(shadow.shadowMapDimension);
    shadowMapDesc.Width = static_cast<UINT>(shadow.shadowMapDimension);

    HRESULT hr = GContext->graphics.device->CreateTexture2D(
        &shadowMapDesc,
        nullptr,
        shadow.shadowMap.GetAddressOf()
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

    hr = GContext->graphics.device->CreateDepthStencilView(
        shadow.shadowMap.Get(),
        &depthStencilViewDesc,
        shadow.shadowDepthView.GetAddressOf()
    );

    hr = GContext->graphics.device->CreateShaderResourceView(
        shadow.shadowMap.Get(),
        &shaderResourceViewDesc,
        &shadow.shadowResourceView
    );

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
    //comparisonSamplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT;

    // Point filtered shadows can be faster, and may be a good choice when
    // rendering on hardware with lower feature levels. This sample has a
    // UI option to enable/disable filtering so you can see the difference
    // in quality and speed.

    GContext->graphics.device->CreateSamplerState(
        &comparisonSamplerDesc,
        shadow.comparisonSampler.GetAddressOf()
    );

    //D3D11_RASTERIZER_DESC drawingRenderStateDesc;
    //ZeroMemory(&drawingRenderStateDesc, sizeof(D3D11_RASTERIZER_DESC));
    //drawingRenderStateDesc.CullMode = D3D11_CULL_BACK;
    //drawingRenderStateDesc.FillMode = D3D11_FILL_SOLID;
    //drawingRenderStateDesc.DepthClipEnable = true; // Feature level 9_1 requires DepthClipEnable == true
    //GContext->graphics.device->CreateRasterizerState(
    //    &drawingRenderStateDesc,
    //    drawingRasterizationState.GetAddressOf()
    //);

    D3D11_RASTERIZER_DESC shadowRenderStateDesc;
    ZeroMemory(&shadowRenderStateDesc, sizeof(D3D11_RASTERIZER_DESC));
    shadowRenderStateDesc.CullMode = D3D11_CULL_FRONT;
    shadowRenderStateDesc.FrontCounterClockwise = TRUE;
    shadowRenderStateDesc.FillMode = D3D11_FILL_SOLID;
    shadowRenderStateDesc.DepthClipEnable = true;
    shadowRenderStateDesc.DepthBias = 50;
    shadowRenderStateDesc.DepthBiasClamp = 0.1f;
    shadowRenderStateDesc.SlopeScaledDepthBias = 2.0f;

    GContext->graphics.device->CreateRasterizerState(
        &shadowRenderStateDesc,
        shadow.shadowRasterizationState.GetAddressOf()
    );

	return shadow;
}

mvShadowCubeMap 
mvCreateShadowCubeMap(u32 resolution)
{
    mvShadowCubeMap shadowCube{};
    shadowCube.shadowMapDimension = resolution;

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
    GContext->graphics.device->CreateShaderResourceView(pTexture.Get(), &srvDesc, shadowCube.shadowResourceView.GetAddressOf());

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
        GContext->graphics.device->CreateDepthStencilView(pTexture.Get(), &descView, shadowCube.shadowDepthViews[face].GetAddressOf());
    }

    D3D11_RASTERIZER_DESC shadowRenderStateDesc;
    ZeroMemory(&shadowRenderStateDesc, sizeof(D3D11_RASTERIZER_DESC));
    shadowRenderStateDesc.CullMode = D3D11_CULL_BACK;
    shadowRenderStateDesc.FillMode = D3D11_FILL_SOLID;
    shadowRenderStateDesc.FrontCounterClockwise = TRUE;
    shadowRenderStateDesc.DepthClipEnable = true;
    shadowRenderStateDesc.DepthBias = 50;
    shadowRenderStateDesc.DepthBiasClamp = 0.1f;
    shadowRenderStateDesc.SlopeScaledDepthBias = 2.0f;

    GContext->graphics.device->CreateRasterizerState(
        &shadowRenderStateDesc,
        shadowCube.shadowRasterizationState.GetAddressOf()
    );

    D3D11_SAMPLER_DESC comparisonSamplerDesc;
    ZeroMemory(&comparisonSamplerDesc, sizeof(D3D11_SAMPLER_DESC));
    comparisonSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    comparisonSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    comparisonSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    comparisonSamplerDesc.BorderColor[0] = 1.0f;
    //comparisonSamplerDesc.BorderColor[1] = 1.0f;
    //comparisonSamplerDesc.BorderColor[2] = 1.0f;
    //comparisonSamplerDesc.BorderColor[3] = 1.0f;
    comparisonSamplerDesc.MinLOD = 0.f;
    comparisonSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
    comparisonSamplerDesc.MipLODBias = 0.f;
    comparisonSamplerDesc.MaxAnisotropy = 0;
    comparisonSamplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
    comparisonSamplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
    //comparisonSamplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT;

    // Point filtered shadows can be faster, and may be a good choice when
    // rendering on hardware with lower feature levels. This sample has a
    // UI option to enable/disable filtering so you can see the difference
    // in quality and speed.

    GContext->graphics.device->CreateSamplerState(
        &comparisonSamplerDesc,
        shadowCube.comparisonSampler.GetAddressOf()
    );

    shadowCube.cameraDirections[0] = {  1.0f,  0.0f,  0.0f };
    shadowCube.cameraDirections[1] = { -1.0f,  0.0f,  0.0f };
    shadowCube.cameraDirections[2] = {  0.0f,  1.0f,  0.0f };
    shadowCube.cameraDirections[3] = {  0.0f, -1.0f,  0.0f };
    shadowCube.cameraDirections[4] = {  0.0f,  0.0f,  1.0f };
    shadowCube.cameraDirections[5] = {  0.0f,  0.0f, -1.0f };

    shadowCube.cameraUps[0] = { 0.0f, 1.0f,  0.0f };
    shadowCube.cameraUps[1] = { 0.0f, 1.0f,  0.0f };
    shadowCube.cameraUps[2] = { 0.0f, 0.0f, -1.0f };
    shadowCube.cameraUps[3] = { 0.0f, 0.0f,  1.0f };
    shadowCube.cameraUps[4] = { 0.0f, 1.0f,  0.0f };
    shadowCube.cameraUps[5] = { 0.0f, 1.0f,  0.0f };

    return shadowCube;
}

mvShadowCamera 
mvCreateShadowCamera()
{
    mvShadowCamera shadowCamera{};
    shadowCamera.buffer = mvCreateConstBuffer(&shadowCamera.info, sizeof(mvShadowCamera));
    return shadowCamera;
}

void 
mvBindSlot_tsPS(mvShadowMap& shadowMap, u32 textureSlot, u32 samplerSlot)
{

    GContext->graphics.imDeviceContext->PSSetSamplers(samplerSlot, 1, shadowMap.comparisonSampler.GetAddressOf());
    GContext->graphics.imDeviceContext->PSSetShaderResources(textureSlot, 1, shadowMap.shadowResourceView.GetAddressOf());
}

void
mvBindSlot_tsPS(mvShadowCubeMap& shadowMap, u32 textureSlot, u32 samplerSlot)
{

    GContext->graphics.imDeviceContext->PSSetSamplers(samplerSlot, 1, shadowMap.comparisonSampler.GetAddressOf());
    GContext->graphics.imDeviceContext->PSSetShaderResources(textureSlot, 1, shadowMap.shadowResourceView.GetAddressOf());
}

void 
mvBindSlot_bVS(u32 slot, mvShadowCamera& camera, mvMat4 oview, mvMat4 dview, mvMat4 proj)
{
    camera.info.pointShadowView = oview;
    camera.info.directShadowView = dview;
    camera.info.directShadowProjection = proj;

    mvUpdateConstBuffer(camera.buffer, &camera.info);
    GContext->graphics.imDeviceContext->VSSetConstantBuffers(slot, 1u, camera.buffer.buffer.GetAddressOf());
}