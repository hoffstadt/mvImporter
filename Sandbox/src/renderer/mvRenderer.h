#pragma once

#include "mvMath.h"
#include "mvMesh.h"
#include "mvMaterials.h"
#include "mvScene.h"

struct mvAssetManager;

struct GlobalInfo
{

    mvVec3 ambientColor = { 0.04f, 0.04f, 0.04f };
    b32    useShadows = true;
    //-------------------------- ( 16 bytes )

    b32  useOmniShadows = true;
    b32  useSkybox = false;
    b32  useAlbedo = true;
    b32  useMetalness = true;
    //-------------------------- ( 2*16 = 32 bytes )

    b32 useRoughness = true;
    b32 useIrradiance = true;
    b32 useReflection = true;
    b32 useEmissiveMap = true;
    //-------------------------- ( 2*16 = 32 bytes )

    mvMat4 projection;
    mvMat4 model;
    mvMat4 view;

    mvVec3 camPos;
    b32 useOcclusionMap = true;

    b32 useNormalMap = true;
    b32 usePCF = false;
    i32 pcfRange = 1;
    b32 usePunctualLights = true;
};

struct mvRenderJob
{
    mvMeshPrimitive* meshPrimitive = nullptr;
    mvMat4           accumulatedTransform = identity_mat4();
};

struct mvRendererContext
{
    mvRenderJob       opaqueJobs[1024];
    mvRenderJob       transparentJobs[1024];
    mvRenderJob       wireframeJobs[1024];
    u32               opaqueJobCount      = 0u;
    u32               transparentJobCount = 0u;
    u32               wireframeJobCount   = 0u;
    ID3D11BlendState* finalBlendState = nullptr;
};

namespace Renderer
{
    void mvSetupCommonAssets(mvAssetManager& am);

    void submit_scene(mvAssetManager& am, mvRendererContext& ctx, mvScene& scene, mvMat4 scaleM, mvMat4 trans);
    void render_jobs(mvAssetManager& am, mvRendererContext& ctx, mvMat4 cam, mvMat4 proj);

    void render_skybox(mvAssetManager& am, mvCubeTexture& cubemap, ID3D11SamplerState* sampler, mvMat4 cam, mvMat4 proj);
    void render_mesh(mvAssetManager& am, mvMesh& mesh, mvMat4 transform, mvMat4 cam, mvMat4 proj);
    void render_mesh_solid(mvAssetManager& am, mvMesh& mesh, mvMat4 transform, mvMat4 cam, mvMat4 proj);
    void render_mesh_wireframe(mvAssetManager& am, mvMesh& mesh, mvMat4 transform, mvMat4 cam, mvMat4 proj);
    void render_mesh_shadow(mvAssetManager& am, mvMesh& mesh, mvMat4 transform, mvMat4 cam, mvMat4 proj);

    void render_scene(mvAssetManager& am, mvScene& scene, mvMat4 cam, mvMat4 proj, mvMat4 scale = identity_mat4(), mvMat4 trans = identity_mat4());
    void render_scene_shadows(mvAssetManager& am, mvScene& scene, mvMat4 cam, mvMat4 proj, mvMat4 scale = identity_mat4(), mvMat4 trans = identity_mat4());

}