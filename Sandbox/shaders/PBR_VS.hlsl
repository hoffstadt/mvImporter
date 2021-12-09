
cbuffer TransformCBuf : register(b0)
{
    matrix model;
    matrix modelView;
    matrix modelViewProj;
};

struct VSOut
{   
    float4 Pos              : SV_Position;
    float3 WorldPos         : POSITION0;
    float3 Normal           : NORMAL0;
    float2 UV               : TEXCOORD0;
    float3 Tangent          : TEXCOORD1;
    float4 dshadowWorldPos  : dshadowPosition; // light pos
    float4 oshadowWorldPos  : oshadowPosition; // light pos
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

VSOut main(float3 pos : Position, float3 n : Normal, float2 tc : Texcoord, float3 tan : Tangent)
{
    VSOut output;
    float3 locPos = mul(model, float4(pos, 1.0)).xyz;
    output.WorldPos = locPos;
    output.Normal = mul((float3x3)model, n);
    output.Tangent = mul((float3x3) model, tan);
    output.UV = tc;
    output.Pos = mul(modelViewProj, float4(pos, 1.0f));
    output.dshadowWorldPos = ToDirectShadowHomoSpace(pos, model);
    output.oshadowWorldPos = ToShadowHomoSpace(pos, model);
    return output;
}