#pragma once

#include "mvMath.h"
#include "mvMesh.h"
#include "mvMaterials.h"
#include "mvScene.h"

// forward declarations
struct mvModel;
struct mvRenderJob;
struct mvRendererContext;
struct mvCamera;
struct mvPipeline;

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
    mvMeshPrimitive*       meshPrimitive = nullptr;
    mvMat4                 accumulatedTransform = identity_mat4();
    mvSkin*                skin = nullptr;
    mvComPtr<ID3D11Buffer> morphBuffer = nullptr;
};

struct mvRendererContext
{
    GlobalInfo               globalInfo{};
    mvConstBuffer            globalInfoBuffer;
    mvCamera*                camera = nullptr;
    std::vector<mvRenderJob> opaqueJobs;
    std::vector<mvRenderJob> transparentJobs;
    std::vector<mvRenderJob> wireframeJobs;
    ID3D11BlendState*        finalBlendState = nullptr;
    mvPipeline               skyboxPipeline;
    mvPipeline               solidPipeline;
    mvPipeline               solidWireframePipeline;
};

namespace Renderer
{
    mvRendererContext create_renderer_context();

    void submit_scene(mvModel& model, mvRendererContext& ctx, mvScene& scene);
    void render_scenes(mvModel& model, mvRendererContext& ctx, mvMat4 cam, mvMat4 proj);

    void render_skybox(mvRendererContext& rendererCtx, mvModel& model, mvCubeTexture& cubemap, ID3D11SamplerState* sampler, mvMat4 cam, mvMat4 proj);
    void render_mesh_solid(mvRendererContext& rendererCtx, mvModel& model, mvMesh& mesh, mvMat4 transform, mvMat4 cam, mvMat4 proj);

}