#pragma once

#include "mvTypes.h"
#include "mvTextures.h"

// forward declarations
struct mvEnvironment;

mvEnvironment create_environment(const std::string& path, i32 resolution, i32 sampleCount, f32 lodBias, i32 mipLevels);
void          cleanup_environment(mvEnvironment& environment);

struct mvEnvironment
{
    mvComPtr<ID3D11SamplerState> sampler = nullptr;
    mvComPtr<ID3D11SamplerState> brdfSampler = nullptr;
    mvCubeTexture                skyMap;
    mvCubeTexture                irradianceMap;
    mvComPtr<ID3D11Texture2D>    specularTextureResource;
    mvCubeTexture                specularMap;
    mvTexture                    brdfLUT;
};