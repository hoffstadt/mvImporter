#pragma once

#include <Metal/Metal.hpp>
#include <simd/simd.h>

// For pipeline executing.
constexpr int uniformBufferCount = 3;

// forward declarations
struct mvGraphics;

mvGraphics* setup_graphics(MTL::Device* device, unsigned int sampleCount);

struct mvGraphics
{
    MTL::Device*              device = nullptr;
    MTL::CommandQueue*        commandQueue = nullptr;
    MTL::DepthStencilState*   depthState = nullptr;
    dispatch_semaphore_t      semaphore = nullptr;
    MTL::Buffer*              uniformBuffers[uniformBufferCount] = {};
    MTL::Buffer*              vertexBuffer = nullptr;
    int                       uniformBufferIndex = 0;
    long                      frame = 0;
};

enum VertexAttributes {
    VertexAttributePosition = 0,
    VertexAttributeColor = 1,
};

enum BufferIndex  {
    MeshVertexBuffer = 0,
    FrameUniformBuffer = 1,
};

// Vertex structure on CPU memory.
struct Vertex
{
    float position[3];
    unsigned char color[4];
};

struct FrameUniforms
{
    simd::float4x4 projectionViewModel;
};