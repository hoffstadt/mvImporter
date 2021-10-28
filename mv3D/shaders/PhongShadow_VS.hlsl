#include "common/transform.hlsli"

struct VSOut
{
    float3 viewPos    : Position;
    float3 viewNormal : Normal;
    float2 tc         : Texcoord;
    float3 tan        : Tangent;
    float3 bitan      : Bitangent;
    float4 pos        : SV_Position;
};

VSOut main(float3 pos : Position, float3 n : Normal, float2 tc : Texcoord, float3 tan : Tangent, float3 bitan : Bitangent)
{
    VSOut vso;
    vso.viewPos = pos;
    vso.viewNormal = n;
    vso.tan = tan;
    vso.bitan = bitan;
    vso.pos = mul(modelViewProj, float4(pos, 1.0f));
    vso.tc = tc;
    return vso;
}