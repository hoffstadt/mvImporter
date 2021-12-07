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

void mvLoadGLTFAssets(mvAssetManager& assetManager, mvGLTFModel& model);
