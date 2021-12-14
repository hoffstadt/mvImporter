#pragma once

#include "mvMath.h"
#include "mvMesh.h"
#include "mvMaterials.h"
#include "mvScene.h"

struct mvAssetManager;

namespace Renderer
{
    void mvSetupCommonAssets(mvAssetManager& am);

    void mvRenderMesh(mvAssetManager& am, mvMesh& mesh, mvMat4 transform, mvMat4 cam, mvMat4 proj);
    void mvRenderMeshSolid(mvAssetManager& am, mvMesh& mesh, mvMat4 transform, mvMat4 cam, mvMat4 proj);
    void mvRenderMeshSolidWireframe(mvAssetManager& am, mvMesh& mesh, mvMat4 transform, mvMat4 cam, mvMat4 proj);
    void mvRenderMeshShadows(mvAssetManager& am, mvMesh& mesh, mvMat4 transform, mvMat4 cam, mvMat4 proj);

    void mvRenderScene(mvAssetManager& am, mvScene& scene, mvMat4 cam, mvMat4 proj, mvMat4 scale = mvIdentityMat4(), mvMat4 trans = mvIdentityMat4());
    void mvRenderSceneShadows(mvAssetManager& am, mvScene& scene, mvMat4 cam, mvMat4 proj, mvMat4 scale = mvIdentityMat4(), mvMat4 trans = mvIdentityMat4());

}