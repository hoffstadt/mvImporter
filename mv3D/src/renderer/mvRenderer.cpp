#include "mvRenderer.h"
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>
#include <implot.h>
#include "mv3D_internal.h"
#include "mvGraphics.h"
#include "mvBuffers.h"
#include "mvVertexLayout.h"
#include "mvAssetManager.h"

void
mvRenderer_Resize()
{
    mvRecreateSwapChain();
}

void
mvRenderer_StartRenderer()
{
    mvInitializeViewport(1850, 900);
    mvSetupGraphics();

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.IniFilename = nullptr;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking

    ImGui::StyleColorsDark();

    ImGui_ImplWin32_Init(GContext->viewport.hWnd);
    ImGui_ImplDX11_Init(GContext->graphics.device.Get(), GContext->graphics.imDeviceContext.Get());
}

void
mvRenderer_StopRenderer()
{
    // Cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    mvCleanupGraphics();
}

void
mvRenderer_BeginFrame()
{
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ID3D11ShaderResourceView* const pSRV[6] = { NULL };
    GContext->graphics.imDeviceContext->PSSetShaderResources(0, 1, pSRV);
    GContext->graphics.imDeviceContext->PSSetShaderResources(1, 1, pSRV);
    GContext->graphics.imDeviceContext->PSSetShaderResources(2, 1, pSRV);
    GContext->graphics.imDeviceContext->PSSetShaderResources(3, 6, pSRV); // depth map
    GContext->graphics.imDeviceContext->PSSetShaderResources(4, 6, pSRV); // depth map
    GContext->graphics.imDeviceContext->PSSetShaderResources(5, 6, pSRV); // depth map
    GContext->graphics.imDeviceContext->PSSetShaderResources(6, 1, pSRV); // depth map

    ImGui::GetForegroundDrawList()->AddText(ImVec2(45, 45),
        ImColor(0.0f, 1.0f, 0.0f), std::string(std::to_string(ImGui::GetIO().Framerate) + " FPS").c_str());
}

void
mvRenderer_EndFrame()
{
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void
mvRenderer_Present()
{
    GContext->graphics.swapChain->Present(1, 0);
}

void
mvRenderer_ClearPass(mvPass& pass)
{
    // clear render target
    mv_local_persist float backgroundColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    if (pass.target)
        GContext->graphics.imDeviceContext->ClearRenderTargetView(*pass.target, backgroundColor);
    if(pass.depthStencil)
        GContext->graphics.imDeviceContext->ClearDepthStencilView(*pass.depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0u);
}

void
mvRenderer_BeginPass(mvPass& pass)
{
    if(pass.depthStencil && pass.target)
        GContext->graphics.imDeviceContext->OMSetRenderTargets(1, pass.target, *pass.depthStencil);
    else if(pass.depthStencil)
        GContext->graphics.imDeviceContext->OMSetRenderTargets(0, nullptr, *pass.depthStencil);
    else 
        GContext->graphics.imDeviceContext->OMSetRenderTargets(1, pass.target, nullptr);

    GContext->graphics.imDeviceContext->RSSetViewports(1u, &pass.viewport);
    
    if(pass.rasterizationState)
        GContext->graphics.imDeviceContext->RSSetState(*pass.rasterizationState);
}

void
mvRenderer_EndPass()
{

}

mv_internal void
mvRenderer_RenderMeshPhong(mvAssetManager& am, mvMesh& mesh, mvMat4 transform, mvMat4 cam, mvMat4 proj)
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
    transforms.model = transform * mvTranslate(mvIdentityMat4(), mesh.pos) *
        mvRotate(mvIdentityMat4(), mesh.rot.x, mvVec3{ 1.0f, 0.0f, 0.0f }) *
        mvRotate(mvIdentityMat4(), mesh.rot.y, mvVec3{ 0.0f, 1.0f, 0.0f }) *
        mvRotate(mvIdentityMat4(), mesh.rot.z, mvVec3{ 0.0f, 0.0f, 1.0f });
    transforms.model = transforms.model * mvScale(mvIdentityMat4(), mesh.scale);
    transforms.modelView = cam * transforms.model;
    transforms.modelViewProjection = proj * cam * transforms.model;

    D3D11_MAPPED_SUBRESOURCE mappedSubresource;
    device->Map(GContext->graphics.tranformCBuf.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &mappedSubresource);
    memcpy(mappedSubresource.pData, &transforms, sizeof(mvTransforms));
    device->Unmap(GContext->graphics.tranformCBuf.Get(), 0u);

    // mesh
    mv_local_persist const UINT offset = 0u;
    device->VSSetConstantBuffers(0u, 1u, GContext->graphics.tranformCBuf.GetAddressOf());
    device->IASetIndexBuffer(am.buffers[mesh.indexBuffer].buffer.buffer.Get(), DXGI_FORMAT_R32_UINT, 0u);
    device->IASetVertexBuffers(0u, 1u,
        am.buffers[mesh.vertexBuffer].buffer.buffer.GetAddressOf(),
        &material->pipeline.info.layout.size, &offset);

    // draw
    device->DrawIndexed(am.buffers[mesh.indexBuffer].buffer.size / sizeof(u32), 0u, 0u);
}

