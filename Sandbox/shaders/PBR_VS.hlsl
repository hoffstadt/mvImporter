
cbuffer TransformCBuf : register(b0)
{
    matrix model;
    matrix modelView;
    matrix modelViewProj;
    matrix normalMatrix;
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

#ifdef HAS_UV_SET1
    float2 tc : Texcoord;
#endif

#ifdef HAS_UV_SET2
    float2 tc : Texcoord;
#endif

#ifdef HAS_VERTEX_COLOR_VEC3
    float3 a_color : Color;
#endif

#ifdef HAS_VERTEX_COLOR_VEC4
    float4 a_color : Color;
#endif
    float3 b : Bitangent;
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

#ifdef HAS_VERTEX_COLOR_VEC4
    float4 v_color : COLOR0;
#endif
    float3 WorldPos : POSITION0;
    float3 WorldNormal : NORMAL1;
    float2 UV0 : TEXCOORD0;
    float2 UV1 : TEXCOORD1;
    float4 dshadowWorldPos : dshadowPosition; // light pos
    float4 oshadowWorldPos : oshadowPosition; // light pos

};

cbuffer DirectionalShadowTransformCBuf : register(b1)
{
    matrix directShadowView;
    matrix directShadowProjection;
};

cbuffer OmniShadowTransformCBuf : register(b2)
{
    matrix pointShadowView;
};

float4 ToShadowHomoSpace(const in float3 pos, uniform matrix modelTransform)
{
    const float4 world = mul(modelTransform, float4(pos, 1.0f));
    return mul(pointShadowView, world);
}

float4 ToDirectShadowHomoSpace(const in float3 pos, uniform matrix modelTransform)
{
    float4x4 fin = mul(directShadowView, modelTransform);
    fin = mul(directShadowProjection, fin);
    return mul(fin, float4(pos, 1.0f));
}

float4 getPosition(VSIn input)
{
    float4 pos = float4(input.pos, 1.0);

#ifdef USE_MORPHING
    pos += getTargetPosition();
#endif

#ifdef USE_SKINNING
    pos = getSkinningMatrix() * pos;
#endif

    return pos;
}

#ifdef HAS_NORMALS
float3 getNormal(VSIn input)
{
    float3 normal = input.n;

#ifdef USE_MORPHING
    normal += getTargetNormal();
#endif

#ifdef USE_SKINNING
    normal = mat3(getSkinningNormalMatrix()) * normal;
#endif

    return normalize(normal);
}
#endif

#ifdef HAS_TANGENTS
float3 getTangent(VSIn input)
{
    float3 tangent = input.tan.xyz;

#ifdef USE_MORPHING
    tangent += getTargetTangent();
#endif

#ifdef USE_SKINNING
    tangent = mat3(getSkinningMatrix()) * tangent;
#endif

    return normalize(tangent);
}
#endif

VSOut main(VSIn input)
{
    VSOut output;
    float3 locPos = mul(model, getPosition(input)).xyz;
    output.WorldPos = locPos;
#ifdef HAS_NORMALS
    output.WorldNormal = mul((float3x3)model, getNormal(input));
#endif
    float3 WorldTangent = mul((float3x3) model, input.tan.xyz);
    float3 WorldBitangent = mul((float3x3) model, input.b);
    output.TBN = transpose(float3x3(WorldTangent, WorldBitangent, output.WorldNormal));
    
    output.UV0 = float2(0.0, 0.0);
    output.UV1 = float2(0.0, 0.0);

#ifdef HAS_UV_SET1
        output.UV0 =input.tc;
    #endif

    output.Pos = mul(modelViewProj, float4(input.pos, 1.0f));
    output.dshadowWorldPos = ToDirectShadowHomoSpace(getPosition(input).xyz, model);
    output.oshadowWorldPos = ToShadowHomoSpace(getPosition(input).xyz, model);


    #ifdef HAS_NORMALS
    #ifdef HAS_TANGENTS
    float3 tangent = getTangent(input);
    float3 normalW = normalize(float4(mul(normalMatrix, float4(getNormal(input), 0.0))));
    float3 tangentW = normalize(float3(mul(model, float4(tangent, 0.0)).xyz));
    float3 bitangentW = cross(normalW, tangentW) * input.tan.w;
    //output.TBN = transpose(float3x3(tangentW, bitangentW, normalW));
    #else // !HAS_TANGENTS
        output.v_Normal = normalize(vec3(u_NormalMatrix * vec4(getNormal(), 0.0)));
    #endif
    #endif // !HAS_NORMALS

    #if defined(HAS_VERTEX_COLOR_VEC3) || defined(HAS_VERTEX_COLOR_VEC4)
        output.v_Color = input.a_Color;
    #endif

    return output;
}