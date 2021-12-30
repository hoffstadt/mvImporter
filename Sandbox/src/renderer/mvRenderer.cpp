#include "mvRenderer.h"
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>
#include <implot.h>
#include "mvSandbox.h"
#include "mvGraphics.h"
#include "mvBuffers.h"
#include "mvVertexLayout.h"
#include "mvAssetManager.h"

namespace Renderer{

void 
mvSetupCommonAssets(mvAssetManager& am)
{
    {
        mvPipelineInfo pipelineInfo{};
        pipelineInfo.pixelShader = "Shadow_PS.hlsl";
        pipelineInfo.vertexShader = "Shadow_VS.hlsl";
        pipelineInfo.depthBias = 50;
        pipelineInfo.slopeBias = 2.0f;
        pipelineInfo.clamp = 0.1f;
        pipelineInfo.cull = false;

        pipelineInfo.layout = create_vertex_layout(
            {
                mvVertexElement::Position3D,
                mvVertexElement::Normal,
                mvVertexElement::Texture2D,
                mvVertexElement::Tangent,
                mvVertexElement::Bitangent
            }
        );

        register_asset(&am, "shadow_alpha", finalize_pipeline(pipelineInfo));
    }

    {
        mvPipelineInfo pipelineInfo{};
        pipelineInfo.pixelShader = "";
        pipelineInfo.vertexShader = "Shadow_VS.hlsl";
        pipelineInfo.depthBias = 50;
        pipelineInfo.slopeBias = 2.0f;
        pipelineInfo.clamp = 0.1f;
        pipelineInfo.cull = false;

        pipelineInfo.layout = create_vertex_layout(
            {
                mvVertexElement::Position3D,
                mvVertexElement::Normal,
                mvVertexElement::Texture2D,
                mvVertexElement::Tangent,
                mvVertexElement::Bitangent
            }
        );

        register_asset(&am, "shadow_no_alpha", finalize_pipeline(pipelineInfo));
    }

    {
        mvPipeline pipeline{};

        D3D11_RASTERIZER_DESC rasterDesc = CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT{});
        rasterDesc.CullMode = D3D11_CULL_NONE;
        rasterDesc.FillMode = D3D11_FILL_WIREFRAME;

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

        mvPixelShader pixelShader = create_pixel_shader(GContext->IO.shaderDirectory + "Solid_PS.hlsl");
        mvVertexShader vertexShader = create_vertex_shader(GContext->IO.shaderDirectory + "Solid_VS.hlsl", create_vertex_layout({ mvVertexElement::Position3D }));

        pipeline.pixelShader = pixelShader.shader;
        pipeline.pixelBlob = pixelShader.blob;
        pipeline.vertexShader = vertexShader.shader;
        pipeline.vertexBlob = vertexShader.blob;
        pipeline.inputLayout = vertexShader.inputLayout;
        pipeline.topology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;

        pipeline.info.depthBias = 50;
        pipeline.info.slopeBias = 2.0f;
        pipeline.info.clamp = 0.1f;
        pipeline.info.cull = false;

        pipeline.info.layout = create_vertex_layout(
            {
                mvVertexElement::Position3D
            }
        );

        register_asset(&am, "solid_wireframe", pipeline);
    }

    {
        mvPipeline pipeline{};

        D3D11_RASTERIZER_DESC rasterDesc = CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT{});
        rasterDesc.CullMode = D3D11_CULL_BACK;
        rasterDesc.FrontCounterClockwise = TRUE;

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

        mvPixelShader pixelShader = create_pixel_shader(GContext->IO.shaderDirectory + "Solid_PS.hlsl");
        mvVertexShader vertexShader = create_vertex_shader(GContext->IO.shaderDirectory + "Solid_VS.hlsl", create_vertex_layout({ mvVertexElement::Position3D }));

        pipeline.pixelShader = pixelShader.shader;
        pipeline.pixelBlob = pixelShader.blob;
        pipeline.vertexShader = vertexShader.shader;
        pipeline.vertexBlob = vertexShader.blob;
        pipeline.inputLayout = vertexShader.inputLayout;
        pipeline.topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

        //pipeline.info.pixelShader = "";
        //pipeline.info.vertexShader = "Shadow_VS.hlsl";
        pipeline.info.depthBias = 50;
        pipeline.info.slopeBias = 2.0f;
        pipeline.info.clamp = 0.1f;
        pipeline.info.cull = false;

        pipeline.info.layout = create_vertex_layout(
            {
                mvVertexElement::Position3D
            }
        );

