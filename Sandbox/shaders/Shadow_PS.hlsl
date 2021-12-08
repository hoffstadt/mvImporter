#include "equations.hlsli"


//-----------------------------------------------------------------------------
// textures
//-----------------------------------------------------------------------------
Texture2D AlbedoTexture : register(t0);

//-----------------------------------------------------------------------------
// samplers
//-----------------------------------------------------------------------------
SamplerState Sampler : register(s0);


struct VSOut
{
    float2 tc               : Texcoord;        // texture coordinates (model space)
    float4 pixelPos         : SV_Position; // pixel pos           (screen space)
};

void main(VSOut input)
{
    
        float4 albedo = AlbedoTexture.Sample(Sampler, input.tc).rgba;
        
        // bail if highly translucent
        clip(albedo.a < 0.1f ? -1 : 1);
    
}