#include "mvRenderer.h"
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>
#include "mvSandbox.h"
#include "mvGraphics.h"
#include "mvBuffers.h"
#include "mvPipeline.h"
#include "mvAssetLoader.h"
#include "mvAnimation.h"

namespace Renderer{

mvRendererContext
create_renderer_context()
{
    mvRendererContext ctx{};

    ctx.globalInfoBuffer = create_const_buffer(&ctx.globalInfo, sizeof(GlobalInfo));

    D3D11_BLEND_DESC desc{};
    desc.AlphaToCoverageEnable = false;
    desc.RenderTarget[0].BlendEnable = false;
    desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
    desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    GContext->graphics.device->CreateBlendState(&desc, &ctx.finalBlendState);

    {
        D3D11_RASTERIZER_DESC rasterDesc = CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT{});
        rasterDesc.CullMode = D3D11_CULL_NONE;
        rasterDesc.FillMode = D3D11_FILL_WIREFRAME;

        GContext->graphics.device->CreateRasterizerState(&rasterDesc, ctx.solidWireframePipeline.rasterizationState.GetAddressOf());

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

        GContext->graphics.device->CreateDepthStencilState(&dsDesc, ctx.solidWireframePipeline.depthStencilState.GetAddressOf());

        D3D11_BLEND_DESC blendDesc = CD3D11_BLEND_DESC{ CD3D11_DEFAULT{} };
        auto& brt = blendDesc.RenderTarget[0];
        brt.BlendEnable = TRUE;
        brt.SrcBlend = D3D11_BLEND_SRC_ALPHA;
        brt.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        GContext->graphics.device->CreateBlendState(&blendDesc, ctx.solidWireframePipeline.blendState.GetAddressOf());

        mvPixelShader pixelShader = create_pixel_shader(GContext->IO.shaderDirectory + "Solid_PS.hlsl");
        mvVertexShader vertexShader = create_vertex_shader(GContext->IO.shaderDirectory + "Solid_VS.hlsl", create_vertex_layout({ mvVertexElement::Position3D }));

        ctx.solidWireframePipeline.pixelShader = pixelShader.shader;
        ctx.solidWireframePipeline.pixelBlob = pixelShader.blob;
        ctx.solidWireframePipeline.vertexShader = vertexShader.shader;
        ctx.solidWireframePipeline.vertexBlob = vertexShader.blob;
        ctx.solidWireframePipeline.inputLayout = vertexShader.inputLayout;
        ctx.solidWireframePipeline.topology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;

        ctx.solidWireframePipeline.info.depthBias = 50;
        ctx.solidWireframePipeline.info.slopeBias = 2.0f;
        ctx.solidWireframePipeline.info.clamp = 0.1f;
        ctx.solidWireframePipeline.info.cull = false;

        ctx.solidWireframePipeline.info.layout = create_vertex_layout({mvVertexElement::Position3D});

    }

    {

        D3D11_RASTERIZER_DESC rasterDesc = CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT{});
        rasterDesc.CullMode = D3D11_CULL_BACK;
        rasterDesc.FrontCounterClockwise = TRUE;

        GContext->graphics.device->CreateRasterizerState(&rasterDesc, ctx.solidPipeline.rasterizationState.GetAddressOf());

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

        GContext->graphics.device->CreateDepthStencilState(&dsDesc, ctx.solidPipeline.depthStencilState.GetAddressOf());

        D3D11_BLEND_DESC blendDesc = CD3D11_BLEND_DESC{ CD3D11_DEFAULT{} };
        auto& brt = blendDesc.RenderTarget[0];
        brt.BlendEnable = TRUE;
        brt.SrcBlend = D3D11_BLEND_SRC_ALPHA;
        brt.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        GContext->graphics.device->CreateBlendState(&blendDesc, ctx.solidPipeline.blendState.GetAddressOf());

        mvPixelShader pixelShader = create_pixel_shader(GContext->IO.shaderDirectory + "Solid_PS.hlsl");
        mvVertexShader vertexShader = create_vertex_shader(GContext->IO.shaderDirectory + "Solid_VS.hlsl", create_vertex_layout({ mvVertexElement::Position3D }));

