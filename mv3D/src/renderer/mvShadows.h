#pragma once

#include "mvMath.h"
#include "mvBuffers.h"
#include "mvCamera.h"

struct mvShadowCameraInfo
{
    mvMat4 pointShadowView        = mvIdentityMat4();
    mvMat4 directShadowView       = mvIdentityMat4();
    mvMat4 directShadowProjection = mvIdentityMat4();
};

struct mvShadowCamera
{
    mvConstBuffer      buffer;
    mvShadowCameraInfo info;
};

struct mvShadowMap
{
    u32                                shadowMapDimension;
    mvComPtr<ID3D11Texture2D>          shadowMap;
    mvComPtr<ID3D11DepthStencilView>   shadowDepthView;
    mvComPtr<ID3D11ShaderResourceView> shadowResourceView;
    mvComPtr<ID3D11SamplerState>       comparisonSampler;
    mvComPtr<ID3D11RasterizerState>    shadowRasterizationState;
    mvOrthoCamera                      camera;
};

struct mvShadowCubeMap
{
    u32                                shadowMapDimension;
    mvComPtr<ID3D11Texture2D>          shadowMap;
    mvComPtr<ID3D11DepthStencilView>   shadowDepthViews[6];
    mvComPtr<ID3D11ShaderResourceView> shadowResourceView;
    mvComPtr<ID3D11SamplerState>       comparisonSampler;
    mvComPtr<ID3D11RasterizerState>    shadowRasterizationState;
    mvVec3                             cameraDirections[6];
    mvVec3                             cameraUps[6];
};

mvShadowMap     mvCreateShadowMap    (u32 resolution, f32 width);
mvShadowCubeMap mvCreateShadowCubeMap(u32 resolution);
mvShadowCamera  mvCreateShadowCamera ();
void            mvBindSlot_tsPS      (mvShadowMap& shadowMap, u32 textureSlot, u32 samplerSlot);
void            mvBindSlot_tsPS      (mvShadowCubeMap& shadowMap, u32 textureSlot, u32 samplerSlot);
void            mvBindSlot_bVS       (u32 slot, mvShadowCamera& camera, mvMat4 oview, mvMat4 dview, mvMat4 proj);

