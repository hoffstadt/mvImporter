#pragma once

#include <string>
#include "mvMath.h"
#include "mvBuffers.h"
#include "mvPipeline.h"
#include "mvTextures.h"

// forward declarations
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
    mvTexture      morphTexture;
    f32*           morphData = nullptr;
};

struct mvMesh
{
    std::string                  name;
    std::vector<mvMeshPrimitive> primitives;
    std::vector<f32>             weights;
    std::vector<f32>             weightsAnimated;
    u32                          weightCount;
    mvConstBuffer                morphBuffer;
};