        ctx.solidPipeline.pixelShader = pixelShader.shader;
        ctx.solidPipeline.pixelBlob = pixelShader.blob;
        ctx.solidPipeline.vertexShader = vertexShader.shader;
        ctx.solidPipeline.vertexBlob = vertexShader.blob;
        ctx.solidPipeline.inputLayout = vertexShader.inputLayout;
        ctx.solidPipeline.topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        ctx.solidPipeline.info.depthBias = 50;
        ctx.solidPipeline.info.slopeBias = 2.0f;
        ctx.solidPipeline.info.clamp = 0.1f;
        ctx.solidPipeline.info.cull = false;
        ctx.solidPipeline.info.layout = create_vertex_layout({mvVertexElement::Position3D});
    }

    {
        ctx.skyboxPipeline.topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;


        mvVertexLayout vertexLayout = create_vertex_layout({ mvVertexElement::Position3D });

        mvPixelShader pixelShader = create_pixel_shader(GContext->IO.shaderDirectory + "Skybox_PS.hlsl");
        ctx.skyboxPipeline.pixelShader = pixelShader.shader;
        ctx.skyboxPipeline.pixelBlob = pixelShader.blob;

        mvVertexShader vertexShader = create_vertex_shader(GContext->IO.shaderDirectory + "Skybox_VS.hlsl", vertexLayout);
        ctx.skyboxPipeline.vertexShader = vertexShader.shader;
        ctx.skyboxPipeline.vertexBlob = vertexShader.blob;
        ctx.skyboxPipeline.inputLayout = vertexShader.inputLayout;

        // depth stencil state
        D3D11_DEPTH_STENCIL_DESC dsDesc = CD3D11_DEPTH_STENCIL_DESC{ CD3D11_DEFAULT{} };
        dsDesc.DepthEnable = true;
        dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
        dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
        GContext->graphics.device->CreateDepthStencilState(&dsDesc, ctx.skyboxPipeline.depthStencilState.GetAddressOf());

        // rasterizer state
        D3D11_RASTERIZER_DESC skyboxRasterDesc = CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT{});
        skyboxRasterDesc.CullMode = D3D11_CULL_NONE;
        skyboxRasterDesc.FrontCounterClockwise = TRUE;
        GContext->graphics.device->CreateRasterizerState(&skyboxRasterDesc, ctx.skyboxPipeline.rasterizationState.GetAddressOf());

        // blend state
        D3D11_BLEND_DESC blendDesc = CD3D11_BLEND_DESC{ CD3D11_DEFAULT{} };
        auto& brt = blendDesc.RenderTarget[0];
        brt.BlendEnable = TRUE;
        brt.SrcBlend = D3D11_BLEND_SRC_ALPHA;
        brt.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        GContext->graphics.device->CreateBlendState(&blendDesc, ctx.skyboxPipeline.blendState.GetAddressOf());
    }

    return ctx;
}

static void
submit_mesh(mvModel& model, mvRendererContext& ctx, mvMesh& mesh, mvMat4 transform, mvSkin* skin)
{
    auto device = GContext->graphics.imDeviceContext;

    for (u32 i = 0; i < mesh.primitives.size(); i++)
    {
        mvMeshPrimitive& primitive = mesh.primitives[i];

        // if material not assigned, get material
        if (primitive.materialID == -1)
        {
            assert(false && "material not assigned");
        }

        mvMaterial* material = &model.materialManager.materials[primitive.materialID].asset;

        if(mesh.morphBuffer.buffer)
            update_const_buffer(mesh.morphBuffer, mesh.weightsAnimated.data());

        if (material->alphaMode == 2)
            ctx.transparentJobs.push_back({ &primitive, transform, skin, mesh.morphBuffer.buffer });
        else
            ctx.opaqueJobs.push_back({ &primitive, transform, skin, mesh.morphBuffer.buffer });
    }
}

