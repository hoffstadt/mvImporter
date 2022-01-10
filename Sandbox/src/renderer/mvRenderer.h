#pragma once

#include "mvMath.h"
#include "mvMesh.h"
#include "mvMaterials.h"
#include "mvScene.h"

// forward declarations
struct mvAssetManager;
struct mvRenderJob;
struct mvRendererContext;
struct mvCamera;

struct GlobalInfo
{

    mvVec3 ambientColor = { 0.04f, 0.04f, 0.04f };
    i32 pcfRange = 0;
    //-------------------------- ( 16 bytes )

    mvVec3 camPos;
    char _padding[4];
    //-------------------------- ( 16 bytes )
    
    mvMat4 projection;
    mvMat4 model;
    mvMat4 view;
    //-------------------------- ( 192 bytes )

    //-------------------------- ( 124 bytes )
};

struct mvRenderJob
{
    mvMeshPrimitive* meshPrimitive = nullptr;
    mvMat4           accumulatedTransform = identity_mat4();
    mvSkin*          skin = nullptr;
    ID3D11Buffer*    morphBuffer = nullptr;
};

struct mvRendererContext
{
    GlobalInfo        globalInfo{};
    mvConstBuffer     globalInfoBuffer;
    mvCamera*         camera = nullptr;
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
    mvRendererContext create_renderer_context(mvAssetManager& am);
    void setup_common_assets(mvAssetManager& am);

    void submit_scene(mvAssetManager& am, mvRendererContext& ctx, mvScene& scene, mvMat4 scaleM, mvMat4 trans);
    void render_scenes(mvAssetManager& am, mvRendererContext& ctx, mvMat4 cam, mvMat4 proj);

    void render_skybox(mvAssetManager& am, mvCubeTexture& cubemap, ID3D11SamplerState* sampler, mvMat4 cam, mvMat4 proj);
    void render_mesh_solid(mvAssetManager& am, mvMesh& mesh, mvMat4 transform, mvMat4 cam, mvMat4 proj);
    void render_mesh_shadow(mvAssetManager& am, mvMesh& mesh, mvMat4 transform, mvMat4 cam, mvMat4 proj);

    void render_scene_shadows(mvAssetManager& am, mvScene& scene, mvMat4 cam, mvMat4 proj, mvMat4 scale = identity_mat4(), mvMat4 trans = identity_mat4());

}