#pragma once

#include <string>
#include <vector>
#include "mvTypes.h"
#include "mvGraphics.h"

struct mvTexture;
struct mvCubeTexture;

mvTexture     create_texture     (const std::string& path);
mvTexture     create_texture     (std::vector<unsigned char> data);
mvCubeTexture create_cube_texture(const std::string& path);

mvTexture create_dynamic_texture(u32 width, u32 height, u32 arraySize = 1);
void      update_dynamic_texture(mvTexture& texture, u32 width, u32 height, f32* data);

struct mvTexture
{
    ID3D11Texture2D*          texture     = nullptr;
    ID3D11ShaderResourceView* textureView = nullptr;
    b8                        alpha       = false;
    ID3D11SamplerState*       sampler     = nullptr;
};

struct mvCubeTexture
{
    ID3D11Texture2D*          texture     = nullptr;
    ID3D11ShaderResourceView* textureView = nullptr;
    //ID3D11SamplerState*       sampler     = nullptr;
};

