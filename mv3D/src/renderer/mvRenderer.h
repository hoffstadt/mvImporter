#pragma once

#include "mvMath.h"
#include "mvMesh.h"
#include "mvMaterials.h"
#include "mvPasses.h"
#include "mvScene.h"

struct mvAssetManager;

namespace Renderer
{

    void mvResize();
    void mvStartRenderer();
    void mvStopRenderer();
    void mvBeginFrame();
    void mvEndFrame();
    void mvPresent();
    void mvBeginPass(mvPass& pass);
    void mvClearPass(mvPass& pass);
    void mvEndPass();
    void mvRenderMesh(mvAssetManager& am, mvMesh& mesh, mvMat4 transform, mvMat4 cam, mvMat4 proj);
    void mvRenderMesh(mvAssetManager& am, mvMesh& mesh, mvMat4 transform, mvMat4 cam, mvMat4 proj);
    void mvRenderMeshShadows(mvAssetManager& am, mvMesh& mesh, mvMat4 transform, mvMat4 cam, mvMat4 proj);
    void mvRenderSkybox(mvAssetManager& am, mvSkyboxPass& skyboxPass, mvMat4 cam, mvMat4 proj);

    void mvRenderScene(mvAssetManager& am, mvScene& scene, mvMat4 cam, mvMat4 proj);
    void mvRenderSceneShadows(mvAssetManager& am, mvScene& scene, mvMat4 cam, mvMat4 proj);

}