        register_asset(&am, "solid", pipeline);
    }

    {

        mvPipeline pipeline{};
        pipeline.topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

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

        mvBuffer vertexBuffer = create_buffer(vertices.data(), vertices.size() * sizeof(f32), D3D11_BIND_VERTEX_BUFFER);
        mvBuffer indexBuffer = create_buffer(indices.data(), indices.size() * sizeof(u32), D3D11_BIND_INDEX_BUFFER);
        mvVertexLayout vertexLayout = create_vertex_layout({ mvVertexElement::Position3D });

        mvPixelShader pixelShader = create_pixel_shader(GContext->IO.shaderDirectory + "Skybox_PS.hlsl");
        pipeline.pixelShader = pixelShader.shader;
        pipeline.pixelBlob = pixelShader.blob;

        mvVertexShader vertexShader = create_vertex_shader(GContext->IO.shaderDirectory + "Skybox_VS.hlsl", vertexLayout);
        pipeline.vertexShader = vertexShader.shader;
        pipeline.vertexBlob = vertexShader.blob;
        pipeline.inputLayout = vertexShader.inputLayout;

        // depth stencil state
        D3D11_DEPTH_STENCIL_DESC dsDesc = CD3D11_DEPTH_STENCIL_DESC{ CD3D11_DEFAULT{} };
        dsDesc.DepthEnable = true;
        dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
        dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
        GContext->graphics.device->CreateDepthStencilState(&dsDesc, &pipeline.depthStencilState);

        // rasterizer state
        D3D11_RASTERIZER_DESC skyboxRasterDesc = CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT{});
        skyboxRasterDesc.CullMode = D3D11_CULL_NONE;
        skyboxRasterDesc.FrontCounterClockwise = TRUE;
        GContext->graphics.device->CreateRasterizerState(&skyboxRasterDesc, &pipeline.rasterizationState);

        // blend state
        D3D11_BLEND_DESC blendDesc = CD3D11_BLEND_DESC{ CD3D11_DEFAULT{} };
        auto& brt = blendDesc.RenderTarget[0];
        brt.BlendEnable = TRUE;
        brt.SrcBlend = D3D11_BLEND_SRC_ALPHA;
        brt.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        GContext->graphics.device->CreateBlendState(&blendDesc, &pipeline.blendState);

        // sampler
        D3D11_SAMPLER_DESC samplerDesc{};
        samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
        samplerDesc.AddressV = samplerDesc.AddressU;
        samplerDesc.AddressW = samplerDesc.AddressU;
        samplerDesc.BorderColor[0] = 0.0f;
        samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        samplerDesc.MaxAnisotropy = D3D11_REQ_MAXANISOTROPY;

        ID3D11SamplerState* sampler = nullptr;
        GContext->graphics.device->CreateSamplerState(&samplerDesc, &sampler);

        register_asset(&am, "skybox_pipeline", pipeline);
        register_asset(&am, "skybox_sampler", sampler);
        register_asset(&am, "skybox_ibuffer", indexBuffer);
        register_asset(&am, "skybox_vbuffer", vertexBuffer);
    }
}

static void
submit_mesh(mvAssetManager& am, mvRendererContext& ctx, mvMesh& mesh, mvMat4 transform)
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

        mvMaterial* material = &am.materials[primitive.materialID].asset;

        if (material->data.alphaMode == 2)
        {
            ctx.transparentJobs[ctx.transparentJobCount] = { &primitive, transform };
            ctx.transparentJobCount++;
        }
        else
        {
            ctx.opaqueJobs[ctx.opaqueJobCount] = { &primitive, transform };
            ctx.opaqueJobCount++;
        }
    }
}

static void
submit_node(mvAssetManager& am, mvRendererContext& ctx, mvNode& node, mvMat4 transform)
{

    if (node.mesh > -1 && node.camera == -1)
        submit_mesh(am, ctx, am.meshes[node.mesh].asset, transform * node.matrix);
    else if (node.camera > -1)
    {
        for (u32 i = 0; i < am.meshes[node.mesh].asset.primitives.size(); i++)
        {
            mvMeshPrimitive& primitive = am.meshes[node.mesh].asset.primitives[i];
            ctx.wireframeJobs[ctx.wireframeJobCount] = { &primitive, transform * node.matrix * scale(identity_mat4(), { -1.0f, -1.0f, -1.0f }) };
            ctx.wireframeJobCount++;
        }
    }

    for (u32 i = 0; i < node.childCount; i++)
    {
        submit_node(am, ctx, am.nodes[node.children[i]].asset, transform * node.matrix);
    }
}

