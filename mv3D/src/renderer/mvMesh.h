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
    mvVec3         pos                   = { 0.0f, 0.0f, 0.0f};
    mvVec3         rot                   = { 0.0f, 0.0f, 0.0f };
    mvVec3         scale                 = { 1.0f, 1.0f, 1.0f };
    s32            indexBuffer           = -1;
    s32            vertexBuffer          = -1;
    s32            diffuseTexture        = -1;
    s32            normalTexture         = -1;
    s32            specularTexture       = -1;
    s32            albedoTexture         = -1;
    s32            metalRoughnessTexture = -1;
    s32            phongMaterialID       = -1;
    s32            pbrMaterialID         = -1;
    s32            shadowMaterialID      = -1;
    b8             pbr                   = false;
};

mvMesh mvCreateTexturedCube(mvAssetManager& assetManager, f32 size = 1.0f);
mvMesh mvCreateTexturedQuad(mvAssetManager& assetManager, f32 size = 1.0f);

void mvLoadOBJAssets(mvAssetManager& assetManager, const std::string& root, const std::string& file);
void mvLoadGLTFAssets(mvAssetManager& assetManager, mvGLTFModel& model);
