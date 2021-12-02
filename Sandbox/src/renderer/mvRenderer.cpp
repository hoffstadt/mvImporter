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
mvRenderMesh(mvAssetManager& am, mvMesh& mesh, mvMat4 transform, mvMat4 cam, mvMat4 proj)
{

    auto device = GContext->graphics.imDeviceContext;

    // if material not assigned, get material
    if (mesh.materialID == -1)
    {
        assert(false && "material not assigned");
    }

    mvMaterial* material = &am.materials[mesh.materialID].material;
    mvTexture* albedoMap = mesh.albedoTexture == -1 ? nullptr : &am.textures[mesh.albedoTexture].texture;
    mvTexture* normMap = mesh.normalTexture == -1 ? nullptr : &am.textures[mesh.normalTexture].texture;
    mvTexture* metalRoughMap = mesh.metalRoughnessTexture == -1 ? nullptr : &am.textures[mesh.metalRoughnessTexture].texture;

    if (material->pipeline.info.layout != mesh.layout)
    {
        assert(false && "Mesh and material vertex layouts don't match.");
        return;
    }

    // pipeline
    mvSetPipelineState(material->pipeline);
    device->PSSetSamplers(0, 1, material->colorSampler.state.GetAddressOf());

    // maps
    ID3D11ShaderResourceView* const pSRV[1] = { NULL };
    device->PSSetShaderResources(0, 1, albedoMap ? albedoMap->textureView.GetAddressOf() : pSRV);
    device->PSSetShaderResources(1, 1, normMap ? normMap->textureView.GetAddressOf() : pSRV);
    device->PSSetShaderResources(2, 1, metalRoughMap ? metalRoughMap->textureView.GetAddressOf() : pSRV);

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
    device->IASetIndexBuffer(am.buffers[mesh.indexBuffer].buffer.buffer, DXGI_FORMAT_R32_UINT, 0u);
    device->IASetVertexBuffers(0u, 1u,
        &am.buffers[mesh.vertexBuffer].buffer.buffer,
        &material->pipeline.info.layout.size, &offset);

    // draw
    device->DrawIndexed(am.buffers[mesh.indexBuffer].buffer.size / sizeof(u32), 0u, 0u);
}

void
mvRenderMeshShadows(mvAssetManager& am, mvMesh& mesh, mvMat4 transform, mvMat4 cam, mvMat4 proj)
{

    auto device = GContext->graphics.imDeviceContext;

    mvMaterial* material = &am.materials[mesh.materialID].material;
    mvTexture* albedoMap = mesh.albedoTexture == -1 ? nullptr : &am.textures[mesh.albedoTexture].texture;

    if (material->pipeline.info.layout != mesh.layout)
    {
        assert(false && "Mesh and material vertex layouts don't match.");
        return;
    }

    // pipeline
    device->IASetPrimitiveTopology(material->pipeline.topology);
    device->OMSetBlendState(material->pipeline.blendState, nullptr, 0xFFFFFFFFu);
    device->OMSetDepthStencilState(material->pipeline.depthStencilState, 0xFF);;
    device->IASetInputLayout(material->pipeline.inputLayout);
    device->VSSetShader(material->pipeline.vertexShader, nullptr, 0);
    device->PSSetShader(material->data.hasAlpha ? material->pipeline.pixelShader : nullptr, nullptr, 0);
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
    device->IASetIndexBuffer(am.buffers[mesh.indexBuffer].buffer.buffer, DXGI_FORMAT_R32_UINT, 0u);
    device->IASetVertexBuffers(0u, 1u,
        &am.buffers[mesh.vertexBuffer].buffer.buffer,
        &material->pipeline.info.layout.size, &offset);

    // draw
    device->DrawIndexed(am.buffers[mesh.indexBuffer].buffer.size / sizeof(u32), 0u, 0u);
}

static void
mvRenderNode(mvAssetManager& am, mvNode& node, mvMat4 accumulatedTransform, mvMat4 cam, mvMat4 proj)
{

    if (node.mesh > -1)
        mvRenderMesh(am, am.meshes[node.mesh].mesh, accumulatedTransform * node.matrix, cam, proj);

    for (u32 i = 0; i < node.childCount; i++)
    {
        mvRenderNode(am, am.nodes[node.children[i]].node, accumulatedTransform* node.matrix, cam, proj);
    }
}

void
mvRenderScene(mvAssetManager& am, mvScene& scene, mvMat4 cam, mvMat4 proj)
{
    for (u32 i = 0; i < scene.nodeCount; i++)
    {
        mvNode& rootNode = am.nodes[scene.nodes[i]].node;

        if (rootNode.mesh > -1)
            mvRenderMesh(am, am.meshes[rootNode.mesh].mesh, rootNode.matrix, cam, proj);

        for (u32 j = 0; j < rootNode.childCount; j++)
        {
            mvRenderNode(am, am.nodes[rootNode.children[j]].node, rootNode.matrix, cam, proj);
        }
    }
}

static void
mvRenderNodeShadows(mvAssetManager& am, mvNode& node, mvMat4 accumulatedTransform, mvMat4 cam, mvMat4 proj)
{

    if (node.mesh > -1)
        mvRenderMeshShadows(am, am.meshes[node.mesh].mesh, accumulatedTransform * node.matrix, cam, proj);

    for (u32 i = 0; i < node.childCount; i++)
    {
        mvRenderNodeShadows(am, am.nodes[node.children[i]].node, accumulatedTransform * node.matrix, cam, proj);
    }
}

void
mvRenderSceneShadows(mvAssetManager& am, mvScene& scene, mvMat4 cam, mvMat4 proj)
{
    for (u32 i = 0; i < scene.nodeCount; i++)
    {
        mvNode& rootNode = am.nodes[scene.nodes[i]].node;

        if (rootNode.mesh > -1)
            mvRenderMeshShadows(am, am.meshes[rootNode.mesh].mesh, rootNode.matrix, cam, proj);

        for (u32 j = 0; j < rootNode.childCount; j++)
        {
            mvRenderNodeShadows(am, am.nodes[rootNode.children[j]].node, rootNode.matrix, cam, proj);
        }
    }
}

}