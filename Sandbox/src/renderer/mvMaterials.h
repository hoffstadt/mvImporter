#pragma once

#include <string>
#include "mvMath.h"
#include "mvBuffers.h"
#include "mvPipeline.h"
#include "mvTextures.h"

struct mvAssetManager;

struct mvMaterialData
{
    mvVec4 albedo = { 0.45f, 0.45f, 0.85f, 1.0f };
    //-------------------------- ( 16 bytes )

    f32 metalness = 1.0f;
    f32 roughness = 0.1f;
    f32 radiance  = 1.0f;
    f32 fresnel   = 0.04f;

    //-------------------------- ( 16 bytes )

    b32 useAlbedoMap    = false;
    b32 useNormalMap    = false;
    b32 useRoughnessMap = false;
    b32 useMetalMap     = false;
    //-------------------------- ( 16 bytes )

    
    mvVec3 emisiveFactor = { 0.0f, 0.0f, 0.0f };
    b32 useEmissiveMap = false;

    
    b32 hasAlpha = false;
    b32 useOcclusionMap = false;
    f32 occlusionStrength = 1.0f;
    char _pad0[4];
    //-------------------------- ( 16 bytes )

    //-------------------------- ( 4 * 16 = 64 bytes )
};

struct mvMaterial
{
    mvConstBuffer  buffer;
    mvMaterialData data;
    mvAssetID      pipeline;
    mvSampler      colorSampler;
};

mvMaterial mvCreateMaterial(mvAssetManager& am, const std::string& vs, const std::string& ps, mvMaterialData& materialData);
