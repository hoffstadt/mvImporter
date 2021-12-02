
cbuffer TransformCBuf : register(b0)
{
    matrix modelView;
    matrix modelViewProj;
};

float4 main(float3 pos : Position) : SV_Position
{
    return mul(modelViewProj, float4(pos, 1.0f));
}