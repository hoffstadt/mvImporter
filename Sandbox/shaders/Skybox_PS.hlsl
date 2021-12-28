TextureCube tex : register(t0);
SamplerState sam : register(s0);

float4 main(float3 worldPos : Position) : SV_TARGET
{
    worldPos.x = -worldPos.x;
    worldPos.z = -worldPos.z;
    float4 color =  tex.SampleLevel(sam, worldPos, 0.0f);
    
    // Gamma correct
    color = pow(color, float4(0.4545, 0.4545, 0.4545, 1.0));
    
    return color;
}
