#pragma once

#include <string>
#include "mvVertexLayout.h"

struct mvPixelShader
{
    ID3D11PixelShader* shader;
    ID3DBlob*          blob;
    std::string        path;
};

struct mvVertexShader
{
    ID3D11VertexShader* shader;
    ID3D11InputLayout*  inputLayout;
    ID3DBlob*           blob;
    std::string         path;
};

struct mvPipelineInfo
{
    std::string    pixelShader;
    std::string    vertexShader;
    mvVertexLayout layout;
    i32            depthBias;
    f32            slopeBias;
    f32            clamp;
    b8             cull = true;
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
    D3D11_PRIMITIVE_TOPOLOGY          topology;
    mvPipelineInfo                    info;
};

mvPipeline     mvFinalizePipeline  (mvPipelineInfo& info);
mvPixelShader  mvCreatePixelShader (const std::string& path);
mvVertexShader mvCreateVertexShader(const std::string& path, mvVertexLayout& layout);
