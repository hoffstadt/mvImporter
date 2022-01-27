
#include <simd/simd.h>

enum VertexAttributes {
    VertexAttributePosition = 0,
    VertexAttributeColor = 1,
};

enum BufferIndex
{
    MeshVertexBuffer = 0,
    FrameUniformBuffer = 1,
};

struct FrameUniforms {
    simd::float4x4 projectionViewModel;
};

using namespace metal;

struct VertexInput
{
    float3 position [[attribute(VertexAttributePosition)]];
    half4 color [[attribute(VertexAttributeColor)]];
};

struct ShaderInOut {
    float4 position [[position]];
    half4  color;
};


fragment half4 frag(ShaderInOut in [[stage_in]])
    {
    return in.color;
}
