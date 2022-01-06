#pragma once

#include <string>
#include "mvMath.h"
#include "mvBuffers.h"
#include "mvVertexLayout.h"
#include "mvTextures.h"

struct mvObjMesh;
struct mvAssetManager;
struct mvGLTFModel;
struct mvMeshPrimitive;
struct mvMesh;
struct mvSkin;
struct mvNode;

mvMesh    create_cube         (mvAssetManager& assetManager, f32 size = 1.0f);
mvMesh    create_textured_cube(mvAssetManager& assetManager, f32 size = 1.0f);
mvMesh    create_textured_quad(mvAssetManager& assetManager, f32 size = 1.0f);
mvMesh    create_frustum      (mvAssetManager& assetManager, f32 width, f32 height, f32 nearZ, f32 farZ);
mvMesh    create_frustum2     (mvAssetManager& assetManager, f32 fov, f32 aspect, f32 nearZ, f32 farZ);
mvMesh    create_ortho_frustum(mvAssetManager& assetManager, f32 width, f32 height, f32 nearZ, f32 farZ);
mvAssetID load_gltf_assets    (mvAssetManager& assetManager, mvGLTFModel& model);

void compute_joints(mvAssetManager& am, mvNode& parentNode, mvSkin& skin);

struct mvSkin
{
    u32       skeleton = 0u;
    mvAssetID joints[256];
    u32       jointCount = 0u;
    mvTexture jointTexture;
    std::vector<f32> inverseBindMatrices;
    f32* textureData = nullptr;
};

struct mvMeshPrimitive
{
    mvVertexLayout layout;
    mvAssetID      indexBuffer = -1;
    mvAssetID      vertexBuffer = -1;
    mvAssetID      normalTexture = -1;
    mvAssetID      specularTexture = -1;
    mvAssetID      albedoTexture = -1;
    mvAssetID      emissiveTexture = -1;
    mvAssetID      occlusionTexture = -1;
    mvAssetID      metalRoughnessTexture = -1;
    mvAssetID      clearcoatTexture = -1;
    mvAssetID      clearcoatRoughnessTexture = -1;
    mvAssetID      clearcoatNormalTexture = -1;
    mvAssetID      materialID = -1;
    mvAssetID      shadowMaterialID = -1;
};

struct mvMesh
{
    std::string                  name;
    std::vector<mvMeshPrimitive> primitives;
};


