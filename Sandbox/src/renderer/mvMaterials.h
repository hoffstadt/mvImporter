#pragma once

#include <string>
#include "mvMath.h"
#include "mvBuffers.h"
#include "mvPipeline.h"
#include "mvTextures.h"

struct mvMaterialData
{
    mvVec4 albedo = { 0.45f, 0.45f, 0.85f, 1.0f };
    //-------------------------- ( 16 bytes )

    f32 metalness = 0.5f;
    f32 roughness = 0.5f;
    f32 radiance  = 1.0f;
    f32 fresnel   = 0.04f;

    //-------------------------- ( 16 bytes )

    b32 useAlbedoMap    = false;
    b32 useNormalMap    = false;
    b32 useRoughnessMap = false;
    b32 useMetalMap     = false;
    //-------------------------- ( 16 bytes )

    b32 hasAlpha = false;
    char _pad1[12];
    //-------------------------- ( 16 bytes )

    //-------------------------- ( 4 * 16 = 64 bytes )
};

struct mvMaterial
{
    mvConstBuffer  buffer;
    mvMaterialData data;
    mvPipeline     pipeline;
    mvPipeline     shadowPipeline;
    mvSampler      colorSampler;
};

mvMaterial mvCreateMaterial(const std::string& vs, const std::string& ps, mvMaterialData& materialData);
