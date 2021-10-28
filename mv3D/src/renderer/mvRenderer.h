#pragma once

#include "mvMath.h"
#include "mvMesh.h"
#include "mvMaterials.h"
#include "mvPasses.h"
#include "mvScene.h"

struct mvAssetManager;

void mvRenderer_Resize();
void mvRenderer_StartRenderer();
void mvRenderer_StopRenderer();
void mvRenderer_BeginFrame();
void mvRenderer_EndFrame();
void mvRenderer_Present();
void mvRenderer_BeginPass(mvPass& pass);
void mvRenderer_ClearPass(mvPass& pass);
void mvRenderer_EndPass();
void mvRenderer_RenderMesh       (mvAssetManager& am, mvMesh& mesh, mvMat4 transform, mvMat4 cam, mvMat4 proj);
void mvRenderer_RenderMesh       (mvAssetManager& am, mvMesh& mesh, mvMat4 transform, mvMat4 cam, mvMat4 proj);
void mvRenderer_RenderMeshShadows(mvAssetManager& am, mvMesh& mesh, mvMat4 transform, mvMat4 cam, mvMat4 proj);
void mvRenderer_RenderSkybox     (mvAssetManager& am, mvSkyboxPass& skyboxPass, mvMat4 cam, mvMat4 proj);

void mvRenderer_RenderScene(mvAssetManager& am, mvScene& scene, mvMat4 cam, mvMat4 proj);
void mvRenderer_RenderSceneShadows(mvAssetManager& am, mvScene& scene, mvMat4 cam, mvMat4 proj);

