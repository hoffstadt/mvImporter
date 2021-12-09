TextureCube tex : register(t0);
SamplerState sam : register(s0);

float4 main(float3 worldPos : Position) : SV_TARGET
{
    worldPos.x = -worldPos.x;
    worldPos.z = -worldPos.z;
    return tex.Sample(sam, worldPos);
}