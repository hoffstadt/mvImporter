
cbuffer TransformCBuf : register(b0)
{
    matrix model;
    matrix modelView;
    matrix modelViewProj;
};

struct VSOut
{
    float2 tc               : Texcoord;        // texture coordinates (model space)
    float4 pixelPos         : SV_Position;     // pixel pos           (screen space)
};


VSOut main(float3 pos : Position, float3 n : Normal, float2 tc : Texcoord, float3 tan : Tangent)
{
    VSOut vso;
    vso.pixelPos = mul(modelViewProj, float4(pos, 1.0f));
    vso.tc = tc;
    return vso;
}