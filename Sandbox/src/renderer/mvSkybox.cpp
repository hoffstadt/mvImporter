#include "mvSkybox.h"
#include "mvAssetManager.h"
#include "mvSandbox.h"

mvSkybox
create_skybox(mvAssetManager& am)
{
    mvSkybox skybox{};

    // setup pipeline
    D3D11_DEPTH_STENCIL_DESC dsDesc = CD3D11_DEPTH_STENCIL_DESC{ CD3D11_DEFAULT{} };
    dsDesc.DepthEnable = true;
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

    D3D11_RASTERIZER_DESC skyboxRasterDesc = CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT{});
    skyboxRasterDesc.CullMode = D3D11_CULL_NONE;
    skyboxRasterDesc.FrontCounterClockwise = TRUE;

    D3D11_BLEND_DESC blendDesc = CD3D11_BLEND_DESC{ CD3D11_DEFAULT{} };
    auto& brt = blendDesc.RenderTarget[0];
    brt.BlendEnable = TRUE;
    brt.SrcBlend = D3D11_BLEND_SRC_ALPHA;
    brt.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;

    GContext->graphics.device->CreateDepthStencilState(&dsDesc, &skybox.pipeline.depthStencilState);
    GContext->graphics.device->CreateRasterizerState(&skyboxRasterDesc, &skybox.pipeline.rasterizationState);
    GContext->graphics.device->CreateBlendState(&blendDesc, &skybox.pipeline.blendState);

    mvPixelShader pixelShader = create_pixel_shader(GContext->IO.shaderDirectory + "Skybox_PS.hlsl");
    mvVertexShader vertexShader = create_vertex_shader(GContext->IO.shaderDirectory + "Skybox_VS.hlsl", create_vertex_layout({ mvVertexElement::Position3D }));

    skybox.pipeline.pixelShader = pixelShader.shader;
    skybox.pipeline.pixelBlob = pixelShader.blob;
    skybox.pipeline.vertexShader = vertexShader.shader;
    skybox.pipeline.vertexBlob = vertexShader.blob;
    skybox.pipeline.inputLayout = vertexShader.inputLayout;
    skybox.pipeline.topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

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

    skybox.vertexBuffer = create_buffer(vertices.data(), vertices.size() * sizeof(f32), D3D11_BIND_VERTEX_BUFFER);
    skybox.indexBuffer = create_buffer(indices.data(), indices.size() * sizeof(u32), D3D11_BIND_INDEX_BUFFER);
    skybox.vertexLayout = create_vertex_layout({ mvVertexElement::Position3D });
    //skybox.cubeTexture = create_cube_texture("../../Resources/SkyBox");
    skybox.cubeTexture = create_cube_texture("../../data/glTF-Sample-Environments/footprint_court.hdr", false);

    D3D11_SAMPLER_DESC samplerDesc{};
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
    samplerDesc.BorderColor[0] = 0.0f;
    //samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.MaxAnisotropy = D3D11_REQ_MAXANISOTROPY;
    GContext->graphics.device->CreateSamplerState(&samplerDesc, &skybox.cubeSampler);

    register_asset(&am, "skybox_pipeline", skybox.pipeline);
    register_asset(&am, "skybox_texture", skybox.cubeTexture);
    register_asset(&am, "skybox_sampler", mvSampler{ skybox.cubeSampler });
    register_asset(&am, "skybox_ibuffer", skybox.indexBuffer);
    register_asset(&am, "skybox_vbuffer", skybox.vertexBuffer);

    return skybox;
}

void
render_skybox(mvSkybox& skybox, mvMat4 cam, mvMat4 proj)
{

    auto ctx = GContext->graphics.imDeviceContext;

    // pipeline
    set_pipeline_state(skybox.pipeline);
    ctx->PSSetSamplers(0, 1, &skybox.cubeSampler);
    ctx->PSSetShaderResources(0, 1, &skybox.cubeTexture.textureView);

    mvTransforms transforms{};
    transforms.model = identity_mat4() * scale(identity_mat4(), mvVec3{ 1.0f, 1.0f, -1.0f });
    transforms.modelView = cam * transforms.model;
    transforms.modelViewProjection = proj * cam * transforms.model;

    D3D11_MAPPED_SUBRESOURCE mappedSubresource;
    ctx->Map(GContext->graphics.tranformCBuf.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &mappedSubresource);
    memcpy(mappedSubresource.pData, &transforms, sizeof(mvTransforms));
    ctx->Unmap(GContext->graphics.tranformCBuf.Get(), 0u);

    // mesh
    static const UINT offset = 0u;
    ctx->VSSetConstantBuffers(0u, 1u, GContext->graphics.tranformCBuf.GetAddressOf());
    ctx->IASetIndexBuffer(skybox.indexBuffer.buffer, DXGI_FORMAT_R32_UINT, 0u);
    ctx->IASetVertexBuffers(0u, 1u, &skybox.vertexBuffer.buffer, &skybox.vertexLayout.size, &offset);

    // draw
    ctx->DrawIndexed(skybox.indexBuffer.size / sizeof(u32), 0u, 0u);

}