void 
submit_scene(mvAssetManager& am, mvRendererContext& ctx, mvScene& scene, mvMat4 scaleM, mvMat4 trans)
{
    for (u32 i = 0; i < scene.nodeCount; i++)
    {
        mvNode& rootNode = am.nodes[scene.nodes[i]].asset;

        if (rootNode.mesh > -1 && rootNode.camera == -1)
            submit_mesh(am, ctx, am.meshes[rootNode.mesh].asset, trans * rootNode.matrix * scaleM);
        else if (rootNode.camera > -1)
        {
            for (u32 i = 0; i < am.meshes[rootNode.mesh].asset.primitives.size(); i++)
            {
                mvMeshPrimitive& primitive = am.meshes[rootNode.mesh].asset.primitives[i];
                ctx.wireframeJobs[ctx.wireframeJobCount] = { &primitive, trans * rootNode.matrix * scale(identity_mat4(), { -1.0f, -1.0f, -1.0f }) };
                ctx.wireframeJobCount++;
            }
        }

        for (u32 j = 0; j < rootNode.childCount; j++)
        {
            submit_node(am, ctx, am.nodes[rootNode.children[j]].asset, trans * rootNode.matrix * scaleM);
        }
    }
}

static void
render_job(mvAssetManager& am, mvRenderJob& job, mvMat4 cam, mvMat4 proj)
{
    auto device = GContext->graphics.imDeviceContext;

    mvMeshPrimitive& primitive = *job.meshPrimitive;

    mvMaterial* material = &am.materials[primitive.materialID].asset;

    mvTexture* albedoMap = primitive.albedoTexture == -1 ? nullptr : &am.textures[primitive.albedoTexture].asset;
    mvTexture* normMap = primitive.normalTexture == -1 ? nullptr : &am.textures[primitive.normalTexture].asset;
    mvTexture* metalRoughMap = primitive.metalRoughnessTexture == -1 ? nullptr : &am.textures[primitive.metalRoughnessTexture].asset;
    mvTexture* emissiveMap = primitive.emissiveTexture == -1 ? nullptr : &am.textures[primitive.emissiveTexture].asset;
    mvTexture* occlussionMap = primitive.occlusionTexture == -1 ? nullptr : &am.textures[primitive.occlusionTexture].asset;

    mvPipeline& pipeline = am.pipelines[material->pipeline].asset;

    if (pipeline.info.layout != primitive.layout)
    {
        assert(false && "Mesh and material vertex layouts don't match.");
        return;
    }

    // pipeline
    set_pipeline_state(pipeline);
    static ID3D11SamplerState* emptySamplers = nullptr;
    device->PSSetSamplers(0, 1, albedoMap ? &albedoMap->sampler : &emptySamplers);
    device->PSSetSamplers(1, 1, normMap ? &normMap->sampler : &emptySamplers);
    device->PSSetSamplers(2, 1, metalRoughMap ? &metalRoughMap->sampler : &emptySamplers);
    device->PSSetSamplers(3, 1, metalRoughMap ? &metalRoughMap->sampler : &emptySamplers);
    device->PSSetSamplers(4, 1, occlussionMap ? &occlussionMap->sampler : &emptySamplers);

    // maps
    ID3D11ShaderResourceView* const pSRV[1] = { NULL };
    device->PSSetShaderResources(0, 1, albedoMap ? &albedoMap->textureView : pSRV);
    device->PSSetShaderResources(1, 1, normMap ? &normMap->textureView : pSRV);
    device->PSSetShaderResources(2, 1, metalRoughMap ? &metalRoughMap->textureView : pSRV);
    device->PSSetShaderResources(3, 1, emissiveMap ? &emissiveMap->textureView : pSRV);
    device->PSSetShaderResources(4, 1, occlussionMap ? &occlussionMap->textureView : pSRV);

    update_const_buffer(material->buffer, &material->data);
    device->PSSetConstantBuffers(1u, 1u, &material->buffer.buffer);

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
    device->IASetIndexBuffer(am.buffers[primitive.indexBuffer].asset.buffer, DXGI_FORMAT_R32_UINT, 0u);
    device->IASetVertexBuffers(0u, 1u,
        &am.buffers[primitive.vertexBuffer].asset.buffer,
        &pipeline.info.layout.size, &offset);

    // draw
    device->DrawIndexed(am.buffers[primitive.indexBuffer].asset.size / sizeof(u32), 0u, 0u);
}

