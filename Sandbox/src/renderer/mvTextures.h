#pragma once

#include <string>
#include "mvTypes.h"
#include "mvGraphics.h"

struct mvTexture
{
    mvComPtr<ID3D11Texture2D>          texture;
    mvComPtr<ID3D11ShaderResourceView> textureView;
    b8                                 alpha = false;
};

struct mvCubeTexture
{
    ID3D11ShaderResourceView* textureView;
};

struct mvSampler
{
    mvComPtr<ID3D11SamplerState> state;
};

mvTexture     mvCreateTexture    (const std::string& path);
mvCubeTexture mvCreateCubeTexture(const std::string& path);
mvSampler     mvCreateSampler    (D3D11_FILTER mode = D3D11_FILTER_ANISOTROPIC, D3D11_TEXTURE_ADDRESS_MODE addressing = D3D11_TEXTURE_ADDRESS_WRAP, b8 hwPcf=false);
