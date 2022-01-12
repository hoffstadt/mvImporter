#pragma once

#include "mvMath.h"
#include "mvTypes.h"
#include "mvBuffers.h"
#include "mvCamera.h"
#include "mvSandbox.h"
#include "mvAssetManager.h"

struct DirectionalShadowTransformInfo
{
    mvMat4 view = identity_mat4();
    mvMat4 projection = identity_mat4();
};

struct OmniShadowTransformInfo
{
    mvMat4 view = identity_mat4();
};

struct mvDirectionalShadowPass
{
    ID3D11Texture2D*               texture;
    ID3D11DepthStencilView*        depthView;
    ID3D11ShaderResourceView*      resourceView;
    ID3D11RasterizerState*         rasterizationState;
    ID3D11SamplerState*            sampler;
    mvCamera                       camera;
    D3D11_VIEWPORT                 viewport;
    mvConstBuffer                  buffer;
    DirectionalShadowTransformInfo info;
    f32                            angle = 10.0f;
    f32                            width;
    i32                            depthBias = 50;
    f32                            slopeBias = 2.0f;
    b8                             backface = false;

    mvDirectionalShadowPass(mvAssetManager& am, u32 resolution, f32 w)
    {
        width = w;
        viewport = { 0.0f, 0.0f, (f32)resolution, (f32)resolution, 0.0f, 1.0f };

        buffer = create_const_buffer(&info, sizeof(DirectionalShadowTransformInfo));

        // setup camera
        f32 zcomponent = sinf(M_PI * angle / 180.0f);
        f32 ycomponent = cosf(M_PI * angle / 180.0f);

        camera = create_ortho_camera(
            { 0.0f, 100.0f, 0.0f },
            { 0.0f, -ycomponent, zcomponent },
            width * 2.0f,
            width * 2.0f,
            -121.0f,
            121.0f
        );

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
            &texture
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
            texture,
            &depthStencilViewDesc,
            &depthView
        );

        GContext->graphics.device->CreateShaderResourceView(
            texture,
            &shaderResourceViewDesc,
            &resourceView
        );

        D3D11_RASTERIZER_DESC shadowRenderStateDesc;
        ZeroMemory(&shadowRenderStateDesc, sizeof(D3D11_RASTERIZER_DESC));
        shadowRenderStateDesc.CullMode = D3D11_CULL_FRONT;
        shadowRenderStateDesc.FrontCounterClockwise = true;
        shadowRenderStateDesc.FillMode = D3D11_FILL_SOLID;
        shadowRenderStateDesc.DepthClipEnable = false;
        shadowRenderStateDesc.DepthBias = depthBias;
        shadowRenderStateDesc.DepthBiasClamp = 0.0f;
        shadowRenderStateDesc.SlopeScaledDepthBias = slopeBias;

        GContext->graphics.device->CreateRasterizerState(&shadowRenderStateDesc, &rasterizationState);

        D3D11_SAMPLER_DESC comparisonSamplerDesc{};
        ZeroMemory(&comparisonSamplerDesc, sizeof(D3D11_SAMPLER_DESC));
        comparisonSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
        comparisonSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
        comparisonSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        comparisonSamplerDesc.BorderColor[0] = 0.0f;
        comparisonSamplerDesc.BorderColor[1] = 0.0f;
        comparisonSamplerDesc.BorderColor[2] = 0.0f;
        comparisonSamplerDesc.BorderColor[3] = 1.0f;
        comparisonSamplerDesc.MinLOD = 0.0f;
        comparisonSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
        comparisonSamplerDesc.MipLODBias = 0.0f;
        comparisonSamplerDesc.MaxAnisotropy = D3D11_REQ_MAXANISOTROPY;
        comparisonSamplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
        comparisonSamplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT;

        GContext->graphics.device->CreateSamplerState(&comparisonSamplerDesc, &sampler);

        info.view = getViewMatrix();
        info.projection = getProjectionMatrix();

        register_asset(&am, "shadowmap_depthview", depthView);
        register_asset(&am, "shadowmap_sampler", sampler);
        register_asset(&am, "shadowmap_buffer", buffer);

    }

    void cleanup()
    {
        texture->Release();
        resourceView->Release();
        rasterizationState->Release();
    }

    void recreate()
    {
        rasterizationState->Release();

        D3D11_RASTERIZER_DESC shadowRenderStateDesc;
        ZeroMemory(&shadowRenderStateDesc, sizeof(D3D11_RASTERIZER_DESC));
        shadowRenderStateDesc.CullMode = backface ? D3D11_CULL_BACK : D3D11_CULL_FRONT;
        shadowRenderStateDesc.FrontCounterClockwise = true;
        shadowRenderStateDesc.FillMode = D3D11_FILL_SOLID;
        shadowRenderStateDesc.DepthClipEnable = true;
        shadowRenderStateDesc.DepthBias = depthBias;
        shadowRenderStateDesc.DepthBiasClamp = 0.0f;
        shadowRenderStateDesc.SlopeScaledDepthBias = slopeBias;

        GContext->graphics.device->CreateRasterizerState(&shadowRenderStateDesc, &rasterizationState);
    }

    mvMat4 getViewMatrix()
    {
        return lookat(camera.pos, camera.pos - camera.dir, camera.up);
    }

    mvMat4 getProjectionMatrix()
    {
        return ortho(
            -camera.width / 2.0f,
            camera.width / 2.0f,
            -camera.height / 2.0f,
            camera.height / 2.0f,
            camera.nearZ,
            camera.farZ);
    }
};

