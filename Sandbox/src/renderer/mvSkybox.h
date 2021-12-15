#pragma once

#include "mvTypes.h"
#include "mvMath.h"
#include "mvPipeline.h"
#include "mvBuffers.h"
#include "mvTextures.h"

struct mvAssetManager;

struct mvSkybox
{
    ID3D11SamplerState* cubeSampler = nullptr;
    mvBuffer            vertexBuffer;
    mvBuffer            indexBuffer;
    mvVertexLayout      vertexLayout;
    mvCubeTexture       cubeTexture;
    mvPipeline          pipeline;
};

mvSkybox create_skybox(mvAssetManager& am);
void     render_skybox(mvSkybox& skybox, mvMat4 cam, mvMat4 proj);