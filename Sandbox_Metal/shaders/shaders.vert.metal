
#include <simd/simd.h>

enum VertexAttributes {
    VertexAttributePosition = 0,
    VertexAttributeColor = 1,
};

enum BufferIndex  {
    MeshVertexBuffer = 0,
    FrameUniformBuffer = 1,
};

struct FrameUniforms {
    simd::float4x4 projectionViewModel;
};

using namespace metal;

struct VertexInput {
    float3 position [[attribute(VertexAttributePosition)]];
    half4 color [[attribute(VertexAttributeColor)]];
};

struct ShaderInOut {
    float4 position [[position]];
    half4  color;
};

vertex ShaderInOut vert(VertexInput in [[stage_in]], constant FrameUniforms& uniforms [[buffer(FrameUniformBuffer)]])
    {
    ShaderInOut out;
    float4 pos4 = float4(in.position, 1.0);
    out.position = uniforms.projectionViewModel * pos4;
    out.color = in.color / 255.0;
    return out;
}
