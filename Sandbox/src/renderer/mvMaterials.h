#pragma once

#include <string>
#include "mvMath.h"
#include "mvBuffers.h"
#include "mvPipeline.h"
#include "mvTextures.h"

struct mvObjMaterial;

struct mvPhongMaterialData
{
    mvVec4 materialColor = { 0.45f, 0.45f, 0.85f, 1.0f };
    //-------------------------- ( 16 bytes )

    mvVec3 specularColor = { 0.18f, 0.18f, 0.18f };
    f32    specularGloss = 8.0f;
    //-------------------------- ( 16 bytes )

    f32 normalMapWeight = 1.0f;
    b32 useTextureMap   = false;
    b32 useNormalMap    = false;
    b32 useSpecularMap  = false;
    //-------------------------- ( 16 bytes )

    b32 useGlossAlpha = false;
    b32 hasAlpha      = false;
    char _pad1[8];
    //-------------------------- ( 16 bytes )
    //-------------------------- ( 4 * 16 = 64 bytes )
};

struct mvPBRMaterialData
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

struct mvPhongMaterial
{
    mvConstBuffer       buffer;
    mvPhongMaterialData data;
    mvPipeline          pipeline;
    mvSampler           colorSampler;
};

struct mvPBRMaterial
{
    mvConstBuffer     buffer;
    mvPBRMaterialData data;
    mvPipeline        pipeline;
    mvSampler         colorSampler;
};

mvPhongMaterial mvCreatePhongMaterial(const std::string& vs, const std::string& ps, b8 cull, b8 useDiffusemap, b8 useNormalmap, b8 useSpecularMap);
mvPBRMaterial   mvCreatePBRMaterial  (const std::string& vs, const std::string& ps, mvPBRMaterialData& materialData);
