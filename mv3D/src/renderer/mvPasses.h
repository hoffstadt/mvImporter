#pragma once

#include "mv3D_internal.h"
#include "mvShadows.h"
#include "mvTextures.h"
#include "mvPipeline.h"

struct mvAssetManager;

struct mvPass
{
    ID3D11RenderTargetView**   target;
    ID3D11DepthStencilView**   depthStencil;
    ID3D11ShaderResourceView** shaderResource;
    D3D11_VIEWPORT             viewport;
    ID3D11RasterizerState**    rasterizationState;
};

struct mvSkyboxPass
{
    mvPass     basePass;
    mvPipeline pipeline;
    mvAssetID  mesh;
    mvAssetID  cubeTexture;
    mvAssetID  sampler;
};

mvPass       mvCreateMainPass();
mvPass       mvCreateShadowPass(mvShadowMap& shadowMap);
mvPass       mvCreateShadowPass(mvShadowCubeMap& shadowMap, u32 index);
mvSkyboxPass mvCreateSkyboxPass(mvAssetManager* assetManager, const std::string& path);