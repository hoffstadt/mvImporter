#pragma once

#include <Metal/Metal.hpp>
#include <string>

// forward declarations
enum class mvVertexElement;
struct mvPipeline;
struct mvPipelineSpec;
struct mvGraphics;

mvPipeline finalize_pipeline(mvGraphics graphics, mvPipelineSpec& spec);

enum class mvVertexElement
{
    Position2D,
    Position3D,
    TexCoord0,
    TexCoord1,
    Color3_0,
    Color3_1,
    Color4_0,
    Color4_1,
    Normal,
    Tangent,
    Joints0,
    Joints1,
    Weights0,
    Weights1,
};

struct mvPipelineSpec
{
    std::string pixelShader;
    std::string vertexShader;
    unsigned int sampleCount = 0u;
};

struct mvPipeline
{
    mvPipelineSpec            spec;
    MTL::Library*             vertexLibrary = nullptr;
    MTL::Library*             pixelLibrary = nullptr;
    MTL::RenderPipelineState* pipelineState = nullptr;
};
