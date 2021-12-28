#pragma once

#include <string>
#include <vector>
#include "mvTypes.h"
#include "mvGraphics.h"

struct mvTexture;
struct mvCubeTexture;
struct mvPBRTextures;
struct mvSampler;

mvTexture     create_texture        (const std::string& path);
mvTexture     create_texture        (std::vector<unsigned char> data);
mvCubeTexture create_cube_texture   (const std::string& path);
mvCubeTexture create_cube_map       (const std::string& path);
mvCubeTexture create_irradiance_map (mvCubeTexture& cubemap, i32 resolution, i32 sampleCount, f32 lodBias);
mvPBRTextures create_specular_map   (mvCubeTexture& cubemap, i32 resolution, i32 sampleCount, f32 lodBias);
mvSampler     create_sampler        (D3D11_FILTER mode = D3D11_FILTER_ANISOTROPIC, D3D11_TEXTURE_ADDRESS_MODE addressing = D3D11_TEXTURE_ADDRESS_WRAP, b8 hwPcf = false);

struct mvTexture
{
    mvComPtr<ID3D11Texture2D>          texture = nullptr;
    mvComPtr<ID3D11ShaderResourceView> textureView = nullptr;
    b8                                 alpha = false;
};

struct mvCubeTexture
{
    ID3D11Texture2D*          texture;
    ID3D11ShaderResourceView* textureView;
};

struct mvPBRTextures
{
    mvCubeTexture specular;
    mvTexture lut;
};

struct mvSampler
{
    mvComPtr<ID3D11SamplerState> state;
};

