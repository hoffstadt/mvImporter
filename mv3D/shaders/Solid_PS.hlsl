#include "common/equations.hlsli"
#include "common/structs.hlsli"

//-----------------------------------------------------------------------------
// textures
//-----------------------------------------------------------------------------
Texture2D ColorTexture : register(t0);

//-----------------------------------------------------------------------------
// samplers
//-----------------------------------------------------------------------------
SamplerState           Sampler        : register(s0);
SamplerComparisonState DShadowSampler : register(s1);
SamplerComparisonState OShadowSampler : register(s2);

//-----------------------------------------------------------------------------
// constant buffers
//-----------------------------------------------------------------------------
cbuffer mvPointLightCBuf       : register(b0){ mvPointLight PointLight;};
cbuffer mvMaterialCBuf         : register(b1){ mvPhongMaterial material;};
cbuffer mvDirectionalLightCBuf : register(b2){ mvDirectionalLight DirectionalLight;};
cbuffer mvSceneCBuf            : register(b3){ mvScene scene;};

struct VSOut
{
    float3 viewPos          : Position;        // pixel pos           (view space)
    float3 viewNormal       : Normal;          // pixel norm          (view space)
    float3 worldNormal      : WorldNormal;     // pixel normal        (view space)
    float2 tc               : Texcoord;        // texture coordinates (model space)
    float3x3 tangentBasis   : TangentBasis;    // tangent basis       (view space)
    float4 pixelPos         : SV_Position;     // pixel pos           (screen space)
    float4 dshadowWorldPos  : dshadowPosition; // light pos           (world space)
    float4 oshadowWorldPos  : oshadowPosition; // light pos           (world space)
};

float4 main(VSOut input) : SV_Target
{
    return material.materialColor;
}