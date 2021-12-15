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

        pipelineInfo.layout = mvCreateVertexLayout(
            {
                mvVertexElement::Position3D,
                mvVertexElement::Normal,
                mvVertexElement::Texture2D,
                mvVertexElement::Tangent,
                mvVertexElement::Bitangent
            }
        );

        mvRegisterAsset(&am, "shadow_alpha", mvFinalizePipeline(pipelineInfo));
    }

    {
        mvPipelineInfo pipelineInfo{};
        pipelineInfo.pixelShader = "";
        pipelineInfo.vertexShader = "Shadow_VS.hlsl";
        pipelineInfo.depthBias = 50;
        pipelineInfo.slopeBias = 2.0f;
        pipelineInfo.clamp = 0.1f;
        pipelineInfo.cull = false;

        pipelineInfo.layout = mvCreateVertexLayout(
            {
                mvVertexElement::Position3D,
                mvVertexElement::Normal,
                mvVertexElement::Texture2D,
                mvVertexElement::Tangent,
                mvVertexElement::Bitangent
            }
        );

        mvRegisterAsset(&am, "shadow_no_alpha", mvFinalizePipeline(pipelineInfo));
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

        mvPixelShader pixelShader = mvCreatePixelShader(GContext->IO.shaderDirectory + "Solid_PS.hlsl");
        mvVertexShader vertexShader = mvCreateVertexShader(GContext->IO.shaderDirectory + "Solid_VS.hlsl", mvCreateVertexLayout({ mvVertexElement::Position3D }));

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

        pipeline.info.layout = mvCreateVertexLayout(
            {
                mvVertexElement::Position3D
            }
        );

        mvRegisterAsset(&am, "solid_wireframe", pipeline);
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

        mvPixelShader pixelShader = mvCreatePixelShader(GContext->IO.shaderDirectory + "Solid_PS.hlsl");
        mvVertexShader vertexShader = mvCreateVertexShader(GContext->IO.shaderDirectory + "Solid_VS.hlsl", mvCreateVertexLayout({ mvVertexElement::Position3D }));

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

        pipeline.info.layout = mvCreateVertexLayout(
            {
                mvVertexElement::Position3D
            }
        );

        mvRegisterAsset(&am, "solid", pipeline);
    }
}

