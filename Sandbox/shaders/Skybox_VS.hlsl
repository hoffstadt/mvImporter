#include "common/structs.hlsli"
#include "common/transform.hlsli"

struct VSOut
{
    float3 worldPos : Position;
    float4 pos : SV_Position;
};

VSOut main(float3 pos : Position)
{
    VSOut vso;
    vso.worldPos = pos;
    vso.pos = mul(modelViewProj, float4(pos, 0.0f));
    // make sure that the depth after w divide will be 1.0 (so that the z-buffering will work)
    vso.pos.z = vso.pos.w;
    return vso;
}