mv_internal void
mvRenderer_RenderMeshPBR(mvAssetManager& am, mvMesh& mesh, mvMat4 transform, mvMat4 cam, mvMat4 proj)
{

    auto device = GContext->graphics.imDeviceContext;

    // if material not assigned, get material
    if (mesh.pbrMaterialID == -1)
    {
        mesh.pbrMaterialID = mvGetPBRMaterialAsset(&am,
            "Phong_VS.hlsl", "PBR_PS.hlsl",
            mesh.albedoTexture == -1, 
            mesh.albedoTexture > -1, 
            mesh.normalTexture > -1, 
            mesh.metalRoughnessTexture > -1,
            mesh.metalRoughnessTexture > -1);
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
    transforms.model = transform*mvTranslate(mvIdentityMat4(), mesh.pos) *
        mvRotate(mvIdentityMat4(), mesh.rot.x, mvVec3{ 1.0f, 0.0f, 0.0f }) *
        mvRotate(mvIdentityMat4(), mesh.rot.y, mvVec3{ 0.0f, 1.0f, 0.0f }) *
        mvRotate(mvIdentityMat4(), mesh.rot.z, mvVec3{ 0.0f, 0.0f, 1.0f });
    transforms.model = transforms.model * mvScale(mvIdentityMat4(), mesh.scale);
    transforms.modelView = cam * transforms.model;
    transforms.modelViewProjection = proj * cam * transforms.model;

    D3D11_MAPPED_SUBRESOURCE mappedSubresource;
    device->Map(GContext->graphics.tranformCBuf.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &mappedSubresource);
    memcpy(mappedSubresource.pData, &transforms, sizeof(mvTransforms));
    device->Unmap(GContext->graphics.tranformCBuf.Get(), 0u);

    // mesh
    static const UINT offset = 0u;
    device->VSSetConstantBuffers(0u, 1u, GContext->graphics.tranformCBuf.GetAddressOf());
    device->IASetIndexBuffer(am.buffers[mesh.indexBuffer].buffer.buffer.Get(), DXGI_FORMAT_R32_UINT, 0u);
    device->IASetVertexBuffers(0u, 1u,
        am.buffers[mesh.vertexBuffer].buffer.buffer.GetAddressOf(),
        &material->pipeline.info.layout.size, &offset);

    // draw
    device->DrawIndexed(am.buffers[mesh.indexBuffer].buffer.size / sizeof(u32), 0u, 0u);
}

void
mvRenderer_RenderMesh(mvAssetManager& am, mvMesh& mesh, mvMat4 transform, mvMat4 cam, mvMat4 proj)
{
    if (mesh.pbr)
        mvRenderer_RenderMeshPBR(am, mesh, transform, cam, proj);
    else
        mvRenderer_RenderMeshPhong(am, mesh, transform, cam, proj);
}

void
mvRenderer_RenderMeshShadows(mvAssetManager& am, mvMesh& mesh, mvMat4 transform, mvMat4 cam, mvMat4 proj)
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
    transforms.model = transform * mvTranslate(mvIdentityMat4(), mesh.pos) *
        mvRotate(mvIdentityMat4(), mesh.rot.x, mvVec3{ 1.0f, 0.0f, 0.0f }) *
        mvRotate(mvIdentityMat4(), mesh.rot.y, mvVec3{ 0.0f, 1.0f, 0.0f }) *
        mvRotate(mvIdentityMat4(), mesh.rot.z, mvVec3{ 0.0f, 0.0f, 1.0f });
    transforms.model = transforms.model * mvScale(mvIdentityMat4(), mesh.scale);
    transforms.modelView = cam * transforms.model;
    transforms.modelViewProjection = proj * cam * transforms.model;

    D3D11_MAPPED_SUBRESOURCE mappedSubresource;
    device->Map(GContext->graphics.tranformCBuf.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &mappedSubresource);
    memcpy(mappedSubresource.pData, &transforms, sizeof(mvTransforms));
    device->Unmap(GContext->graphics.tranformCBuf.Get(), 0u);

    // mesh
    static const UINT offset = 0u;
    device->VSSetConstantBuffers(0u, 1u, GContext->graphics.tranformCBuf.GetAddressOf());
    device->IASetIndexBuffer(am.buffers[mesh.indexBuffer].buffer.buffer.Get(), DXGI_FORMAT_R32_UINT, 0u);
    device->IASetVertexBuffers(0u, 1u,
        am.buffers[mesh.vertexBuffer].buffer.buffer.GetAddressOf(),
        &material->pipeline.info.layout.size, &offset);

    // draw
    device->DrawIndexed(am.buffers[mesh.indexBuffer].buffer.size / sizeof(u32), 0u, 0u);
}

