#include "mvPasses.h"
#include "mvMesh.h"
#include "mvAssetManager.h"

mvPass
mvCreateMainPass()
{
    mvPass pass{};

    pass.target = GContext->graphics.target.GetAddressOf();
    pass.depthStencil = GContext->graphics.targetDepth.GetAddressOf();
    pass.viewport.MinDepth = 0.0f;
    pass.viewport.MaxDepth = 1.0f;
    pass.viewport.TopLeftX = 0.0f;
    pass.viewport.TopLeftY = 0.0f;
    pass.viewport.Width = GContext->viewport.width;
    pass.viewport.Height = GContext->viewport.height;
    pass.rasterizationState = nullptr;
    return pass;
}

mvPass
mvCreateShadowPass(mvShadowMap& shadowMap)
{
    mvPass pass{};

    pass.target = nullptr;
    pass.depthStencil = shadowMap.shadowDepthView.GetAddressOf();
    pass.viewport.MinDepth = 0.0f;
    pass.viewport.MaxDepth = 1.0f;
    pass.viewport.TopLeftX = 0.0f;
    pass.viewport.TopLeftY = 0.0f;
    pass.viewport.Width = shadowMap.shadowMapDimension;
    pass.viewport.Height = shadowMap.shadowMapDimension;
    pass.rasterizationState = shadowMap.shadowRasterizationState.GetAddressOf();
    return pass;
}

mvPass
mvCreateShadowPass(mvShadowCubeMap& shadowMap, u32 index)
{
    mvPass pass{};

    pass.target = nullptr;
    pass.depthStencil = shadowMap.shadowDepthViews[index].GetAddressOf();
    pass.viewport.MinDepth = 0.0f;
    pass.viewport.MaxDepth = 1.0f;
    pass.viewport.TopLeftX = 0.0f;
    pass.viewport.TopLeftY = 0.0f;
    pass.viewport.Width = shadowMap.shadowMapDimension;
    pass.viewport.Height = shadowMap.shadowMapDimension;
    pass.rasterizationState = shadowMap.shadowRasterizationState.GetAddressOf();
    return pass;
}

mvSkyboxPass
mvCreateSkyboxPass(mvAssetManager* assetManager, const std::string& path)
{
    mvSkyboxPass pass{};

    pass.basePass.target = GContext->graphics.target.GetAddressOf();
    pass.basePass.depthStencil = GContext->graphics.targetDepth.GetAddressOf();
    pass.basePass.viewport.MinDepth = 0.0f;
    pass.basePass.viewport.MaxDepth = 1.0f;
    pass.basePass.viewport.TopLeftX = 0.0f;
    pass.basePass.viewport.TopLeftY = 0.0f;
    pass.basePass.viewport.Width = GContext->viewport.width;
    pass.basePass.viewport.Height = GContext->viewport.height;
    pass.basePass.rasterizationState = nullptr;

    pass.pipeline = {};
    pass.pipeline.info.pixelShader = "Skybox_PS.hlsl";
    pass.pipeline.info.vertexShader = "Skybox_VS.hlsl";
    pass.pipeline.info.layout = mvCreateVertexLayout({mvVertexElement::Position3D});
    pass.pipeline.info.depthBias = 0;    // not used
    pass.pipeline.info.slopeBias = 0.0f; // not used
    pass.pipeline.info.clamp = 0.0f;     // not used
    pass.pipeline.info.cull = false;

    pass.pipeline.topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    D3D11_RASTERIZER_DESC rasterDesc = CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT{});
    rasterDesc.CullMode = D3D11_CULL_NONE;
    rasterDesc.FrontCounterClockwise = TRUE;
    rasterDesc.DepthBias = pass.pipeline.info.depthBias;
    rasterDesc.DepthBiasClamp = pass.pipeline.info.clamp;
    rasterDesc.SlopeScaledDepthBias = pass.pipeline.info.slopeBias;

    GContext->graphics.device->CreateRasterizerState(&rasterDesc, pass.pipeline.rasterizationState.GetAddressOf());

    D3D11_DEPTH_STENCIL_DESC dsDesc = CD3D11_DEPTH_STENCIL_DESC{ CD3D11_DEFAULT{} };
    
    // Depth test parameters
    dsDesc.DepthEnable = true;
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

    //// Stencil test parameters
    //dsDesc.StencilEnable = true;
    //dsDesc.StencilReadMask = 0xFF;
    //dsDesc.StencilWriteMask = 0xFF;

    //// Stencil operations if pixel is front-facing
    //dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    //dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
    //dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    //dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    //// Stencil operations if pixel is back-facing
    //dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    //dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
    //dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    //dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    GContext->graphics.device->CreateDepthStencilState(&dsDesc, pass.pipeline.depthStencilState.GetAddressOf());

    D3D11_BLEND_DESC blendDesc = CD3D11_BLEND_DESC{ CD3D11_DEFAULT{} };
    auto& brt = blendDesc.RenderTarget[0];
    brt.BlendEnable = TRUE;
    brt.SrcBlend = D3D11_BLEND_SRC_ALPHA;
    brt.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    GContext->graphics.device->CreateBlendState(&blendDesc, pass.pipeline.blendState.GetAddressOf());

    mvPixelShader pixelShader = mvCreatePixelShader(GContext->IO.shaderDirectory + pass.pipeline.info.pixelShader);
    mvVertexShader vertexShader = mvCreateVertexShader(GContext->IO.shaderDirectory + pass.pipeline.info.vertexShader, pass.pipeline.info.layout);

    pass.pipeline.pixelShader = pixelShader.shader;
    pass.pipeline.pixelBlob = pixelShader.blob;
    pass.pipeline.vertexShader = vertexShader.shader;
    pass.pipeline.vertexBlob = vertexShader.blob;
    pass.pipeline.inputLayout = vertexShader.inputLayout;

    const float side = 1.0f / 2.0f;
    auto vertices = std::vector<f32>{
        -side, -side, -side,
         side, -side, -side,
        -side,  side, -side,
         side,  side, -side,
        -side, -side,  side,
         side, -side,  side,
        -side,  side,  side,
         side,  side,  side
    };

    static auto indices = std::vector<u32>{
        1, 2, 0, 1, 3, 2,
        5, 3, 1, 5, 7, 3,
        3, 6, 2, 7, 6, 3,
        7, 5, 4, 6, 7, 4,
        2, 4, 0, 6, 4, 2,
        4, 1, 0, 4, 5, 1
    };

    mvMesh mesh{};

    mesh.name = "skybox cube";
    mesh.layout = pass.pipeline.info.layout;
    mesh.vertexBuffer = mvGetBufferAsset(assetManager,
        vertices.data(),
        vertices.size() * sizeof(f32),
        D3D11_BIND_VERTEX_BUFFER,
        "skybox_vertex" + std::to_string(side));
    mesh.indexBuffer = mvGetBufferAsset(assetManager, indices.data(), indices.size() * sizeof(u32), D3D11_BIND_INDEX_BUFFER, "skybox_index");
    pass.mesh = mvRegistryMeshAsset(assetManager, mesh);
    pass.cubeTexture = mvGetCubeTextureAsset(assetManager, path);
    pass.sampler = mvGetSamplerAsset(assetManager, D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_BORDER, false);

    return pass;
}