static void
submit_node(mvModel& model, mvRendererContext& ctx, mvNode& node, mvMat4 parentTransform)
{
    mvSkin* skin = nullptr;

    node.worldTransform = parentTransform * node.transform;
    node.inverseWorldTransform = invert(node.worldTransform);

    if (node.skin != -1)
        skin = &model.skins[node.skin];
    if (node.mesh > -1 && node.camera == -1)
        submit_mesh(model, ctx, model.meshes[node.mesh], node.worldTransform, skin);
    else if (node.camera > -1)
    {
        for (u32 i = 0; i < model.meshes[node.mesh].primitives.size(); i++)
        {
            mvMeshPrimitive& primitive = model.meshes[node.mesh].primitives[i];
            ctx.wireframeJobs.push_back({ &primitive, node.worldTransform* scale(identity_mat4(), {-1.0f, -1.0f, -1.0f}) });
        }
    }

    for (u32 i = 0; i < node.childCount; i++)
    {
        submit_node(model, ctx, model.nodes[node.children[i]], node.worldTransform);
    }
}

void 
submit_scene(mvModel& model, mvRendererContext& ctx, mvScene& scene)
{
    mvSkin* skin = nullptr;

    for (u32 i = 0; i < scene.nodeCount; i++)
    {
        mvNode& rootNode = model.nodes[scene.nodes[i]];
        rootNode.worldTransform = rootNode.transform;
        rootNode.inverseWorldTransform = invert(rootNode.worldTransform);

        if (rootNode.skin != -1)
            skin = &model.skins[rootNode.skin];

        if (rootNode.mesh > -1 && rootNode.camera == -1)
            submit_mesh(model, ctx, model.meshes[rootNode.mesh], rootNode.worldTransform, skin);
        else if (rootNode.camera > -1)
        {
            for (u32 i = 0; i < model.meshes[rootNode.mesh].primitives.size(); i++)
            {
                mvMeshPrimitive& primitive = model.meshes[rootNode.mesh].primitives[i];
                ctx.wireframeJobs.push_back({ &primitive, rootNode.worldTransform * scale(identity_mat4(), {-1.0f, -1.0f, -1.0f}) });
            }
        }

        for (u32 j = 0; j < rootNode.childCount; j++)
        {
            submit_node(model, ctx, model.nodes[rootNode.children[j]], rootNode.worldTransform);
        }
    }
}

