#pragma once

#include "sMath.h"
#include "mvWindows.h"
#include "mvGraphics.h"

struct mvOffscreenPass
{
    Microsoft::WRL::ComPtr<ID3D11Texture2D>          texture;
    Microsoft::WRL::ComPtr<ID3D11Texture2D>          depthTexture;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView>   targetView = nullptr;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView>   depthView = nullptr;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> resourceView = nullptr;
    D3D11_VIEWPORT                                   viewport;

    mvOffscreenPass(mvGraphics& graphics, float width, float height)
    {
        viewport = { 0.0f, 0.0f, width, height, 0.0f, 1.0f };

        resize(graphics, width, height);
    }

    void cleanup()
    {
        targetView = nullptr;
        depthView = nullptr;
        resourceView = nullptr;
        texture = nullptr;
        depthTexture = nullptr;
    }

    void recreate(mvGraphics& graphics)
    {
        targetView = nullptr;
        depthView = nullptr;
        resourceView = nullptr;
        texture = nullptr;
        depthTexture = nullptr;
        resize(graphics, viewport.Width, viewport.Height);
    }

    void resize(mvGraphics& graphics, float width, float height)
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

            graphics.device->CreateTexture2D(
                &textureDesc,
                nullptr,
                texture.GetAddressOf()
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

            graphics.device->CreateTexture2D(
                &textureDesc,
                nullptr,
                depthTexture.GetAddressOf()
            );
        }

        // render target view
        {
            D3D11_RENDER_TARGET_VIEW_DESC depthStencilViewDesc{};
            depthStencilViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            depthStencilViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
            depthStencilViewDesc.Texture2D.MipSlice = 0;

            graphics.device->CreateRenderTargetView(
                texture.Get(),
                &depthStencilViewDesc,
                targetView.GetAddressOf()
            );
        }

        // depth stencil view
        {
            D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
            depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
            depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
            depthStencilViewDesc.Texture2D.MipSlice = 0;

            graphics.device->CreateDepthStencilView(
                depthTexture.Get(),
                &depthStencilViewDesc,
                depthView.GetAddressOf()
            );
        }

        // shader resource view for render target
        {
            D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc{};
            shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            shaderResourceViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            shaderResourceViewDesc.Texture2D.MipLevels = 1;

            graphics.device->CreateShaderResourceView(
                texture.Get(),
                &shaderResourceViewDesc,
                resourceView.GetAddressOf()
            );
        }
    }

};