void 
render_jobs(mvAssetManager& am, mvRendererContext& ctx, mvMat4 cam, mvMat4 proj)
{
    // opaque objects
    for (int i = 0; i < ctx.opaqueJobCount; i++)
        render_job(am, ctx.opaqueJobs[i], cam, proj);

    // transparent objects
    for (int i = 0; i < ctx.transparentJobCount; i++)
        render_job(am, ctx.transparentJobs[i], cam, proj);

    // wireframe objects
    for (int i = 0; i < ctx.wireframeJobCount; i++)
        render_job(am, ctx.wireframeJobs[i], cam, proj);

    // reset
    ctx.opaqueJobCount = 0u;
    ctx.transparentJobCount = 0u;
    ctx.wireframeJobCount = 0u;
}

void
render_mesh(mvAssetManager& am, mvMesh& mesh, mvMat4 transform, mvMat4 cam, mvMat4 proj)
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

        mvMaterial* material = &am.materials[primitive.materialID].asset;

        mvTexture* albedoMap = primitive.albedoTexture == -1 ? nullptr : &am.textures[primitive.albedoTexture].asset;
        mvTexture* normMap = primitive.normalTexture == -1 ? nullptr : &am.textures[primitive.normalTexture].asset;
        mvTexture* metalRoughMap = primitive.metalRoughnessTexture == -1 ? nullptr : &am.textures[primitive.metalRoughnessTexture].asset;
        mvTexture* emissiveMap = primitive.emissiveTexture == -1 ? nullptr : &am.textures[primitive.emissiveTexture].asset;
        mvTexture* occlussionMap = primitive.occlusionTexture == -1 ? nullptr : &am.textures[primitive.occlusionTexture].asset;

        mvPipeline& pipeline = am.pipelines[material->pipeline].asset;

        if (pipeline.info.layout != primitive.layout)
        {
            assert(false && "Mesh and material vertex layouts don't match.");
            return;
        }

        // pipeline
        set_pipeline_state(pipeline);
        static ID3D11SamplerState* emptySamplers = nullptr;
        device->PSSetSamplers(0, 1, albedoMap ? &albedoMap->sampler : &emptySamplers);
        device->PSSetSamplers(1, 1, normMap ? &normMap->sampler : &emptySamplers);
        device->PSSetSamplers(2, 1, metalRoughMap ? &metalRoughMap->sampler : &emptySamplers);
        device->PSSetSamplers(3, 1, metalRoughMap ? &metalRoughMap->sampler : &emptySamplers);
        device->PSSetSamplers(4, 1, occlussionMap ? &occlussionMap->sampler : &emptySamplers);

        // maps
        ID3D11ShaderResourceView* const pSRV[1] = { NULL };
        device->PSSetShaderResources(0, 1, albedoMap ? &albedoMap->textureView : pSRV);
        device->PSSetShaderResources(1, 1, normMap ? &normMap->textureView : pSRV);
        device->PSSetShaderResources(2, 1, metalRoughMap ? &metalRoughMap->textureView : pSRV);
        device->PSSetShaderResources(3, 1, emissiveMap ? &emissiveMap->textureView : pSRV);
        device->PSSetShaderResources(4, 1, occlussionMap ? &occlussionMap->textureView : pSRV);

        update_const_buffer(material->buffer, &material->data);
        device->PSSetConstantBuffers(1u, 1u, &material->buffer.buffer);

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
        device->IASetIndexBuffer(am.buffers[primitive.indexBuffer].asset.buffer, DXGI_FORMAT_R32_UINT, 0u);
        device->IASetVertexBuffers(0u, 1u,
            &am.buffers[primitive.vertexBuffer].asset.buffer,
            &pipeline.info.layout.size, &offset);

        // draw
        device->DrawIndexed(am.buffers[primitive.indexBuffer].asset.size / sizeof(u32), 0u, 0u);
    }
}

void
render_mesh_solid(mvAssetManager& am, mvMesh& mesh, mvMat4 transform, mvMat4 cam, mvMat4 proj)
{
    auto device = GContext->graphics.imDeviceContext;
    mvPipeline* pipeline = mvGetRawPipelineAsset(&am, "solid");

    set_pipeline_state(*pipeline);

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
        device->IASetIndexBuffer(am.buffers[primitive.indexBuffer].asset.buffer, DXGI_FORMAT_R32_UINT, 0u);
        device->IASetVertexBuffers(0u, 1u,
            &am.buffers[primitive.vertexBuffer].asset.buffer,
            &pipeline->info.layout.size, &offset);

        // draw
        device->DrawIndexed(am.buffers[primitive.indexBuffer].asset.size / sizeof(u32), 0u, 0u);
    }
}

