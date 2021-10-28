#pragma once

#include <string>
#include "mvVertexLayout.h"

struct mvPixelShader
{
    mvComPtr<ID3D11PixelShader>  shader;
    mvComPtr<ID3DBlob>           blob;
    std::string                  path;
};

struct mvVertexShader
{
    mvComPtr<ID3D11VertexShader> shader;
    mvComPtr<ID3D11InputLayout>  inputLayout;
    mvComPtr<ID3DBlob>           blob;
    std::string                  path;
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
    mvComPtr<ID3D11PixelShader>       pixelShader;
    mvComPtr<ID3D11VertexShader>      vertexShader;
    mvComPtr<ID3DBlob>                pixelBlob;
    mvComPtr<ID3DBlob>                vertexBlob;
    mvComPtr<ID3D11InputLayout>       inputLayout;
    mvComPtr<ID3D11BlendState>        blendState;
    mvComPtr<ID3D11DepthStencilState> depthStencilState;
    mvComPtr<ID3D11RasterizerState>   rasterizationState;
    D3D11_PRIMITIVE_TOPOLOGY          topology;
    mvPipelineInfo                    info;
};

mvPipeline     mvFinalizePipeline  (mvPipelineInfo& info);
mvPixelShader  mvCreatePixelShader (const std::string& path);
mvVertexShader mvCreateVertexShader(const std::string& path, mvVertexLayout& layout);
