#pragma once

#include <string>
#include "mvMath.h"
#include "mvBuffers.h"
#include "mvVertexLayout.h"

struct mvObjMesh;
struct mvAssetManager;
struct mvGLTFModel;

struct mvMesh
{
    std::string    name;
    mvVertexLayout layout;
    mvAssetID      indexBuffer           = -1;
    mvAssetID      vertexBuffer          = -1;
    mvAssetID      normalTexture         = -1;
    mvAssetID      specularTexture       = -1;
    mvAssetID      albedoTexture         = -1;
    mvAssetID      metalRoughnessTexture = -1;
    mvAssetID      materialID            = -1;
    mvAssetID      shadowMaterialID      = -1;
};

mvMesh mvCreateCube        (mvAssetManager& assetManager, f32 size = 1.0f);
mvMesh mvCreateTexturedCube(mvAssetManager& assetManager, f32 size = 1.0f);
mvMesh mvCreateTexturedQuad(mvAssetManager& assetManager, f32 size = 1.0f);
mvMesh mvCreateRoom        (mvAssetManager& assetManager, f32 size = 1.0f);

void mvLoadGLTFAssets(mvAssetManager& assetManager, mvGLTFModel& model);
