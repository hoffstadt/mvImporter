#pragma once

#include <string>
#include "mvMath.h"
#include "mvBuffers.h"
#include "mvVertexLayout.h"

struct mvObjMesh;
struct mvAssetManager;
struct mvGLTFModel;

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
    mvAssetID      materialID = -1;
    mvAssetID      shadowMaterialID = -1;
};

struct mvMesh
{
    std::string                  name;
    std::vector<mvMeshPrimitive> primitives;
};

mvMesh mvCreateCube        (mvAssetManager& assetManager, f32 size = 1.0f);
mvMesh mvCreateTexturedCube(mvAssetManager& assetManager, f32 size = 1.0f);
mvMesh mvCreateTexturedQuad(mvAssetManager& assetManager, f32 size = 1.0f);
mvMesh mvCreateFrustum     (mvAssetManager& assetManager, f32 width, f32 height, f32 nearZ, f32 farZ);
mvMesh mvCreateFrustum2    (mvAssetManager& assetManager, f32 fov, f32 aspect, f32 nearZ, f32 farZ);

void mvLoadGLTFAssets(mvAssetManager& assetManager, mvGLTFModel& model);