void
render_mesh_wireframe(mvAssetManager& am, mvMesh& mesh, mvMat4 transform, mvMat4 cam, mvMat4 proj)
{
    auto device = GContext->graphics.imDeviceContext;
    mvPipeline* pipeline = mvGetRawPipelineAsset(&am, "solid_wireframe");

    set_pipeline_state(*pipeline);

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
        device->IASetIndexBuffer(am.buffers[primitive.indexBuffer].asset.buffer, DXGI_FORMAT_R32_UINT, 0u);
        device->IASetVertexBuffers(0u, 1u,
            &am.buffers[primitive.vertexBuffer].asset.buffer,
            &pipeline->info.layout.size, &offset);

        // draw
        device->DrawIndexed(am.buffers[primitive.indexBuffer].asset.size / sizeof(u32), 0u, 0u);
    }
}

void
render_mesh_shadow(mvAssetManager& am, mvMesh& mesh, mvMat4 transform, mvMat4 cam, mvMat4 proj)
{

    auto device = GContext->graphics.imDeviceContext;

    for (u32 i = 0; i < mesh.primitives.size(); i++)
    {
        mvMeshPrimitive& primitive = mesh.primitives[i];
        mvMaterial* material = &am.materials[primitive.materialID].asset;
        mvTexture* albedoMap = primitive.albedoTexture == -1 ? nullptr : &am.textures[primitive.albedoTexture].asset;

        mvPipeline* pipeline = material->data.alphaMode == 0 ? mvGetRawPipelineAsset(&am, "shadow_no_alpha") : mvGetRawPipelineAsset(&am, "shadow_alpha");


        // pipeline
        device->IASetPrimitiveTopology(pipeline->topology);
        device->OMSetBlendState(pipeline->blendState, nullptr, 0xFFFFFFFFu);
        device->OMSetDepthStencilState(pipeline->depthStencilState, 0xFF);;
        device->IASetInputLayout(pipeline->inputLayout);
        device->VSSetShader(pipeline->vertexShader, nullptr, 0);
        device->PSSetShader(pipeline->pixelShader, nullptr, 0);
        device->HSSetShader(nullptr, nullptr, 0);
        device->DSSetShader(nullptr, nullptr, 0);
        device->GSSetShader(nullptr, nullptr, 0);
        static ID3D11SamplerState* emptySamplers = nullptr;
        device->PSSetSamplers(0, 1, albedoMap ? &albedoMap->sampler : &emptySamplers);

        // material
        ID3D11ShaderResourceView* const pSRV[1] = { NULL };
        device->PSSetShaderResources(0, 1, albedoMap ? &albedoMap->textureView : pSRV);

        update_const_buffer(material->buffer, &material->data);
        GContext->graphics.imDeviceContext->PSSetConstantBuffers(1u, 1u, &material->buffer.buffer);

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
        device->IASetIndexBuffer(am.buffers[primitive.indexBuffer].asset.buffer, DXGI_FORMAT_R32_UINT, 0u);
        device->IASetVertexBuffers(0u, 1u,
            &am.buffers[primitive.vertexBuffer].asset.buffer,
            &pipeline->info.layout.size, &offset);

        // draw
        device->DrawIndexed(am.buffers[primitive.indexBuffer].asset.size / sizeof(u32), 0u, 0u);
    }
}

static void
mvRenderNode(mvAssetManager& am, mvNode& node, mvMat4 accumulatedTransform, mvMat4 cam, mvMat4 proj)
{

    if (node.mesh > -1 && node.camera == -1)
        render_mesh(am, am.meshes[node.mesh].asset, accumulatedTransform * node.matrix, cam, proj);
    else if (node.camera > -1)
    {
        render_mesh_wireframe(am, am.meshes[node.mesh].asset, accumulatedTransform * node.matrix * scale(identity_mat4(), { -1.0f, -1.0f, -1.0f }), cam, proj);
    }

    for (u32 i = 0; i < node.childCount; i++)
    {
        mvRenderNode(am, am.nodes[node.children[i]].asset, accumulatedTransform * node.matrix, cam, proj);
    }
}