struct mvOmniShadowPass
{
    ID3D11Texture2D*          texture = nullptr;
    ID3D11DepthStencilView*   depthView[6]{};
    ID3D11ShaderResourceView* resourceView = nullptr;
    ID3D11RasterizerState*    rasterizationState;
    mvVec3                    cameraDirections[6];
    mvVec3                    cameraUps[6];
    ID3D11SamplerState*       sampler = nullptr;
    D3D11_VIEWPORT            viewport;
    mvConstBuffer             buffer;
    OmniShadowTransformInfo   info;
    i32                       depthBias = 40;
    f32                       slopeBias = 2.1f;

    mvOmniShadowPass(mvAssetManager& am, u32 resolution)
    {
        viewport = { 0.0f, 0.0f, (f32)resolution, (f32)resolution, 0.0f, 1.0f };

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
        GContext->graphics.device->CreateShaderResourceView(pTexture.Get(), &srvDesc, &resourceView);

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
            GContext->graphics.device->CreateDepthStencilView(pTexture.Get(), &descView, &(depthView[face]));
        }

        cameraDirections[0] = { 0.0f,  0.0f,  1.0f };
        cameraDirections[1] = { 0.0f,  0.0f,  -1.0f };
        cameraDirections[2] = { 0.0f,  -1.0f,  0.0f };
        cameraDirections[3] = { 0.0f,  1.0f,  0.0f };
        cameraDirections[4] = { 1.0f,  0.0f,  0.0f };
        cameraDirections[5] = { -1.0f,  0.0f,   0.0f };

        cameraUps[0] = { 0.0f,  1.0f,  0.0f };
        cameraUps[1] = { 0.0f,  1.0f,  0.0f };
        cameraUps[2] = { 1.0f, 0.0f,   0.0f };
        cameraUps[3] = { -1.0f, 0.0f,   0.0f };
        cameraUps[4] = { 0.0f,  1.0f,  0.0f };
        cameraUps[5] = { 0.0f,  1.0f,  0.0f };

        //cameraDirections[0] = { 1.0f,  0.0f,  0.0f };
        //cameraDirections[1] = { -1.0f,  0.0f,  0.0f };
        //cameraDirections[2] = { 0.0f,  1.0f,  0.0f };
        //cameraDirections[3] = { 0.0f,  -1.0f,  0.0f };
        //cameraDirections[4] = { 0.0f,  0.0f,  1.0f };
        //cameraDirections[5] = { 0.0f,  0.0f,   -1.0f };

        //cameraUps[0] = { 0.0f,  1.0f,  0.0f };
        //cameraUps[1] = { 0.0f,  1.0f,  0.0f };
        //cameraUps[2] = { 0.0f, 0.0f,   -1.0f };
        //cameraUps[3] = { 0.0f, 0.0f,   1.0f };
        //cameraUps[4] = { 0.0f,  1.0f,  0.0f };
        //cameraUps[5] = { 0.0f,  1.0f,  0.0f };


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
        comparisonSamplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
        GContext->graphics.device->CreateSamplerState(&comparisonSamplerDesc, &sampler);

        D3D11_RASTERIZER_DESC shadowRenderStateDesc;
        ZeroMemory(&shadowRenderStateDesc, sizeof(D3D11_RASTERIZER_DESC));
        shadowRenderStateDesc.CullMode = D3D11_CULL_BACK;
        shadowRenderStateDesc.FrontCounterClockwise = true;
        shadowRenderStateDesc.FillMode = D3D11_FILL_SOLID;
        shadowRenderStateDesc.DepthClipEnable = true;
        shadowRenderStateDesc.DepthBias = depthBias;
        shadowRenderStateDesc.DepthBiasClamp = 0.0f;
        shadowRenderStateDesc.SlopeScaledDepthBias = slopeBias;

        GContext->graphics.device->CreateRasterizerState(&shadowRenderStateDesc, &rasterizationState);

        buffer = create_const_buffer(&info, sizeof(OmniShadowTransformInfo));

        register_asset(&am, "oshadowmap_sampler", sampler);
        register_asset(&am, "oshadowmap_buffer", buffer);
        for (int i = 0; i < 6; i++)
            register_asset(&am, "oshadowmap_depthview" + std::to_string(i), depthView[i]);

    }

    void cleanup()
    {
        resourceView->Release();
        rasterizationState->Release();
    }

    void recreate()
    {
        rasterizationState->Release();

        D3D11_RASTERIZER_DESC shadowRenderStateDesc;
        ZeroMemory(&shadowRenderStateDesc, sizeof(D3D11_RASTERIZER_DESC));
        shadowRenderStateDesc.CullMode = D3D11_CULL_BACK;
        shadowRenderStateDesc.FrontCounterClockwise = true;
        shadowRenderStateDesc.FillMode = D3D11_FILL_SOLID;
        shadowRenderStateDesc.DepthClipEnable = true;
        shadowRenderStateDesc.DepthBias = depthBias;
        shadowRenderStateDesc.DepthBiasClamp = 0.0f;
        shadowRenderStateDesc.SlopeScaledDepthBias = slopeBias;

        GContext->graphics.device->CreateRasterizerState(&shadowRenderStateDesc, &rasterizationState);
    }
};