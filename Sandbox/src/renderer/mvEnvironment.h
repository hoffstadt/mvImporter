#pragma once

#include "mvTypes.h"
#include "mvTextures.h"

struct mvAssetManager;
struct mvEnvironment;

mvEnvironment create_environment(const std::string& path, i32 resolution, i32 sampleCount, f32 lodBias, i32 mipLevels);
void          cleanup_environment(mvEnvironment& environment);

struct mvEnvironment
{
    ID3D11SamplerState* sampler = nullptr;
    ID3D11SamplerState* brdfSampler = nullptr;
    mvCubeTexture       skyMap;
    mvCubeTexture       irradianceMap;
    mvCubeTexture       specularMap;
    mvTexture           brdfLUT;
};