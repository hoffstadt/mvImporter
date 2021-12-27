#pragma once

#include <string>
#include <vector>
#include "mvTypes.h"
#include "mvGraphics.h"

struct mvTexture;
struct mvCubeTexture;
struct mvSampler;

mvTexture     create_texture        (const std::string& path);
mvTexture     create_texture        (std::vector<unsigned char> data);
mvCubeTexture create_cube_texture   (const std::string& path);
mvCubeTexture create_environment_map(const std::string& path);
mvSampler     create_sampler     (D3D11_FILTER mode = D3D11_FILTER_ANISOTROPIC, D3D11_TEXTURE_ADDRESS_MODE addressing = D3D11_TEXTURE_ADDRESS_WRAP, b8 hwPcf = false);

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