void
mvRenderer_RenderSkybox(mvAssetManager& am, mvSkyboxPass& skyboxPass, mvMat4 cam, mvMat4 proj)
{
    auto device = GContext->graphics.imDeviceContext;

    mvCubeTexture* texture = &am.cubeTextures[skyboxPass.cubeTexture].texture;
    mvMesh& mesh = am.meshes[skyboxPass.mesh].mesh;
    mvSampler& sampler = am.samplers[skyboxPass.sampler].sampler;

    // pipeline
    device->IASetPrimitiveTopology(skyboxPass.pipeline.topology);
    device->RSSetState(skyboxPass.pipeline.rasterizationState.Get());
    device->OMSetBlendState(skyboxPass.pipeline.blendState.Get(), nullptr, 0xFFFFFFFFu);
    device->OMSetDepthStencilState(skyboxPass.pipeline.depthStencilState.Get(), 0xFF);;
    device->IASetInputLayout(skyboxPass.pipeline.inputLayout.Get());
    device->VSSetShader(skyboxPass.pipeline.vertexShader.Get(), nullptr, 0);
    device->PSSetShader(skyboxPass.pipeline.pixelShader.Get(), nullptr, 0);
    device->HSSetShader(nullptr, nullptr, 0);
    device->DSSetShader(nullptr, nullptr, 0);
    device->GSSetShader(nullptr, nullptr, 0);
    device->PSSetSamplers(0, 1, sampler.state.GetAddressOf());
    device->PSSetShaderResources(0, 1, texture->textureView.GetAddressOf());

    mvTransforms transforms{};
    transforms.model = mvTranslate(mvIdentityMat4(), mesh.pos) *
        mvRotate(mvIdentityMat4(), mesh.rot.x, mvVec3{ 1.0f, 0.0f, 0.0f }) *
        mvRotate(mvIdentityMat4(), mesh.rot.y, mvVec3{ 0.0f, 1.0f, 0.0f }) *
        mvRotate(mvIdentityMat4(), mesh.rot.z, mvVec3{ 0.0f, 0.0f, 1.0f });
    transforms.model = transforms.model * mvScale(mvIdentityMat4(), mesh.scale);
    transforms.modelView = cam * transforms.model;
    transforms.modelViewProjection = proj * cam * transforms.model;

    D3D11_MAPPED_SUBRESOURCE mappedSubresource;
    device->Map(GContext->graphics.tranformCBuf.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &mappedSubresource);
    memcpy(mappedSubresource.pData, &transforms, sizeof(mvTransforms));
    device->Unmap(GContext->graphics.tranformCBuf.Get(), 0u);

    // mesh
    static const UINT offset = 0u;
    device->VSSetConstantBuffers(0u, 1u, GContext->graphics.tranformCBuf.GetAddressOf());
    device->IASetIndexBuffer(am.buffers[mesh.indexBuffer].buffer.buffer.Get(), DXGI_FORMAT_R32_UINT, 0u);
    device->IASetVertexBuffers(0u, 1u,
        am.buffers[mesh.vertexBuffer].buffer.buffer.GetAddressOf(),
        &skyboxPass.pipeline.info.layout.size, &offset);

    // draw
    device->DrawIndexed(am.buffers[mesh.indexBuffer].buffer.size / sizeof(u32), 0u, 0u);
}