static void
render_job(mvModel& model, mvRenderJob& job, mvMat4 cam, mvMat4 proj)
{
    auto device = GContext->graphics.imDeviceContext;

    mvMeshPrimitive& primitive = *job.meshPrimitive;

    mvMaterial* material = &model.materialManager.materials[primitive.materialID].asset;

    if (material->pipeline.info.layout != primitive.layout)
    {
        assert(false && "Mesh and material vertex layouts don't match.");
        return;
    }

    // pipeline
    set_pipeline_state(material->pipeline);
    
    device->PSSetSamplers(0, 1, primitive.albedoTexture.sampler.GetAddressOf());
    device->PSSetSamplers(1, 1, primitive.normalTexture.sampler.GetAddressOf());
    device->PSSetSamplers(2, 1, primitive.metalRoughnessTexture.sampler.GetAddressOf());
    device->PSSetSamplers(3, 1, primitive.emissiveTexture.sampler.GetAddressOf());
    device->PSSetSamplers(4, 1, primitive.occlusionTexture.sampler.GetAddressOf());
    device->PSSetSamplers(5, 1, primitive.clearcoatTexture.sampler.GetAddressOf());
    device->PSSetSamplers(6, 1, primitive.clearcoatRoughnessTexture.sampler.GetAddressOf());
    device->PSSetSamplers(7, 1, primitive.clearcoatNormalTexture.sampler.GetAddressOf());


    static ID3D11SamplerState* emptySamplers = nullptr;
    device->VSSetSamplers(0, 1, job.skin ? job.skin->jointTexture.sampler.GetAddressOf() : &emptySamplers);
    device->VSSetSamplers(1, 1, primitive.morphTexture.sampler.GetAddressOf());

    // maps
    ID3D11ShaderResourceView* const pSRV[1] = { NULL };
    device->PSSetShaderResources(0, 1, primitive.albedoTexture.textureView.GetAddressOf());
    device->PSSetShaderResources(1, 1, primitive.normalTexture.textureView.GetAddressOf());
    device->PSSetShaderResources(2, 1, primitive.metalRoughnessTexture.textureView.GetAddressOf());
    device->PSSetShaderResources(3, 1, primitive.emissiveTexture.textureView.GetAddressOf());
    device->PSSetShaderResources(4, 1, primitive.occlusionTexture.textureView.GetAddressOf());
    device->PSSetShaderResources(5, 1, primitive.clearcoatTexture.textureView.GetAddressOf());
    device->PSSetShaderResources(6, 1, primitive.clearcoatRoughnessTexture.textureView.GetAddressOf());
    device->PSSetShaderResources(7, 1, primitive.clearcoatNormalTexture.textureView.GetAddressOf());

    device->VSSetShaderResources(0, 1, job.skin ? job.skin->jointTexture.textureView.GetAddressOf() : pSRV);
    device->VSSetShaderResources(1, 1, primitive.morphTexture.textureView.GetAddressOf());

    update_const_buffer(material->buffer, &material->data);
    device->PSSetConstantBuffers(1u, 1u, material->buffer.buffer.GetAddressOf());

    mvTransforms transforms{};
    transforms.model = job.accumulatedTransform;
    transforms.modelView = cam * transforms.model;
    transforms.modelViewProjection = proj * cam * transforms.model;

    D3D11_MAPPED_SUBRESOURCE mappedSubresource;
    device->Map(GContext->graphics.tranformCBuf.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &mappedSubresource);
    memcpy(mappedSubresource.pData, &transforms, sizeof(mvTransforms));
    device->Unmap(GContext->graphics.tranformCBuf.Get(), 0u);

    // mesh
    static const UINT offset = 0u;
    device->VSSetConstantBuffers(0u, 1u, GContext->graphics.tranformCBuf.GetAddressOf());
    if (job.morphBuffer)
    {
        device->VSSetConstantBuffers(3u, 1u, job.morphBuffer.GetAddressOf());
    }
    device->IASetIndexBuffer(primitive.indexBuffer.buffer.Get(), DXGI_FORMAT_R32_UINT, 0u);
    device->IASetVertexBuffers(0u, 1u, primitive.vertexBuffer.buffer.GetAddressOf(), &material->pipeline.info.layout.size, &offset);

    // draw
    device->DrawIndexed(primitive.indexBuffer.size / sizeof(u32), 0u, 0u);
}

static void
render_wireframe_job(mvRendererContext& rendererCtx, mvModel& model, mvRenderJob& job, mvMat4 cam, mvMat4 proj)
{

    auto device = GContext->graphics.imDeviceContext;

    set_pipeline_state(rendererCtx.solidWireframePipeline);

    mvMeshPrimitive& primitive = *job.meshPrimitive;

    mvTransforms transforms{};
    transforms.model = job.accumulatedTransform;
    transforms.modelView = cam * transforms.model;
    transforms.modelViewProjection = proj * cam * transforms.model;

    D3D11_MAPPED_SUBRESOURCE mappedSubresource;
    device->Map(GContext->graphics.tranformCBuf.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &mappedSubresource);
    memcpy(mappedSubresource.pData, &transforms, sizeof(mvTransforms));
    device->Unmap(GContext->graphics.tranformCBuf.Get(), 0u);

    // mesh
    static const UINT offset = 0u;
    device->VSSetConstantBuffers(0u, 1u, GContext->graphics.tranformCBuf.GetAddressOf());
    device->IASetIndexBuffer(primitive.indexBuffer.buffer.Get(), DXGI_FORMAT_R32_UINT, 0u);
    device->IASetVertexBuffers(0u, 1u, primitive.vertexBuffer.buffer.GetAddressOf(), &rendererCtx.solidWireframePipeline.info.layout.size, &offset);

    // draw
    device->DrawIndexed(primitive.indexBuffer.size / sizeof(u32), 0u, 0u);
}

