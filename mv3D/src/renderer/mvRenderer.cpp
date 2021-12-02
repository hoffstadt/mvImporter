#include "mvRenderer.h"
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>
#include <implot.h>
#include "mv3D_internal.h"
#include "mvGraphics.h"
#include "mvBuffers.h"
#include "mvVertexLayout.h"
#include "mvAssetManager.h"

namespace Renderer{

mv_internal void
mvRenderMeshPhong(mvAssetManager& am, mvMesh& mesh, mvMat4 transform, mvMat4 cam, mvMat4 proj)
{

    auto device = GContext->graphics.imDeviceContext;

    // if material not assigned, get material
    if (mesh.phongMaterialID == -1)
    {
        mesh.phongMaterialID = mvGetPhongMaterialAsset(&am,
            "Phong_VS.hlsl", "Phong_PS.hlsl", 
            mesh.diffuseTexture == -1, mesh.diffuseTexture > -1, mesh.normalTexture > -1, mesh.specularTexture > -1);
    }

    mvPhongMaterial* material = &am.phongMaterials[mesh.phongMaterialID].material;
    mvTexture* diffMap = mesh.diffuseTexture == -1 ? nullptr : &am.textures[mesh.diffuseTexture].texture;
    mvTexture* normMap = mesh.normalTexture == -1 ? nullptr : &am.textures[mesh.normalTexture].texture;
    mvTexture* specMap = mesh.specularTexture == -1 ? nullptr : &am.textures[mesh.specularTexture].texture;

    if (material->pipeline.info.layout != mesh.layout)
    {
        assert(false && "Mesh and material vertex layouts don't match.");
        return;
    }

    // pipeline
    device->IASetPrimitiveTopology(material->pipeline.topology);
    device->RSSetState(material->pipeline.rasterizationState.Get());
    device->OMSetBlendState(material->pipeline.blendState.Get(), nullptr, 0xFFFFFFFFu);
    device->OMSetDepthStencilState(material->pipeline.depthStencilState.Get(), 0xFF);;
    device->IASetInputLayout(material->pipeline.inputLayout.Get());
    device->VSSetShader(material->pipeline.vertexShader.Get(), nullptr, 0);
    device->PSSetShader(material->pipeline.pixelShader.Get(), nullptr, 0);
    device->HSSetShader(nullptr, nullptr, 0);
    device->DSSetShader(nullptr, nullptr, 0);
    device->GSSetShader(nullptr, nullptr, 0);
    device->PSSetSamplers(0, 1, material->colorSampler.state.GetAddressOf());

    // maps
    ID3D11ShaderResourceView* const pSRV[1] = { NULL };
    device->PSSetShaderResources(0, 1, diffMap ? diffMap->textureView.GetAddressOf() : pSRV);
    device->PSSetShaderResources(1, 1, specMap ? specMap->textureView.GetAddressOf() : pSRV);
    device->PSSetShaderResources(2, 1, normMap ? normMap->textureView.GetAddressOf() : pSRV);

    mvUpdateConstBuffer(material->buffer, &material->data);
    device->PSSetConstantBuffers(1u, 1u, material->buffer.buffer.GetAddressOf());

    mvTransforms transforms{};
    transforms.model = transform;
    transforms.modelView = cam * transforms.model;
    transforms.modelViewProjection = proj * cam * transforms.model;

    D3D11_MAPPED_SUBRESOURCE mappedSubresource;
    device->Map(GContext->graphics.tranformCBuf.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &mappedSubresource);
    memcpy(mappedSubresource.pData, &transforms, sizeof(mvTransforms));
    device->Unmap(GContext->graphics.tranformCBuf.Get(), 0u);

    // mesh
    mv_local_persist const UINT offset = 0u;
    device->VSSetConstantBuffers(0u, 1u, GContext->graphics.tranformCBuf.GetAddressOf());
    device->IASetIndexBuffer(am.buffers[mesh.indexBuffer].buffer.buffer, DXGI_FORMAT_R32_UINT, 0u);
    device->IASetVertexBuffers(0u, 1u,
        &am.buffers[mesh.vertexBuffer].buffer.buffer,
        &material->pipeline.info.layout.size, &offset);

    // draw
    device->DrawIndexed(am.buffers[mesh.indexBuffer].buffer.size / sizeof(u32), 0u, 0u);
}

mv_internal void
mvRenderMeshPBR(mvAssetManager& am, mvMesh& mesh, mvMat4 transform, mvMat4 cam, mvMat4 proj)
{

    auto device = GContext->graphics.imDeviceContext;

    // if material not assigned, get material
    if (mesh.pbrMaterialID == -1)
    {
        assert(false && "material not assigned");
    }

    mvPBRMaterial* material = &am.pbrMaterials[mesh.pbrMaterialID].material;
    mvTexture* albedoMap = mesh.albedoTexture == -1 ? nullptr : &am.textures[mesh.albedoTexture].texture;
    mvTexture* normMap = mesh.normalTexture == -1 ? nullptr : &am.textures[mesh.normalTexture].texture;
    mvTexture* metalRoughMap = mesh.metalRoughnessTexture == -1 ? nullptr : &am.textures[mesh.metalRoughnessTexture].texture;

    if (material->pipeline.info.layout != mesh.layout)
    {
        assert(false && "Mesh and material vertex layouts don't match.");
        return;
    }

    // pipeline
    device->IASetPrimitiveTopology(material->pipeline.topology);
    device->RSSetState(material->pipeline.rasterizationState.Get());
    device->OMSetBlendState(material->pipeline.blendState.Get(), nullptr, 0xFFFFFFFFu);
    device->OMSetDepthStencilState(material->pipeline.depthStencilState.Get(), 0xFF);;
    device->IASetInputLayout(material->pipeline.inputLayout.Get());
    device->VSSetShader(material->pipeline.vertexShader.Get(), nullptr, 0);
    device->PSSetShader(material->pipeline.pixelShader.Get(), nullptr, 0);
    device->HSSetShader(nullptr, nullptr, 0);
    device->DSSetShader(nullptr, nullptr, 0);
    device->GSSetShader(nullptr, nullptr, 0);
    device->PSSetSamplers(0, 1, material->colorSampler.state.GetAddressOf());

    // maps
    ID3D11ShaderResourceView* const pSRV[1] = { NULL };
    device->PSSetShaderResources(0, 1, albedoMap ? albedoMap->textureView.GetAddressOf() : pSRV);
    device->PSSetShaderResources(1, 1, normMap ? normMap->textureView.GetAddressOf() : pSRV);
    device->PSSetShaderResources(2, 1, metalRoughMap ? metalRoughMap->textureView.GetAddressOf() : pSRV);

    mvUpdateConstBuffer(material->buffer, &material->data);
    device->PSSetConstantBuffers(1u, 1u, material->buffer.buffer.GetAddressOf());

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
mvRenderMesh(mvAssetManager& am, mvMesh& mesh, mvMat4 transform, mvMat4 cam, mvMat4 proj)
{
    if (mesh.pbr)
        mvRenderMeshPBR(am, mesh, transform, cam, proj);
    else
        mvRenderMeshPhong(am, mesh, transform, cam, proj);
}

void
mvRenderMeshShadows(mvAssetManager& am, mvMesh& mesh, mvMat4 transform, mvMat4 cam, mvMat4 proj)
{

    // if material not assigned, get material
    if (mesh.shadowMaterialID == -1)
    {
        mesh.shadowMaterialID = mvGetPhongMaterialAsset(&am,
            "PhongShadow_VS.hlsl", "PhongShadow_PS.hlsl",
            mesh.diffuseTexture == -1, mesh.diffuseTexture > -1, false, false);
    }

    auto device = GContext->graphics.imDeviceContext;

    mvPhongMaterial* material = &am.phongMaterials[mesh.shadowMaterialID].material;
    mvTexture* diffMap = mesh.diffuseTexture == -1 ? nullptr : &am.textures[mesh.diffuseTexture].texture;
    mvTexture* albedoMap = mesh.albedoTexture == -1 ? nullptr : &am.textures[mesh.albedoTexture].texture;

    if (material->pipeline.info.layout != mesh.layout)
    {
        assert(false && "Mesh and material vertex layouts don't match.");
        return;
    }

    // pipeline
    device->IASetPrimitiveTopology(material->pipeline.topology);
    device->OMSetBlendState(material->pipeline.blendState.Get(), nullptr, 0xFFFFFFFFu);
    device->OMSetDepthStencilState(material->pipeline.depthStencilState.Get(), 0xFF);;
    device->IASetInputLayout(material->pipeline.inputLayout.Get());
    device->VSSetShader(material->pipeline.vertexShader.Get(), nullptr, 0);
    device->PSSetShader(material->data.hasAlpha ? material->pipeline.pixelShader.Get() : nullptr, nullptr, 0);
    device->HSSetShader(nullptr, nullptr, 0);
    device->DSSetShader(nullptr, nullptr, 0);
    device->GSSetShader(nullptr, nullptr, 0);
    device->PSSetSamplers(0, 1, material->colorSampler.state.GetAddressOf());

    // material
    ID3D11ShaderResourceView* const pSRV[1] = { NULL };
    if(mesh.pbr)
        device->PSSetShaderResources(0, 1, albedoMap ? albedoMap->textureView.GetAddressOf() : pSRV);
    else
        device->PSSetShaderResources(0, 1, diffMap ? diffMap->textureView.GetAddressOf() : pSRV);

    mvUpdateConstBuffer(material->buffer, &material->data);
    GContext->graphics.imDeviceContext->PSSetConstantBuffers(1u, 1u, material->buffer.buffer.GetAddressOf());

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

mv_internal void
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

mv_internal void
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