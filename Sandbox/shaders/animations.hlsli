
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
    float2 tc0 : Tex0Coord;
#endif

#ifdef HAS_TEXCOORD_1_VEC2
    float2 tc1 : Tex1Coord;
#endif

#ifdef HAS_VERTEX_COLOR_VEC3
    float3 a_color : Color;
#endif

#ifdef HAS_VERTEX_COLOR_VEC4
    float4 a_color : Color;
#endif

#ifdef HAS_JOINTS_0_VEC4
    float4 a_joints_0 : Joints0;
#endif

#ifdef HAS_JOINTS_1_VEC4
    float4 a_joints_1 : Joints1;
#endif

#ifdef HAS_WEIGHTS_0_VEC4
    float4 a_weights_0 : Weights0;
#endif

#ifdef HAS_WEIGHTS_1_VEC4
    float4 a_weights_1 : Weights1;
#endif
};

#ifdef USE_SKINNING
Texture2D JointsTexture : register(t0);
SamplerState JointsTextureSampler : register(s0);
#endif

#ifdef USE_SKINNING

float4x4 getMatrixFromTexture(int index)
{
    float4x4 result = float4x4( 
    1,0,0,0, 
    0,1,0,0, 
    0,0,1,0, 
    0,0,0,1 );

    int texWidth = 0;
    int texHeight = 0;
    JointsTexture.GetDimensions(texWidth, texHeight);
    int pixelIndex = index * 4;
    [unroll]
    for (int i = 0; i < 4; ++i)
    {
        int x = (pixelIndex + i) % texWidth;
        //Rounding mode of integers is undefined:
        //https://www.khronos.org/registry/OpenGL/specs/es/3.0/GLSL_ES_Specification_3.00.pdf (section 12.33)
        int y = (pixelIndex + i - x) / texWidth;
        //result[i] = JointsTexture.Load(float3(x, y, 0));
        //float4 value = JointsTexture.SampleLevel(JointsTextureSampler, int2(x, y), 0);
        float4 value = JointsTexture.Load(int3(x, y, 0));
        result[i][0] = value.x;
        result[i][1] = value.y;
        result[i][2] = value.z;
        result[i][3] = value.w;
    }
    return transpose(result);
    //return result;
}

float4x4 getSkinningMatrix(VSIn input)
{
    float4x4 skin = float4x4(
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0);

#if defined(HAS_WEIGHTS_0_VEC4) && defined(HAS_JOINTS_0_VEC4)
    skin +=
        mul(input.a_weights_0.x, getMatrixFromTexture(int(input.a_joints_0.x) * 2)) +
        mul(input.a_weights_0.y, getMatrixFromTexture(int(input.a_joints_0.y) * 2)) +
        mul(input.a_weights_0.z, getMatrixFromTexture(int(input.a_joints_0.z) * 2)) +
        mul(input.a_weights_0.w, getMatrixFromTexture(int(input.a_joints_0.w) * 2));

#endif

#if defined(HAS_WEIGHTS_1_VEC4) && defined(HAS_JOINTS_1_VEC4)
    skin +=
        mul(input.a_weights_1.x, getMatrixFromTexture(int(input.a_joints_1.x) * 2)) +
        mul(input.a_weights_1.y, getMatrixFromTexture(int(input.a_joints_1.y) * 2)) +
        mul(input.a_weights_1.z, getMatrixFromTexture(int(input.a_joints_1.z) * 2)) +
        mul(input.a_weights_1.w, getMatrixFromTexture(int(input.a_joints_1.w) * 2));
#endif

    return skin;
}

float4x4 getSkinningNormalMatrix(VSIn input)
{
    float4x4 skin = float4x4(
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0);

#if defined(HAS_WEIGHTS_0_VEC4) && defined(HAS_JOINTS_0_VEC4)
    skin +=
        mul(input.a_weights_0.x, getMatrixFromTexture(int(input.a_joints_0.x) * 2 + 1)) +
        mul(input.a_weights_0.y, getMatrixFromTexture(int(input.a_joints_0.y) * 2 + 1)) +
        mul(input.a_weights_0.z, getMatrixFromTexture(int(input.a_joints_0.z) * 2 + 1)) +
        mul(input.a_weights_0.w, getMatrixFromTexture(int(input.a_joints_0.w) * 2 + 1));
#endif

#if defined(HAS_WEIGHTS_1_VEC4) && defined(HAS_JOINTS_1_VEC4)
    skin +=
        mul(input.a_weights_1.x, getMatrixFromTexture(int(input.a_joints_1.x) * 2 + 1)) +
        mul(input.a_weights_1.y, getMatrixFromTexture(int(input.a_joints_1.y) * 2 + 1)) +
        mul(input.a_weights_1.z, getMatrixFromTexture(int(input.a_joints_1.z) * 2 + 1)) +
        mul(input.a_weights_1.w, getMatrixFromTexture(int(input.a_joints_1.w) * 2 + 1));
#endif

    return skin;
}

#endif // !USE_SKINNING