mv_internal void
mvRenderer_RenderNode(mvAssetManager& am, mvNode& node, mvMat4 accumulatedTransform, mvMat4 cam, mvMat4 proj)
{
    mvMat4 transform = mvTranslate(mvIdentityMat4(), node.translation) *
        mvRotate(mvIdentityMat4(), 0.0f, mvVec3{ 1.0f, 0.0f, 0.0f }) *
        mvRotate(mvIdentityMat4(), 0.0f, mvVec3{ 0.0f, 1.0f, 0.0f }) *
        mvRotate(mvIdentityMat4(), 0.0f, mvVec3{ 0.0f, 0.0f, 1.0f }) * mvScale(mvIdentityMat4(), node.scale);

    if (node.mesh > -1)
        mvRenderer_RenderMesh(am, am.meshes[node.mesh].mesh, accumulatedTransform * transform, cam, proj);

    for (u32 i = 0; i < node.childCount; i++)
    {
        mvRenderer_RenderNode(am, am.nodes[node.children[i]].node, accumulatedTransform*transform, cam, proj);
    }
}

void
mvRenderer_RenderScene(mvAssetManager& am, mvScene& scene, mvMat4 cam, mvMat4 proj)
{
    for (u32 i = 0; i < scene.nodeCount; i++)
    {
        mvNode& rootNode = am.nodes[scene.nodes[i]].node;

        mvMat4 transform = mvTranslate(mvIdentityMat4(), rootNode.translation) *
            mvRotate(mvIdentityMat4(), 0.0f, mvVec3{ 1.0f, 0.0f, 0.0f }) *
            mvRotate(mvIdentityMat4(), 0.0f, mvVec3{ 0.0f, 1.0f, 0.0f }) *
            mvRotate(mvIdentityMat4(), 0.0f, mvVec3{ 0.0f, 0.0f, 1.0f }) * mvScale(mvIdentityMat4(), rootNode.scale);

        if (rootNode.mesh > -1)
            mvRenderer_RenderMesh(am, am.meshes[rootNode.mesh].mesh, transform, cam, proj);

        for (u32 j = 0; j < rootNode.childCount; j++)
        {
            mvRenderer_RenderNode(am, am.nodes[rootNode.children[j]].node, transform, cam, proj);
        }
    }
}

mv_internal void
mvRenderer_RenderNodeShadows(mvAssetManager& am, mvNode& node, mvMat4 accumulatedTransform, mvMat4 cam, mvMat4 proj)
{
    mvMat4 transform = mvTranslate(mvIdentityMat4(), node.translation) *
        mvRotate(mvIdentityMat4(), 0.0f, mvVec3{ 1.0f, 0.0f, 0.0f }) *
        mvRotate(mvIdentityMat4(), 0.0f, mvVec3{ 0.0f, 1.0f, 0.0f }) *
        mvRotate(mvIdentityMat4(), 0.0f, mvVec3{ 0.0f, 0.0f, 1.0f }) * mvScale(mvIdentityMat4(), node.scale);

    if (node.mesh > -1)
        mvRenderer_RenderMeshShadows(am, am.meshes[node.mesh].mesh, accumulatedTransform * transform, cam, proj);

    for (u32 i = 0; i < node.childCount; i++)
    {
        mvRenderer_RenderNodeShadows(am, am.nodes[node.children[i]].node, accumulatedTransform * transform, cam, proj);
    }
}

void
mvRenderer_RenderSceneShadows(mvAssetManager& am, mvScene& scene, mvMat4 cam, mvMat4 proj)
{
    for (u32 i = 0; i < scene.nodeCount; i++)
    {
        mvNode& rootNode = am.nodes[scene.nodes[i]].node;

        mvMat4 transform = mvTranslate(mvIdentityMat4(), rootNode.translation) *
            mvRotate(mvIdentityMat4(), 0.0f, mvVec3{ 1.0f, 0.0f, 0.0f }) *
            mvRotate(mvIdentityMat4(), 0.0f, mvVec3{ 0.0f, 1.0f, 0.0f }) *
            mvRotate(mvIdentityMat4(), 0.0f, mvVec3{ 0.0f, 0.0f, 1.0f }) * mvScale(mvIdentityMat4(), rootNode.scale);

        if (rootNode.mesh > -1)
            mvRenderer_RenderMeshShadows(am, am.meshes[rootNode.mesh].mesh, transform, cam, proj);

        for (u32 j = 0; j < rootNode.childCount; j++)
        {
            mvRenderer_RenderNodeShadows(am, am.nodes[rootNode.children[j]].node, transform, cam, proj);
        }
    }
}