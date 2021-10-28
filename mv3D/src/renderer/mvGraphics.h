#pragma once

#include <d3d11_1.h>
#include <wrl.h>
#include <thread>
#include "mvWindows.h"

void mvSetupGraphics();
void mvCleanupGraphics();
void mvRecreateSwapChain();

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
};