void
mvRenderMesh(mvAssetManager& am, mvMesh& mesh, mvMat4 transform, mvMat4 cam, mvMat4 proj)
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
        mvSetPipelineState(pipeline);
        device->PSSetSamplers(0, 1, material->colorSampler.state.GetAddressOf());

        // maps
        ID3D11ShaderResourceView* const pSRV[1] = { NULL };
        device->PSSetShaderResources(0, 1, albedoMap ? albedoMap->textureView.GetAddressOf() : pSRV);
        device->PSSetShaderResources(1, 1, normMap ? normMap->textureView.GetAddressOf() : pSRV);
        device->PSSetShaderResources(2, 1, metalRoughMap ? metalRoughMap->textureView.GetAddressOf() : pSRV);
        device->PSSetShaderResources(3, 1, emissiveMap ? emissiveMap->textureView.GetAddressOf() : pSRV);
        device->PSSetShaderResources(4, 1, occlussionMap ? occlussionMap->textureView.GetAddressOf() : pSRV);

        mvUpdateConstBuffer(material->buffer, &material->data);
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
mvRenderMeshSolid(mvAssetManager& am, mvMesh& mesh, mvMat4 transform, mvMat4 cam, mvMat4 proj)
{
    auto device = GContext->graphics.imDeviceContext;
    mvPipeline* pipeline = mvGetRawPipelineAsset(&am, "solid");

    mvSetPipelineState(*pipeline);

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
mvRenderMeshSolidWireframe(mvAssetManager& am, mvMesh& mesh, mvMat4 transform, mvMat4 cam, mvMat4 proj)
{
    auto device = GContext->graphics.imDeviceContext;
    mvPipeline* pipeline = mvGetRawPipelineAsset(&am, "solid_wireframe");

    mvSetPipelineState(*pipeline);

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
mvRenderMeshShadows(mvAssetManager& am, mvMesh& mesh, mvMat4 transform, mvMat4 cam, mvMat4 proj)
{

    auto device = GContext->graphics.imDeviceContext;

    for (u32 i = 0; i < mesh.primitives.size(); i++)
    {
        mvMeshPrimitive& primitive = mesh.primitives[i];
        mvMaterial* material = &am.materials[primitive.materialID].asset;
        mvTexture* albedoMap = primitive.albedoTexture == -1 ? nullptr : &am.textures[primitive.albedoTexture].asset;

        mvPipeline* pipeline = material->data.hasAlpha ? mvGetRawPipelineAsset(&am, "shadow_alpha") : mvGetRawPipelineAsset(&am, "shadow_no_alpha");


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
        device->PSSetSamplers(0, 1, material->colorSampler.state.GetAddressOf());

        // material
        ID3D11ShaderResourceView* const pSRV[1] = { NULL };
        device->PSSetShaderResources(0, 1, albedoMap ? albedoMap->textureView.GetAddressOf() : pSRV);

        mvUpdateConstBuffer(material->buffer, &material->data);
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
        mvRenderMesh(am, am.meshes[node.mesh].asset, accumulatedTransform * node.matrix, cam, proj);
    else if (node.camera > -1)
    {
        mvRenderMeshSolidWireframe(am, am.meshes[node.mesh].asset, accumulatedTransform * node.matrix * mvScale(mvIdentityMat4(), { -1.0f, -1.0f, -1.0f }), cam, proj);
    }

    for (u32 i = 0; i < node.childCount; i++)
    {
        mvRenderNode(am, am.nodes[node.children[i]].asset, accumulatedTransform * node.matrix, cam, proj);
    }
}

void
mvRenderScene(mvAssetManager& am, mvScene& scene, mvMat4 cam, mvMat4 proj, mvMat4 scale, mvMat4 trans)
{
    for (u32 i = 0; i < scene.nodeCount; i++)
    {
        mvNode& rootNode = am.nodes[scene.nodes[i]].asset;

        if (rootNode.mesh > -1 && rootNode.camera == -1)
            mvRenderMesh(am, am.meshes[rootNode.mesh].asset, trans * rootNode.matrix * scale, cam, proj);
        else if (rootNode.camera > -1)
        {
            mvRenderMeshSolidWireframe(am, am.meshes[rootNode.mesh].asset, trans * rootNode.matrix * mvScale(mvIdentityMat4(), { -1.0f, -1.0f, -1.0f }), cam, proj);
        }

        for (u32 j = 0; j < rootNode.childCount; j++)
        {
            mvRenderNode(am, am.nodes[rootNode.children[j]].asset, trans * rootNode.matrix * scale, cam, proj);
        }
    }
}

static void
mvRenderNodeShadows(mvAssetManager& am, mvNode& node, mvMat4 accumulatedTransform, mvMat4 cam, mvMat4 proj)
{

    if (node.mesh > -1 && node.camera == -1)
        mvRenderMeshShadows(am, am.meshes[node.mesh].asset, accumulatedTransform * node.matrix, cam, proj);

    for (u32 i = 0; i < node.childCount; i++)
    {
        mvRenderNodeShadows(am, am.nodes[node.children[i]].asset, accumulatedTransform * node.matrix, cam, proj);
    }
}

void
mvRenderSceneShadows(mvAssetManager& am, mvScene& scene, mvMat4 cam, mvMat4 proj, mvMat4 scale, mvMat4 trans)
{
    for (u32 i = 0; i < scene.nodeCount; i++)
    {
        mvNode& rootNode = am.nodes[scene.nodes[i]].asset;

        if (rootNode.mesh > -1 && rootNode.camera == -1)
            mvRenderMeshShadows(am, am.meshes[rootNode.mesh].asset, trans * rootNode.matrix * scale, cam, proj);

        for (u32 j = 0; j < rootNode.childCount; j++)
        {
            mvRenderNodeShadows(am, am.nodes[rootNode.children[j]].asset, trans *rootNode.matrix * scale, cam, proj);
        }
    }
}

}