void 
render_scenes(mvModel& model, mvRendererContext& ctx, mvMat4 cam, mvMat4 proj)
{
    // opaque objects
    for (int i = 0; i < ctx.opaqueJobs.size(); i++)
        render_job(model, ctx.opaqueJobs[i], cam, proj);

    // transparent objects
    for (int i = 0; i < ctx.transparentJobs.size(); i++)
        render_job(model, ctx.transparentJobs[i], cam, proj);

    // wireframe objects
    for (int i = 0; i < ctx.wireframeJobs.size(); i++)
        render_wireframe_job(ctx, model, ctx.wireframeJobs[i], cam, proj);

    // reset
    ctx.opaqueJobs.clear();
    ctx.transparentJobs.clear();
    ctx.wireframeJobs.clear();
}

void
render_mesh_solid(mvRendererContext& rendererCtx, mvModel& model, mvMesh& mesh, mvMat4 transform, mvMat4 cam, mvMat4 proj)
{
    auto device = GContext->graphics.imDeviceContext;

    set_pipeline_state(rendererCtx.solidPipeline);

    for (u32 i = 0; i < mesh.primitives.size(); i++)
    {
        mvMeshPrimitive& primitive = mesh.primitives[i];

        mvTransforms transforms{};
        transforms.model = transform;
        transforms.modelView = cam * transforms.model;
        transforms.modelViewProjection = proj * cam * transforms.model;

        D3D11_MAPPED_SUBRESOURCE mappedSubresource;
        device->Map(GContext->graphics.tranformCBuf.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &mappedSubresource);
        memcpy(mappedSubresource.pData, &transforms, sizeof(mvTransforms));
        device->Unmap(GContext->graphics.tranformCBuf.Get(), 0u);

        // mesh
        static const UINT offset = 0u;
        device->VSSetConstantBuffers(0u, 1u, GContext->graphics.tranformCBuf.GetAddressOf());
        device->IASetIndexBuffer(primitive.indexBuffer.buffer.Get(), DXGI_FORMAT_R32_UINT, 0u);
        device->IASetVertexBuffers(0u, 1u, primitive.vertexBuffer.buffer.GetAddressOf(), &rendererCtx.solidPipeline.info.layout.size, &offset);

        // draw
        device->DrawIndexed(primitive.indexBuffer.size / sizeof(u32), 0u, 0u);
    }
}

void 
render_skybox(mvRendererContext& rendererCtx, mvModel& model, mvCubeTexture& cubemap, ID3D11SamplerState* sampler, mvMat4 cam, mvMat4 proj)
{

    static const float side = 1.0f / 2.0f;
    static auto vertices = std::vector<f32>{
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

    static mvVertexLayout vertexLayout = create_vertex_layout({ mvVertexElement::Position3D });
    static mvBuffer vertexBuffer = create_buffer(vertices.data(), vertices.size() * sizeof(f32), D3D11_BIND_VERTEX_BUFFER);
    static mvBuffer indexBuffer = create_buffer(indices.data(), indices.size() * sizeof(u32), D3D11_BIND_INDEX_BUFFER);

    auto ctx = GContext->graphics.imDeviceContext;

    ctx->PSSetSamplers(0, 1, &sampler);
    ctx->PSSetShaderResources(0, 1, cubemap.textureView.GetAddressOf());

    // pipeline
    set_pipeline_state(rendererCtx.skyboxPipeline);

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
    ctx->IASetIndexBuffer(indexBuffer.buffer.Get(), DXGI_FORMAT_R32_UINT, 0u);
    ctx->IASetVertexBuffers(0u, 1u, vertexBuffer.buffer.GetAddressOf(), &vertexLayout.size, &offset);

    // draw
    ctx->DrawIndexed(indexBuffer.size / sizeof(u32), 0u, 0u);
}

}