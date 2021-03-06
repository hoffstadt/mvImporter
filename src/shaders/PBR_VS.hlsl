#include "animations.hlsli"


cbuffer TransformCBuf : register(b0)
{
    matrix model;
    matrix modelView;
    matrix modelViewProj;
};

struct VSOut
{

    float4 Pos : SV_Position;

#ifdef HAS_NORMALS
#ifdef HAS_TANGENTS
    float3x3 TBN : TangentBasis;
#else
    float3 v_Normal: NORMAL0;
#endif
#endif

#ifdef HAS_VERTEX_COLOR_VEC3
    float3 v_color : COLOR0;
#endif

#ifdef HAS_VERTEX_COLOR_0_VEC4
    float4 v_color0 : COLOR0;
#endif

#ifdef HAS_VERTEX_COLOR_1_VEC4
    float4 v_color1 : COLOR1;
#endif

    float3 WorldPos : POSITION0;
    float3 WorldNormal : NORMAL1;
    float2 UV0 : TEXCOORD0;
    float2 UV1 : TEXCOORD1;

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

#ifdef HAS_NORMALS
float3 getNormal(VSIn input)
{
    float3 normal = input.n;

#ifdef USE_MORPHING
    normal += getTargetNormal(input.vid);
#endif

#ifdef USE_SKINNING
    normal = mul((float3x3)getSkinningNormalMatrix(input), normal);
#endif

    return normalize(normal);
}
#endif

#ifdef HAS_TANGENTS
float3 getTangent(VSIn input)
{
    float3 tangent = input.tan.xyz;

#ifdef USE_MORPHING
    tangent += getTargetTangent(input.vid);
#endif

#ifdef USE_SKINNING
    tangent = mul((float3x3)getSkinningMatrix(input), tangent);
#endif

    return normalize(tangent);
}
#endif

VSOut main(VSIn input)
{
    VSOut output;
    float3 locPos = mul(model, getPosition(input)).xyz;
    output.WorldPos = locPos;

    output.UV0 = float2(0.0, 0.0);
    output.UV1 = float2(0.0, 0.0);

    #ifdef HAS_TEXCOORD_0_VEC2
        output.UV0 =input.tc0;
    #endif

    #ifdef HAS_TEXCOORD_1_VEC2
        output.UV1 =input.tc1;
    #endif

    #ifdef USE_MORPHING
    output.UV0  += getTargetTexCoord0(input.vid);
    output.UV1  += getTargetTexCoord1(input.vid);
#endif

    output.Pos = mul(modelViewProj, getPosition(input));

    #ifdef HAS_NORMALS
        output.WorldNormal = mul((float3x3)model, getNormal(input));
    #ifdef HAS_TANGENTS
        float3 tangent = getTangent(input);
        float3 WorldTangent = mul((float3x3) model, tangent);
        float3 WorldBitangent = cross(getNormal(input), tangent)*input.tan.w;
        WorldBitangent = mul((float3x3) model, WorldBitangent);
        output.TBN = transpose(float3x3(WorldTangent, WorldBitangent, output.WorldNormal));
    #else // !HAS_TANGENTS
        output.v_Normal = normalize(float3(mul(normalMatrix, float4(getNormal(input), 0.0)).xyz));
    #endif
    #endif // !HAS_NORMALS

    #if defined(HAS_VERTEX_COLOR_0_VEC3) || defined(HAS_VERTEX_COLOR_0_VEC4)
        output.v_color0 = input.a_color0;
    #endif

    #if defined(HAS_VERTEX_COLOR_1_VEC3) || defined(HAS_VERTEX_COLOR_1_VEC4)
        output.v_color1 = input.a_color1;
    #endif

    return output;
}