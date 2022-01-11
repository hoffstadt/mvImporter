#include "mvPipeline.h"
#include <assert.h>
#include <d3dcompiler.h>
#include "mvSandbox.h"
#include "mvShader.h"

mvPipeline
finalize_pipeline(mvPipelineInfo& info)
{
    mvPipeline pipeline{};
    pipeline.info = info;

    pipeline.topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    D3D11_RASTERIZER_DESC rasterDesc = CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT{});
    rasterDesc.CullMode = info.cull ? D3D11_CULL_BACK : D3D11_CULL_NONE;
    rasterDesc.FrontCounterClockwise = TRUE;
    rasterDesc.DepthBias = info.depthBias;
    rasterDesc.DepthBiasClamp = info.clamp;
    rasterDesc.SlopeScaledDepthBias = info.slopeBias;

    GContext->graphics.device->CreateRasterizerState(&rasterDesc, &pipeline.rasterizationState);

	D3D11_DEPTH_STENCIL_DESC dsDesc = CD3D11_DEPTH_STENCIL_DESC{ CD3D11_DEFAULT{} };
    // Depth test parameters
    dsDesc.DepthEnable = true;
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    dsDesc.DepthFunc = D3D11_COMPARISON_LESS;

    // Stencil test parameters
    dsDesc.StencilEnable = true;
    dsDesc.StencilReadMask = 0xFF;
    dsDesc.StencilWriteMask = 0xFF;

    // Stencil operations if pixel is front-facing
    dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
    dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    // Stencil operations if pixel is back-facing
    dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
    dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    GContext->graphics.device->CreateDepthStencilState(&dsDesc, &pipeline.depthStencilState);

    D3D11_BLEND_DESC blendDesc = CD3D11_BLEND_DESC{ CD3D11_DEFAULT{} };
    auto& brt = blendDesc.RenderTarget[0];
    brt.BlendEnable = TRUE;
    brt.SrcBlend = D3D11_BLEND_SRC_ALPHA;
    brt.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    GContext->graphics.device->CreateBlendState(&blendDesc, &pipeline.blendState);

    std::vector<D3D_SHADER_MACRO> macros;
    for (auto& mac : info.macros)
        macros.push_back({ mac.macro.c_str(), mac.value.c_str()});

    macros.push_back({ NULL, NULL });

    if (!info.pixelShader.empty())
    {
        mvPixelShader pixelShader = create_pixel_shader(GContext->IO.shaderDirectory + info.pixelShader, macros.empty() ? nullptr : &macros);
        pipeline.pixelShader = pixelShader.shader;
        pipeline.pixelBlob = pixelShader.blob;
    }

    mvVertexShader vertexShader = create_vertex_shader(GContext->IO.shaderDirectory + info.vertexShader, info.layout, macros.empty() ? nullptr : &macros);

    pipeline.vertexShader = vertexShader.shader;
    pipeline.vertexBlob = vertexShader.blob;
    pipeline.inputLayout = vertexShader.inputLayout;

    return pipeline;
}