void
render_scene(mvAssetManager& am, mvScene& scene, mvMat4 cam, mvMat4 proj, mvMat4 scaleM, mvMat4 trans)
{
    // extremely dumb method of handling sorting for transparency. Needs to be fixed

    // opaque objects
    for (u32 i = 0; i < scene.nodeCount; i++)
    {
        mvNode& rootNode = am.nodes[scene.nodes[i]].asset;

        if (rootNode.mesh > -1 && rootNode.camera == -1)
            render_mesh(am, am.meshes[rootNode.mesh].asset, trans * rootNode.matrix * scaleM, cam, proj);
        else if (rootNode.camera > -1)
        {
            render_mesh_wireframe(am, am.meshes[rootNode.mesh].asset, trans * rootNode.matrix * scale(identity_mat4(), { -1.0f, -1.0f, -1.0f }), cam, proj);
        }

        for (u32 j = 0; j < rootNode.childCount; j++)
        {
            mvRenderNode(am, am.nodes[rootNode.children[j]].asset, trans * rootNode.matrix * scaleM, cam, proj);
        }
    }

    // transparent objects
    for (u32 i = 0; i < scene.nodeCount; i++)
    {
        mvNode& rootNode = am.nodes[scene.nodes[i]].asset;

        if (rootNode.mesh > -1 && rootNode.camera == -1)
            render_mesh(am, am.meshes[rootNode.mesh].asset, trans * rootNode.matrix * scaleM, cam, proj);

        for (u32 j = 0; j < rootNode.childCount; j++)
        {
            mvRenderNode(am, am.nodes[rootNode.children[j]].asset, trans * rootNode.matrix * scaleM, cam, proj);
        }
    }
}

static void
mvRenderNodeShadows(mvAssetManager& am, mvNode& node, mvMat4 accumulatedTransform, mvMat4 cam, mvMat4 proj)
{

    if (node.mesh > -1 && node.camera == -1)
        render_mesh_shadow(am, am.meshes[node.mesh].asset, accumulatedTransform * node.matrix, cam, proj);

    for (u32 i = 0; i < node.childCount; i++)
    {
        mvRenderNodeShadows(am, am.nodes[node.children[i]].asset, accumulatedTransform * node.matrix, cam, proj);
    }
}

void
render_scene_shadows(mvAssetManager& am, mvScene& scene, mvMat4 cam, mvMat4 proj, mvMat4 scale, mvMat4 trans)
{
    for (u32 i = 0; i < scene.nodeCount; i++)
    {
        mvNode& rootNode = am.nodes[scene.nodes[i]].asset;

        if (rootNode.mesh > -1 && rootNode.camera == -1)
            render_mesh_shadow(am, am.meshes[rootNode.mesh].asset, trans * rootNode.matrix * scale, cam, proj);

        for (u32 j = 0; j < rootNode.childCount; j++)
        {
            mvRenderNodeShadows(am, am.nodes[rootNode.children[j]].asset, trans *rootNode.matrix * scale, cam, proj);
        }
    }
}

void 
render_skybox(mvAssetManager& am, mvCubeTexture& cubemap, ID3D11SamplerState* sampler, mvMat4 cam, mvMat4 proj)
{

    static mvAssetID pipelineID = mvGetPipelineAssetID(&am, "skybox_pipeline");
    static mvAssetID indexBufferID = mvGetBufferAssetID(&am, "skybox_ibuffer", nullptr, 0u, D3D11_BIND_INDEX_BUFFER);
    static mvAssetID vertexBufferID = mvGetBufferAssetID(&am, "skybox_vbuffer", nullptr, 0u, D3D11_BIND_VERTEX_BUFFER);
    static mvVertexLayout vertexLayout = create_vertex_layout({ mvVertexElement::Position3D });

    auto ctx = GContext->graphics.imDeviceContext;

    ctx->PSSetSamplers(0, 1, &sampler);
    ctx->PSSetShaderResources(0, 1, &cubemap.textureView);

    // pipeline
    set_pipeline_state(am.pipelines[pipelineID].asset);

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
    ctx->IASetIndexBuffer(am.buffers[indexBufferID].asset.buffer, DXGI_FORMAT_R32_UINT, 0u);
    ctx->IASetVertexBuffers(0u, 1u, &am.buffers[vertexBufferID].asset.buffer, &vertexLayout.size, &offset);

    // draw
    ctx->DrawIndexed(am.buffers[indexBufferID].asset.size / sizeof(u32), 0u, 0u);
}

}