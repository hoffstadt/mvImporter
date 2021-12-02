
cbuffer TransformCBuf : register(b0)
{
    matrix model;
    matrix modelView;
    matrix modelViewProj;
};

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

cbuffer DirectionalShadowTransformCBuf : register(b1)
{
    matrix directShadowView;
    matrix directShadowProjection;
};

cbuffer OmniShadowTransformCBuf : register(b2)
{
    matrix pointShadowView;
};

float4 ToShadowHomoSpace(const in float3 pos, uniform matrix modelTransform)
{
    const float4 world = mul(modelTransform, float4(pos, 1.0f));
    return mul(pointShadowView, world);
}

float4 ToDirectShadowHomoSpace(const in float3 pos, uniform matrix modelTransform)
{
    float4x4 fin = mul(directShadowView, modelTransform);
    fin = mul(directShadowProjection, fin);
    return mul(fin, float4(pos, 1.0f));
}

VSOut main(float3 pos : Position, float3 n : Normal, float2 tc : Texcoord, float3 tan : Tangent, float3 bitan : Bitangent)
{
    VSOut vso;
    vso.viewPos = (float3) mul(modelView, float4(pos, 1.0f));
    vso.viewNormal = mul((float3x3) modelView, n);
    vso.worldNormal = mul((float3x3) model, n);
    vso.tangentBasis._m00_m10_m20 = mul((float3x3) modelView, tan);
    vso.tangentBasis._m01_m11_m21 = mul((float3x3) modelView, bitan);
    vso.tangentBasis._m02_m12_m22 = mul((float3x3) modelView, n);
    vso.pixelPos = mul(modelViewProj, float4(pos, 1.0f));
    vso.dshadowWorldPos = ToDirectShadowHomoSpace(pos, model);
    vso.oshadowWorldPos = ToShadowHomoSpace(pos, model);
    vso.tc = tc;
    return vso;
}