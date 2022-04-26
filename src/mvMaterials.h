#pragma once

#include <string>
#include "sMath.h"
#include "mvGraphics.h"

// forward declarations
struct mvMaterial;
struct mvMaterialData;
struct mvVertexLayout;
struct mvMaterialAsset;
struct mvMaterialManager;

mvMaterial  create_material(mvGraphics& graphics, const std::string& vs, const std::string& ps, mvMaterial materialInfo);
std::string hash_material (const mvMaterial& materialInfo, const mvVertexLayout& layout, const std::string& pixelShader, const std::string& vertexShader);
mvAssetID   register_asset(mvMaterialManager* manager, const std::string& tag, mvMaterial asset);
void        clear_materials(mvMaterialManager* manager);
void        reload_materials(mvGraphics& graphics, mvMaterialManager* manager);
mvAssetID   mvGetMaterialAssetID(mvMaterialManager* manager, const std::string& tag);
mvMaterial* mvGetRawMaterialAsset(mvMaterialManager* manager, const std::string& tag);

struct mvMaterialData
{
    sVec4 albedo = { 0.45f, 0.45f, 0.85f, 1.0f };
    //-------------------------- ( 16 bytes )

    float metalness = 1.0f;
    float roughness = 0.1f;
    float radiance  = 1.0f;
    float fresnel   = 0.04f;
    //-------------------------- ( 16 bytes )

    sVec3 emisiveFactor = { 0.0f, 0.0f, 0.0f };
    float occlusionStrength = 1.0f;
    //-------------------------- ( 16 bytes )

    int doubleSided = false;
    float alphaCutoff;
    float clearcoatFactor = 0.0;
    float clearcoatRoughnessFactor = 0.0;
    //-------------------------- ( 16 bytes )

    float normalScale              = 1.0f;
    float clearcoatNormalScale     = 1.0f;
    float transmissionFactor       = 1.0f;
    float thicknessFactor          = 1.0f;
    //-------------------------- ( 16 bytes )

    sVec4 attenuationColor = { 0.45f, 0.45f, 0.85f, 1.0f };
    //-------------------------- ( 16 bytes )

    sVec3 sheenColorFactor = { 0.0f, 0.0f, 0.0f };
    float sheenRoughnessFactor = 1.0f;
    //-------------------------- ( 16 bytes )

    float attenuationDistance = 1.0f;
    float ior = 1.0f;
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

    int alphaMode = 0;
    bool hasNormalMap = false;
    bool hasEmmissiveMap = false;
    bool hasOcculusionMap = false;
    bool hasSpecularMap = false;
    bool hasSpecularColorMap = false;
    bool hasSpecularGlossinessMap = false;
    bool hasSheenColorMap = false;
    bool hasSheenRoughnessMap = false;
    bool hasTransmissionMap = false;
    bool hasThicknessMap = false;

    // pbr
    bool pbrMetallicRoughness = false;
    bool hasAlbedoMap = false;
    bool hasMetallicRoughnessMap = false;

    // clearcoat extension
    bool extensionClearcoat = false;
    bool hasClearcoatMap = false;
    bool hasClearcoatNormalMap = false;
    bool hasClearcoatRoughnessMap = false;

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
