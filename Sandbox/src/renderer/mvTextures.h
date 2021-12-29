#pragma once

#include <string>
#include <vector>
#include "mvTypes.h"
#include "mvGraphics.h"

struct mvTexture;
struct mvCubeTexture;
struct mvPBRTextures;

mvTexture     create_texture     (const std::string& path);
mvTexture     create_texture     (std::vector<unsigned char> data);
mvCubeTexture create_cube_texture(const std::string& path);

struct mvTexture
{
    ID3D11Texture2D*          texture     = nullptr;
    ID3D11ShaderResourceView* textureView = nullptr;
    b8                        alpha       = false;
};

struct mvCubeTexture
{
    ID3D11Texture2D*          texture     = nullptr;
    ID3D11ShaderResourceView* textureView = nullptr;
};

struct mvPBRTextures
{
    mvCubeTexture specular;
    mvTexture lut;
};

