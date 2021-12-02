
cbuffer TransformCBuf : register(b0)
{
    matrix modelView;
    matrix modelViewProj;
};

struct VSOut
{
    float3 viewPos          : Position;        // pixel pos           (view space)
    float3 viewNormal       : Normal;          // pixel norm          (view space)
    float4 pixelPos         : SV_Position;     // pixel pos           (screen space)
};


VSOut main(float3 pos : Position, float3 n : Normal)
{
    VSOut vso;
    vso.viewPos = (float3) mul(modelView, float4(pos, 1.0f));
    vso.viewNormal = mul((float3x3) modelView, n);
    vso.pixelPos = mul(modelViewProj, float4(pos, 1.0f));
    return vso;
}