#pragma once

#include "mvTypes.h"
#include "mvMath.h"
#include "mvWindows.h"
#include "mvSandbox.h"
#include <d3d11.h>

struct mvOffscreenPass
{
    ID3D11Texture2D*          texture;
    ID3D11Texture2D*          depthTexture;
    ID3D11RenderTargetView*   targetView = nullptr;
    ID3D11DepthStencilView*   depthView = nullptr;
    ID3D11ShaderResourceView* resourceView = nullptr;
    D3D11_VIEWPORT            viewport;

    mvOffscreenPass(float width, float height)
    {
        viewport = { 0.0f, 0.0f, width, height, 0.0f, 1.0f };

        resize(width, height);
    }

    void cleanup()
    {
        targetView->Release();
        depthView->Release();
        resourceView->Release();
        texture->Release();
        depthTexture->Release();
    }

    void recreate()
    {
        targetView->Release();
        depthView->Release();
        resourceView->Release();
        texture->Release();
        depthTexture->Release();
        resize(viewport.Width, viewport.Height);
    }

    void resize(float width, float height)
    {


        // render target texture
        {
            D3D11_TEXTURE2D_DESC textureDesc{};
            textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            textureDesc.MipLevels = 1;
            textureDesc.ArraySize = 1;
            textureDesc.SampleDesc.Count = 1;
            textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
            textureDesc.Height = static_cast<UINT>(height);
            textureDesc.Width = static_cast<UINT>(width);

            GContext->graphics.device->CreateTexture2D(
                &textureDesc,
                nullptr,
                &texture
            );
        }

        // depth stencil texture
        {
            D3D11_TEXTURE2D_DESC textureDesc{};
            textureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
            textureDesc.MipLevels = 1;
            textureDesc.ArraySize = 1;
            textureDesc.SampleDesc.Count = 1;
            textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
            textureDesc.Height = static_cast<UINT>(height);
            textureDesc.Width = static_cast<UINT>(width);

            GContext->graphics.device->CreateTexture2D(
                &textureDesc,
                nullptr,
                &depthTexture
            );
        }

        // render target view
        {
            D3D11_RENDER_TARGET_VIEW_DESC depthStencilViewDesc{};
            depthStencilViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            depthStencilViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
            depthStencilViewDesc.Texture2D.MipSlice = 0;

            GContext->graphics.device->CreateRenderTargetView(
                texture,
                &depthStencilViewDesc,
                &targetView
            );
        }

        // depth stencil view
        {
            D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
            depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
            depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
            depthStencilViewDesc.Texture2D.MipSlice = 0;

            GContext->graphics.device->CreateDepthStencilView(
                depthTexture,
                &depthStencilViewDesc,
                &depthView
            );
        }

        // shader resource view for render target
        {
            D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc{};
            shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            shaderResourceViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            shaderResourceViewDesc.Texture2D.MipLevels = 1;

            GContext->graphics.device->CreateShaderResourceView(
                texture,
                &shaderResourceViewDesc,
                &resourceView
            );
        }
    }

};