
#include "animations.hlsli"

cbuffer TransformCBuf : register(b0)
{
    matrix model;
    matrix modelView;
    matrix modelViewProj;
};

struct VSOut
{
    float2 tc               : Tex0Coord;        // texture coordinates (model space)
    float4 pixelPos         : SV_Position;     // pixel pos           (screen space)
};

float4 getPosition(VSIn input)
{
    float4 pos = float4(input.pos, 1.0);

#ifdef USE_MORPHING
    pos += getTargetPosition(input.vid);
#endif

#ifdef USE_SKINNING
    pos = mul(getSkinningMatrix(input), pos);
#endif

    return pos;
}

VSOut main(VSIn input)
{
    VSOut vso;
    vso.pixelPos = mul(modelViewProj, getPosition(input));
    vso.tc = float2(0.0, 0.0);
    #ifdef HAS_TEXCOORD_0_VEC2
        vso.tc =input.tc0;
    #endif
    return vso;
}