#include "mvPasses.h"
#include "mvMesh.h"
#include "mvAssetManager.h"
#include <assert.h>

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

    HRESULT hResult = GContext->graphics.device->CreateRasterizerState(&rasterDesc, pass.pipeline.rasterizationState.GetAddressOf());
    assert(SUCCEEDED(hResult));

    //pass.basePass.rasterizationState = pass.pipeline.rasterizationState.GetAddressOf();

    D3D11_DEPTH_STENCIL_DESC dsDesc = CD3D11_DEPTH_STENCIL_DESC{ CD3D11_DEFAULT{} };
    
    // Depth test parameters
    dsDesc.DepthEnable = true;
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

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
        0, 2, 1, 2, 3, 1,
        1, 3, 5, 3, 7, 5,
        2, 6, 3, 3, 6, 7,
        4, 5, 7, 4, 7, 6,
        0, 4, 2, 2, 4, 6,
        0, 1, 4, 1, 5, 4
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