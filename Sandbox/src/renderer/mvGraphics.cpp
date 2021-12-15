#include "mvGraphics.h"
#include <assert.h>
#include "mvSandbox.h"
#include "mvMath.h"
#include "mvPipeline.h"

void
setup_graphics(mvViewport& viewport)
{
    GContext->graphics.threadID = std::this_thread::get_id();

    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferDesc.Width = viewport.width;
    sd.BufferDesc.Height = viewport.height;
    sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 0;
    sd.BufferDesc.RefreshRate.Denominator = 0;
    sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.BufferCount = 2;
    sd.OutputWindow = viewport.hWnd;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; //DXGI_SWAP_EFFECT_DISCARD;
    sd.Flags = 0;

    UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef MV_DEBUG
    creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif // MV_DEBUG


    // create device and front/back buffers, and swap chain and rendering context
    HRESULT hResult = D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        creationFlags,
        nullptr,
        0,
        D3D11_SDK_VERSION,
        &sd,
        &GContext->graphics.swapChain,
        &GContext->graphics.device,
        nullptr,
        &GContext->graphics.imDeviceContext
    );
    assert(SUCCEEDED(hResult));

    // Create Framebuffer Render Target
    hResult = GContext->graphics.swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)GContext->graphics.frameBuffer.GetAddressOf());
    assert(SUCCEEDED(hResult));

    // create render target
    D3D11_TEXTURE2D_DESC textureDesc;
    GContext->graphics.frameBuffer->GetDesc(&textureDesc);

    // create the target view on the texture
    D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
    rtvDesc.Format = textureDesc.Format;
    rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    rtvDesc.Texture2D = D3D11_TEX2D_RTV{ 0 };

    hResult = GContext->graphics.device->CreateRenderTargetView(GContext->graphics.frameBuffer.Get(), &rtvDesc, GContext->graphics.target.GetAddressOf());
    assert(SUCCEEDED(hResult));

    // create transform constant buffer
    D3D11_BUFFER_DESC cbd;
    cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbd.Usage = D3D11_USAGE_DYNAMIC;
    cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    cbd.MiscFlags = 0u;
    cbd.ByteWidth = sizeof(mvTransforms);
    cbd.StructureByteStride = 0u;

    GContext->graphics.device->CreateBuffer(&cbd, nullptr, GContext->graphics.tranformCBuf.GetAddressOf());

    // create depth stensil texture
    mvComPtr<ID3D11Texture2D> pDepthStencil;
    D3D11_TEXTURE2D_DESC descDepth = {};
    descDepth.Width = viewport.width;
    descDepth.Height = viewport.height;
    descDepth.MipLevels = 1u;
    descDepth.ArraySize = 1u;
    descDepth.Format = DXGI_FORMAT_R24G8_TYPELESS;
    descDepth.SampleDesc.Count = 1u;
    descDepth.SampleDesc.Quality = 0u;
    descDepth.Usage = D3D11_USAGE_DEFAULT;
    descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    GContext->graphics.device->CreateTexture2D(&descDepth, nullptr, &pDepthStencil);

    // create view of depth stensil texture
    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
    descDSV.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    descDSV.Texture2D.MipSlice = 0u;
    descDSV.Flags = 0;
    GContext->graphics.device->CreateDepthStencilView(pDepthStencil.Get(), &descDSV, GContext->graphics.targetDepth.GetAddressOf());

    GContext->graphics.viewport = { 0.0f, 0.0f, (f32)viewport.width, (f32)viewport.height, 0.0f, 1.0f };
}

void
recreate_swapchain(unsigned width, unsigned height)
{
    if (GContext->graphics.device)
    {
        GContext->graphics.target->Release();
        GContext->graphics.targetDepth->Release();
        GContext->graphics.imDeviceContext->OMSetRenderTargets(0, 0, 0);
        GContext->graphics.frameBuffer->Release();

        GContext->graphics.swapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);

        // Create Framebuffer Render Target
        GContext->graphics.swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)GContext->graphics.frameBuffer.GetAddressOf());

        // create render target
        D3D11_TEXTURE2D_DESC textureDesc;
        GContext->graphics.frameBuffer->GetDesc(&textureDesc);

        // create the target view on the texture
        D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
        rtvDesc.Format = textureDesc.Format;
        rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
        rtvDesc.Texture2D = D3D11_TEX2D_RTV{ 0 };

        HRESULT hResult = GContext->graphics.device->CreateRenderTargetView(GContext->graphics.frameBuffer.Get(), &rtvDesc, GContext->graphics.target.GetAddressOf());
        assert(SUCCEEDED(hResult));

        // create depth stensil texture
        mvComPtr<ID3D11Texture2D> pDepthStencil;
        D3D11_TEXTURE2D_DESC descDepth = {};
        descDepth.Width = width;
        descDepth.Height = height;
        descDepth.MipLevels = 1u;
        descDepth.ArraySize = 1u;
        descDepth.Format = DXGI_FORMAT_R24G8_TYPELESS;
        descDepth.SampleDesc.Count = 1u;
        descDepth.SampleDesc.Quality = 0u;
        descDepth.Usage = D3D11_USAGE_DEFAULT;
        descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        GContext->graphics.device->CreateTexture2D(&descDepth, nullptr, &pDepthStencil);

        // create view of depth stensil texture
        D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
        descDSV.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        descDSV.Texture2D.MipSlice = 0u;
        descDSV.Flags = 0;
        GContext->graphics.device->CreateDepthStencilView(pDepthStencil.Get(), &descDSV, GContext->graphics.targetDepth.GetAddressOf());

        GContext->graphics.viewport.Width = width;
        GContext->graphics.viewport.Height = height;

    }
}

void 
set_pipeline_state(mvPipeline& pipeline)
{
    auto device = GContext->graphics.imDeviceContext;
    device->IASetPrimitiveTopology(pipeline.topology);
    device->RSSetState(pipeline.rasterizationState);
    device->OMSetBlendState(pipeline.blendState, nullptr, 0xFFFFFFFFu);
    device->OMSetDepthStencilState(pipeline.depthStencilState, 0xFF);;
    device->IASetInputLayout(pipeline.inputLayout);
    device->VSSetShader(pipeline.vertexShader, nullptr, 0);
    device->PSSetShader(pipeline.pixelShader, nullptr, 0);
    device->HSSetShader(nullptr, nullptr, 0);
    device->DSSetShader(nullptr, nullptr, 0);
    device->GSSetShader(nullptr, nullptr, 0);
}