#pragma once

#include <string>
#include "mvVertexLayout.h"
#include "mvShader.h"

struct mvPipelineInfo;
struct mvPipeline;

mvPipeline finalize_pipeline(mvPipelineInfo& info);

struct mvPipelineInfo
{
    std::string                   pixelShader;
    std::string                   vertexShader;
    mvVertexLayout                layout;
    i32                           depthBias;
    f32                           slopeBias;
    f32                           clamp;
    b8                            cull = true;
    std::vector<D3D_SHADER_MACRO> macros;
};

struct mvPipeline
{
    ID3D11PixelShader*       pixelShader = nullptr;
    ID3D11VertexShader*      vertexShader = nullptr;
    ID3DBlob*                pixelBlob = nullptr;
    ID3DBlob*                vertexBlob = nullptr;
    ID3D11InputLayout*       inputLayout = nullptr;
    ID3D11BlendState*        blendState = nullptr;
    ID3D11DepthStencilState* depthStencilState = nullptr;
    ID3D11RasterizerState*   rasterizationState = nullptr;
    D3D11_PRIMITIVE_TOPOLOGY topology;
    mvPipelineInfo           info;
};


