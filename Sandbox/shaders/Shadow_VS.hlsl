
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

struct VSIn
{
    float3 pos : Position;
#ifdef HAS_NORMALS
    float3 n: Normal;
#endif

#ifdef HAS_TANGENTS
    float4 tan: Tangent;
#endif

#ifdef HAS_TEXCOORD_0_VEC2
    float2 tc : Texcoord;
#endif

#ifdef HAS_TEXCOORD_1_VEC2
    float2 tc : Texcoord;
#endif

#ifdef HAS_VERTEX_COLOR_VEC3
    float3 a_color : Color;
#endif

#ifdef HAS_VERTEX_COLOR_VEC4
    float4 a_color : Color;
#endif
};


VSOut main(VSIn input)
{
    VSOut vso;
    vso.pixelPos = mul(modelViewProj, float4(input.pos, 1.0f));
    vso.tc = float2(0.0, 0.0);
    #ifdef HAS_TEXCOORD_0_VEC2
        vso.tc =input.tc;
    #endif
    return vso;
}