#pragma once

#include "mv3D_internal.h"
#include "mvShadows.h"
#include "mvTextures.h"
#include "mvPipeline.h"

struct mvAssetManager;

struct mvPass
{
    ID3D11RenderTargetView**   target = nullptr;
    ID3D11DepthStencilView**   depthStencil = nullptr;
    ID3D11ShaderResourceView** shaderResource = nullptr;
    D3D11_VIEWPORT             viewport;
    ID3D11RasterizerState**    rasterizationState = nullptr;
};

struct mvSkyboxPass
{
    mvPass     basePass{};
    mvPipeline pipeline;
    mvAssetID  mesh;
    mvAssetID  cubeTexture;
    mvAssetID  sampler;
};

mvPass       mvCreateMainPass();
mvPass       mvCreateShadowPass(mvShadowMap& shadowMap);
mvPass       mvCreateShadowPass(mvShadowCubeMap& shadowMap, u32 index);
mvSkyboxPass mvCreateSkyboxPass(mvAssetManager* assetManager, const std::string& path);