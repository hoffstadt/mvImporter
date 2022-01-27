#pragma once

#include <string>
#include "mvMath.h"
#include "mvBuffers.h"
#include "mvPipeline.h"
#include "mvTextures.h"

// forward declarations
struct mvMaterial;
struct mvMaterialData;
struct mvVertexLayout;
struct mvMaterialAsset;
struct mvMaterialManager;

mvMaterial  create_material(const std::string& vs, const std::string& ps, mvMaterial materialInfo);
std::string hash_material (const mvMaterial& materialInfo, const mvVertexLayout& layout, const std::string& pixelShader, const std::string& vertexShader);
mvAssetID   register_asset(mvMaterialManager* manager, const std::string& tag, mvMaterial asset);
void        clear_materials(mvMaterialManager* manager);
void        reload_materials(mvMaterialManager* manager);
mvAssetID   mvGetMaterialAssetID(mvMaterialManager* manager, const std::string& tag);
mvMaterial* mvGetRawMaterialAsset(mvMaterialManager* manager, const std::string& tag);

struct mvMaterialData
{
    mvVec4 albedo = { 0.45f, 0.45f, 0.85f, 1.0f };
    //-------------------------- ( 16 bytes )

    f32 metalness = 1.0f;
    f32 roughness = 0.1f;
    f32 radiance  = 1.0f;
    f32 fresnel   = 0.04f;
    //-------------------------- ( 16 bytes )

    mvVec3 emisiveFactor = { 0.0f, 0.0f, 0.0f };
    f32 occlusionStrength = 1.0f;
    //-------------------------- ( 16 bytes )

    b32 doubleSided = false;
    f32 alphaCutoff;
    f32 clearcoatFactor = 0.0;
    f32 clearcoatRoughnessFactor = 0.0;
    //-------------------------- ( 16 bytes )

    f32 normalScale              = 1.0f;
    f32 clearcoatNormalScale     = 1.0f;
    f32 transmissionFactor       = 1.0f;
    f32 thicknessFactor          = 1.0f;
    //-------------------------- ( 16 bytes )

    mvVec4 attenuationColor = { 0.45f, 0.45f, 0.85f, 1.0f };
    //-------------------------- ( 16 bytes )

    mvVec3 sheenColorFactor = { 0.0f, 0.0f, 0.0f };
    f32 sheenRoughnessFactor = 1.0f;
    //-------------------------- ( 16 bytes )

    f32 attenuationDistance = 1.0f;
    f32 ior = 1.0f;
    char _padding[8];
    //-------------------------- ( 16 bytes )

};

struct mvMaterial
{
    mvConstBuffer              buffer;
    mvMaterialData             data;
    mvPipeline                 pipeline; 
    std::vector<mvShaderMacro> macros;
    std::vector<mvShaderMacro> extramacros;
    mvVertexLayout             layout;

    i32 alphaMode = 0;
    b8 hasNormalMap = false;
    b8 hasEmmissiveMap = false;
    b8 hasOcculusionMap = false;
    b8 hasSpecularMap = false;
    b8 hasSpecularColorMap = false;
    b8 hasSpecularGlossinessMap = false;
    b8 hasSheenColorMap = false;
    b8 hasSheenRoughnessMap = false;
    b8 hasTransmissionMap = false;
    b8 hasThicknessMap = false;

    // pbr
    b8 pbrMetallicRoughness = false;
    b8 hasAlbedoMap = false;
    b8 hasMetallicRoughnessMap = false;

    // clearcoat extension
    b8 extensionClearcoat = false;
    b8 hasClearcoatMap = false;
    b8 hasClearcoatNormalMap = false;
    b8 hasClearcoatRoughnessMap = false;

};

struct mvMaterialAsset
{
    std::string hash;
    mvMaterial asset;
};

struct mvMaterialManager
{
    std::vector<mvMaterialAsset> materials;
};
