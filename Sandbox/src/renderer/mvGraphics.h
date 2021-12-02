#pragma once

#include <d3d11_1.h>
#include <wrl.h>
#include <thread>
#include "mvWindows.h"

struct mvViewport;
struct mvPipeline;

void mvSetupGraphics(mvViewport& viewport);
void mvCleanupGraphics();
void mvRecreateSwapChain(unsigned width, unsigned height);
void mvSetPipelineState(mvPipeline& pipeline);

template <typename T>
using mvComPtr = Microsoft::WRL::ComPtr<T>;

struct mvGraphics
{
    mvComPtr<ID3D11Buffer>             tranformCBuf;
    mvComPtr<ID3D11Texture2D>          frameBuffer;
    mvComPtr<ID3D11RenderTargetView>   target;
    mvComPtr<ID3D11DepthStencilView>   targetDepth;
    mvComPtr<IDXGISwapChain>           swapChain;
    mvComPtr<ID3D11DeviceContext>      imDeviceContext;  
    mvComPtr<ID3D11Device>             device;
    std::thread::id                    threadID;
    D3D11_VIEWPORT                     viewport;
};
