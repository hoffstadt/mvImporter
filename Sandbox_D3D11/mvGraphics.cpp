#include "mvGraphics.h"
#include <assert.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#include <filesystem>
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include "mvAssetLoader.h"
#include "mvAnimation.h"
#include "mvViewport.h"

#define S_GLTF_IMPLEMENTATION
#include "sGltf.h"

#define MV_MATH_IMPLEMENTATION
#include "mvMath.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

mvGraphics
setup_graphics(mvViewport& viewport, const char* shaderDirectory)
{
	mvGraphics graphics{};
	graphics.shaderDirectory = shaderDirectory;
    graphics.threadID = std::this_thread::get_id();

    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferDesc.Width = viewport.width;
    sd.BufferDesc.Height = viewport.height;
    sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 0;
    sd.BufferDesc.RefreshRate.Denominator = 0;
    sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.BufferCount = 2;
    sd.OutputWindow = viewport.hWnd;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; //DXGI_SWAP_EFFECT_DISCARD;
    sd.Flags = 0;

    UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT | D3D11_CREATE_DEVICE_DISABLE_GPU_TIMEOUT;
    #ifdef MV_DEBUG
    creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
    #endif // MV_DEBUG


    // create device and front/back buffers, and swap chain and rendering context
    HRESULT hResult = D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        creationFlags,
        nullptr,
        0,
        D3D11_SDK_VERSION,
        &sd,
        graphics.swapChain.GetAddressOf(),
        graphics.device.GetAddressOf(),
        nullptr,
        graphics.imDeviceContext.GetAddressOf()
    );
    assert(SUCCEEDED(hResult));

    // Create Framebuffer Render Target
    hResult = graphics.swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)graphics.frameBuffer.GetAddressOf());
    assert(SUCCEEDED(hResult));

    // create render target
    D3D11_TEXTURE2D_DESC textureDesc;
    graphics.frameBuffer->GetDesc(&textureDesc);

    // create the target view on the texture
    D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
    rtvDesc.Format = textureDesc.Format;
    rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    rtvDesc.Texture2D = D3D11_TEX2D_RTV{ 0 };

    hResult = graphics.device->CreateRenderTargetView(graphics.frameBuffer.Get(), &rtvDesc, graphics.target.GetAddressOf());
    assert(SUCCEEDED(hResult));

    // create transform constant buffer
    D3D11_BUFFER_DESC cbd;
    cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbd.Usage = D3D11_USAGE_DYNAMIC;
    cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    cbd.MiscFlags = 0u;
    cbd.ByteWidth = sizeof(mvTransforms);
    cbd.StructureByteStride = 0u;

    graphics.device->CreateBuffer(&cbd, nullptr, graphics.tranformCBuf.GetAddressOf());

    // create depth stensil texture
    Microsoft::WRL::ComPtr<ID3D11Texture2D> pDepthStencil;
    D3D11_TEXTURE2D_DESC descDepth = {};
    descDepth.Width = viewport.width;
    descDepth.Height = viewport.height;
    descDepth.MipLevels = 1u;
    descDepth.ArraySize = 1u;
    descDepth.Format = DXGI_FORMAT_R24G8_TYPELESS;
    descDepth.SampleDesc.Count = 1u;
    descDepth.SampleDesc.Quality = 0u;
    descDepth.Usage = D3D11_USAGE_DEFAULT;
    descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    graphics.device->CreateTexture2D(&descDepth, nullptr, pDepthStencil.GetAddressOf());

    // create view of depth stensil texture
    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
    descDSV.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    descDSV.Texture2D.MipSlice = 0u;
    descDSV.Flags = 0;
    graphics.device->CreateDepthStencilView(pDepthStencil.Get(), &descDSV, graphics.targetDepth.GetAddressOf());

    graphics.viewport = { 0.0f, 0.0f, (float)viewport.width, (float)viewport.height, 0.0f, 1.0f };

	return graphics;
}

void
recreate_swapchain(mvGraphics& graphics, unsigned width, unsigned height)
{
    if (graphics.device)
    {
        graphics.target->Release();
        graphics.targetDepth->Release();
        graphics.imDeviceContext->OMSetRenderTargets(0, 0, 0);
        graphics.frameBuffer->Release();

        graphics.swapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);

        // Create Framebuffer Render Target
        graphics.swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)graphics.frameBuffer.GetAddressOf());

        // create render target
        D3D11_TEXTURE2D_DESC textureDesc;
        graphics.frameBuffer->GetDesc(&textureDesc);

        // create the target view on the texture
        D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
        rtvDesc.Format = textureDesc.Format;
        rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
        rtvDesc.Texture2D = D3D11_TEX2D_RTV{ 0 };

        HRESULT hResult = graphics.device->CreateRenderTargetView(graphics.frameBuffer.Get(), &rtvDesc, graphics.target.GetAddressOf());
        assert(SUCCEEDED(hResult));

        // create depth stensil texture
        Microsoft::WRL::ComPtr<ID3D11Texture2D> pDepthStencil;
        D3D11_TEXTURE2D_DESC descDepth = {};
        descDepth.Width = width;
        descDepth.Height = height;
        descDepth.MipLevels = 1u;
        descDepth.ArraySize = 1u;
        descDepth.Format = DXGI_FORMAT_R24G8_TYPELESS;
        descDepth.SampleDesc.Count = 1u;
        descDepth.SampleDesc.Quality = 0u;
        descDepth.Usage = D3D11_USAGE_DEFAULT;
        descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        graphics.device->CreateTexture2D(&descDepth, nullptr, pDepthStencil.GetAddressOf());

        // create view of depth stensil texture
        D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
        descDSV.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        descDSV.Texture2D.MipSlice = 0u;
        descDSV.Flags = 0;
        graphics.device->CreateDepthStencilView(pDepthStencil.Get(), &descDSV, graphics.targetDepth.GetAddressOf());

        graphics.viewport.Width = width;
        graphics.viewport.Height = height;

    }
}

void 
set_pipeline_state(mvGraphics& graphics, mvPipeline& pipeline)
{
    auto device = graphics.imDeviceContext;
    device->IASetPrimitiveTopology(pipeline.topology);
    device->RSSetState(pipeline.rasterizationState.Get());
    device->OMSetBlendState(pipeline.blendState.Get(), nullptr, 0xFFFFFFFFu);
    device->OMSetDepthStencilState(pipeline.depthStencilState.Get(), 0xFF);;
    device->IASetInputLayout(pipeline.inputLayout.Get());
    device->VSSetShader(pipeline.vertexShader.Get(), nullptr, 0);
    device->PSSetShader(pipeline.pixelShader.Get(), nullptr, 0);
    device->HSSetShader(nullptr, nullptr, 0);
    device->DSSetShader(nullptr, nullptr, 0);
    device->GSSetShader(nullptr, nullptr, 0);
}

mvRendererContext
create_renderer_context(mvGraphics& graphics)
{
    mvRendererContext ctx{};

    ctx.globalInfoBuffer = create_const_buffer(graphics, &ctx.globalInfo, sizeof(GlobalInfo));

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
    graphics.device->CreateBlendState(&desc, &ctx.finalBlendState);

    {
        D3D11_RASTERIZER_DESC rasterDesc = CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT{});
        rasterDesc.CullMode = D3D11_CULL_NONE;
        rasterDesc.FillMode = D3D11_FILL_WIREFRAME;

        graphics.device->CreateRasterizerState(&rasterDesc, ctx.solidWireframePipeline.rasterizationState.GetAddressOf());

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

        graphics.device->CreateDepthStencilState(&dsDesc, ctx.solidWireframePipeline.depthStencilState.GetAddressOf());

        D3D11_BLEND_DESC blendDesc = CD3D11_BLEND_DESC{ CD3D11_DEFAULT{} };
        auto& brt = blendDesc.RenderTarget[0];
        brt.BlendEnable = TRUE;
        brt.SrcBlend = D3D11_BLEND_SRC_ALPHA;
        brt.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        graphics.device->CreateBlendState(&blendDesc, ctx.solidWireframePipeline.blendState.GetAddressOf());

        mvPixelShader pixelShader = create_pixel_shader(graphics, std::string(graphics.shaderDirectory) + "Solid_PS.hlsl");
        mvVertexLayout layout = create_vertex_layout({ Position3D });
        mvVertexShader vertexShader = create_vertex_shader(graphics, std::string(graphics.shaderDirectory) + "Solid_VS.hlsl", layout);

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

        ctx.solidWireframePipeline.info.layout = create_vertex_layout({Position3D});

    }

    {

        D3D11_RASTERIZER_DESC rasterDesc = CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT{});
        rasterDesc.CullMode = D3D11_CULL_BACK;
        rasterDesc.FrontCounterClockwise = TRUE;

        graphics.device->CreateRasterizerState(&rasterDesc, ctx.solidPipeline.rasterizationState.GetAddressOf());

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

        graphics.device->CreateDepthStencilState(&dsDesc, ctx.solidPipeline.depthStencilState.GetAddressOf());

        D3D11_BLEND_DESC blendDesc = CD3D11_BLEND_DESC{ CD3D11_DEFAULT{} };
        auto& brt = blendDesc.RenderTarget[0];
        brt.BlendEnable = TRUE;
        brt.SrcBlend = D3D11_BLEND_SRC_ALPHA;
        brt.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        graphics.device->CreateBlendState(&blendDesc, ctx.solidPipeline.blendState.GetAddressOf());

        mvPixelShader pixelShader = create_pixel_shader(graphics, std::string(graphics.shaderDirectory) + "Solid_PS.hlsl");
        mvVertexLayout layout = create_vertex_layout({ Position3D });
        mvVertexShader vertexShader = create_vertex_shader(graphics, std::string(graphics.shaderDirectory) + "Solid_VS.hlsl", layout);

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
        ctx.solidPipeline.info.layout = create_vertex_layout({Position3D});
    }

    {
        ctx.skyboxPipeline.topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;


        mvVertexLayout vertexLayout = create_vertex_layout({ Position3D });

        mvPixelShader pixelShader = create_pixel_shader(graphics, std::string(graphics.shaderDirectory) + "Skybox_PS.hlsl");
        ctx.skyboxPipeline.pixelShader = pixelShader.shader;
        ctx.skyboxPipeline.pixelBlob = pixelShader.blob;

        mvVertexShader vertexShader = create_vertex_shader(graphics, std::string(graphics.shaderDirectory) + "Skybox_VS.hlsl", vertexLayout);
        ctx.skyboxPipeline.vertexShader = vertexShader.shader;
        ctx.skyboxPipeline.vertexBlob = vertexShader.blob;
        ctx.skyboxPipeline.inputLayout = vertexShader.inputLayout;

        // depth stencil state
        D3D11_DEPTH_STENCIL_DESC dsDesc = CD3D11_DEPTH_STENCIL_DESC{ CD3D11_DEFAULT{} };
        dsDesc.DepthEnable = true;
        dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
        dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
        graphics.device->CreateDepthStencilState(&dsDesc, ctx.skyboxPipeline.depthStencilState.GetAddressOf());

        // rasterizer state
        D3D11_RASTERIZER_DESC skyboxRasterDesc = CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT{});
        skyboxRasterDesc.CullMode = D3D11_CULL_NONE;
        skyboxRasterDesc.FrontCounterClockwise = TRUE;
        graphics.device->CreateRasterizerState(&skyboxRasterDesc, ctx.skyboxPipeline.rasterizationState.GetAddressOf());

        // blend state
        D3D11_BLEND_DESC blendDesc = CD3D11_BLEND_DESC{ CD3D11_DEFAULT{} };
        auto& brt = blendDesc.RenderTarget[0];
        brt.BlendEnable = TRUE;
        brt.SrcBlend = D3D11_BLEND_SRC_ALPHA;
        brt.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        graphics.device->CreateBlendState(&blendDesc, ctx.skyboxPipeline.blendState.GetAddressOf());
    }

    return ctx;
}

static void
submit_mesh(mvGraphics& graphics, mvModel& model, mvRendererContext& ctx, mvMesh& mesh, mvMat4 transform, mvSkin* skin)
{
    auto device = graphics.imDeviceContext;

    for (unsigned int i = 0; i < mesh.primitives.size(); i++)
    {
        mvMeshPrimitive& primitive = mesh.primitives[i];

        // if material not assigned, get material
        if (primitive.materialID == -1)
        {
            assert(false && "material not assigned");
        }

        mvMaterial* material = &model.materialManager.materials[primitive.materialID].asset;

        if(mesh.morphBuffer.buffer)
            update_const_buffer(graphics, mesh.morphBuffer, mesh.weightsAnimated.data());

        if (material->alphaMode == 2)
            ctx.transparentJobs.push_back({ &primitive, transform, skin, mesh.morphBuffer.buffer });
        else
            ctx.opaqueJobs.push_back({ &primitive, transform, skin, mesh.morphBuffer.buffer });
    }
}

static void
submit_node(mvGraphics& graphics, mvModel& model, mvRendererContext& ctx, mvNode& node, mvMat4 parentTransform)
{
    mvSkin* skin = nullptr;

    node.worldTransform = parentTransform * node.transform;
    node.inverseWorldTransform = invert(node.worldTransform);

    if (node.skin != -1)
        skin = &model.skins[node.skin];
    if (node.mesh > -1 && node.camera == -1)
        submit_mesh(graphics, model, ctx, model.meshes[node.mesh], node.worldTransform, skin);
    else if (node.camera > -1)
    {
        for (unsigned int i = 0; i < model.meshes[node.mesh].primitives.size(); i++)
        {
            mvMeshPrimitive& primitive = model.meshes[node.mesh].primitives[i];
            ctx.wireframeJobs.push_back({ &primitive, node.worldTransform* scale(identity_mat4(), {-1.0f, -1.0f, -1.0f}) });
        }
    }

    for (unsigned int i = 0; i < node.childCount; i++)
    {
        submit_node(graphics, model, ctx, model.nodes[node.children[i]], node.worldTransform);
    }
}

void 
submit_scene(mvGraphics& graphics, mvModel& model, mvRendererContext& ctx, mvScene& scene)
{
    mvSkin* skin = nullptr;

    for (unsigned int i = 0; i < scene.nodeCount; i++)
    {
        mvNode& rootNode = model.nodes[scene.nodes[i]];
        rootNode.worldTransform = rootNode.transform;
        rootNode.inverseWorldTransform = invert(rootNode.worldTransform);

        if (rootNode.skin != -1)
            skin = &model.skins[rootNode.skin];

        if (rootNode.mesh > -1 && rootNode.camera == -1)
            submit_mesh(graphics, model, ctx, model.meshes[rootNode.mesh], rootNode.worldTransform, skin);
        else if (rootNode.camera > -1)
        {
            for (unsigned int i = 0; i < model.meshes[rootNode.mesh].primitives.size(); i++)
            {
                mvMeshPrimitive& primitive = model.meshes[rootNode.mesh].primitives[i];
                ctx.wireframeJobs.push_back({ &primitive, rootNode.worldTransform * scale(identity_mat4(), {-1.0f, -1.0f, -1.0f}) });
            }
        }

        for (unsigned int j = 0; j < rootNode.childCount; j++)
        {
            submit_node(graphics, model, ctx, model.nodes[rootNode.children[j]], rootNode.worldTransform);
        }
    }
}

static void
render_job(mvGraphics& graphics, mvModel& model, mvRenderJob& job, mvMat4 cam, mvMat4 proj)
{
    auto device = graphics.imDeviceContext;

    mvMeshPrimitive& primitive = *job.meshPrimitive;

    mvMaterial* material = &model.materialManager.materials[primitive.materialID].asset;

    if (material->pipeline.info.layout != primitive.layout)
    {
        assert(false && "Mesh and material vertex layouts don't match.");
        return;
    }

    // pipeline
    set_pipeline_state(graphics, material->pipeline);
    
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

    update_const_buffer(graphics, material->buffer, &material->data);
    device->PSSetConstantBuffers(1u, 1u, material->buffer.buffer.GetAddressOf());

    mvTransforms transforms{};
    transforms.model = job.accumulatedTransform;
    transforms.modelView = cam * transforms.model;
    transforms.modelViewProjection = proj * cam * transforms.model;

    D3D11_MAPPED_SUBRESOURCE mappedSubresource;
    device->Map(graphics.tranformCBuf.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &mappedSubresource);
    memcpy(mappedSubresource.pData, &transforms, sizeof(mvTransforms));
    device->Unmap(graphics.tranformCBuf.Get(), 0u);

    // mesh
    static const UINT offset = 0u;
    device->VSSetConstantBuffers(0u, 1u, graphics.tranformCBuf.GetAddressOf());
    if (job.morphBuffer)
    {
        device->VSSetConstantBuffers(3u, 1u, job.morphBuffer.GetAddressOf());
    }
    device->IASetIndexBuffer(primitive.indexBuffer.buffer.Get(), DXGI_FORMAT_R32_UINT, 0u);
    device->IASetVertexBuffers(0u, 1u, primitive.vertexBuffer.buffer.GetAddressOf(), &material->pipeline.info.layout.size, &offset);

    // draw
    device->DrawIndexed(primitive.indexBuffer.size / sizeof(unsigned int), 0u, 0u);
}

static void
render_wireframe_job(mvGraphics& graphics, mvRendererContext& rendererCtx, mvModel& model, mvRenderJob& job, mvMat4 cam, mvMat4 proj)
{

    auto device = graphics.imDeviceContext;

    set_pipeline_state(graphics, rendererCtx.solidWireframePipeline);

    mvMeshPrimitive& primitive = *job.meshPrimitive;

    mvTransforms transforms{};
    transforms.model = job.accumulatedTransform;
    transforms.modelView = cam * transforms.model;
    transforms.modelViewProjection = proj * cam * transforms.model;

    D3D11_MAPPED_SUBRESOURCE mappedSubresource;
    device->Map(graphics.tranformCBuf.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &mappedSubresource);
    memcpy(mappedSubresource.pData, &transforms, sizeof(mvTransforms));
    device->Unmap(graphics.tranformCBuf.Get(), 0u);

    // mesh
    static const UINT offset = 0u;
    device->VSSetConstantBuffers(0u, 1u, graphics.tranformCBuf.GetAddressOf());
    device->IASetIndexBuffer(primitive.indexBuffer.buffer.Get(), DXGI_FORMAT_R32_UINT, 0u);
    device->IASetVertexBuffers(0u, 1u, primitive.vertexBuffer.buffer.GetAddressOf(), &rendererCtx.solidWireframePipeline.info.layout.size, &offset);

    // draw
    device->DrawIndexed(primitive.indexBuffer.size / sizeof(unsigned int), 0u, 0u);
}

void 
render_scenes(mvGraphics& graphics, mvModel& model, mvRendererContext& ctx, mvMat4 cam, mvMat4 proj)
{
    // opaque objects
    for (int i = 0; i < ctx.opaqueJobs.size(); i++)
        render_job(graphics, model, ctx.opaqueJobs[i], cam, proj);

    // transparent objects
    for (int i = 0; i < ctx.transparentJobs.size(); i++)
        render_job(graphics, model, ctx.transparentJobs[i], cam, proj);

    // wireframe objects
    for (int i = 0; i < ctx.wireframeJobs.size(); i++)
        render_wireframe_job(graphics, ctx, model, ctx.wireframeJobs[i], cam, proj);

    // reset
    ctx.opaqueJobs.clear();
    ctx.transparentJobs.clear();
    ctx.wireframeJobs.clear();
}

void
render_mesh_solid(mvGraphics& graphics, mvRendererContext& rendererCtx, mvModel& model, mvMesh& mesh, mvMat4 transform, mvMat4 cam, mvMat4 proj)
{
    auto device = graphics.imDeviceContext;

    set_pipeline_state(graphics, rendererCtx.solidPipeline);

    for (unsigned int i = 0; i < mesh.primitives.size(); i++)
    {
        mvMeshPrimitive& primitive = mesh.primitives[i];

        mvTransforms transforms{};
        transforms.model = transform;
        transforms.modelView = cam * transforms.model;
        transforms.modelViewProjection = proj * cam * transforms.model;

        D3D11_MAPPED_SUBRESOURCE mappedSubresource;
        device->Map(graphics.tranformCBuf.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &mappedSubresource);
        memcpy(mappedSubresource.pData, &transforms, sizeof(mvTransforms));
        device->Unmap(graphics.tranformCBuf.Get(), 0u);

        // mesh
        static const UINT offset = 0u;
        device->VSSetConstantBuffers(0u, 1u, graphics.tranformCBuf.GetAddressOf());
        device->IASetIndexBuffer(primitive.indexBuffer.buffer.Get(), DXGI_FORMAT_R32_UINT, 0u);
        device->IASetVertexBuffers(0u, 1u, primitive.vertexBuffer.buffer.GetAddressOf(), &rendererCtx.solidPipeline.info.layout.size, &offset);

        // draw
        device->DrawIndexed(primitive.indexBuffer.size / sizeof(unsigned int), 0u, 0u);
    }
}

void 
render_skybox(mvGraphics& graphics, mvRendererContext& rendererCtx, mvModel& model, mvCubeTexture& cubemap, ID3D11SamplerState* sampler, mvMat4 cam, mvMat4 proj)
{

    static const float side = 1.0f / 2.0f;
    static auto vertices = std::vector<float>{
        -side, -side, -side,
         side, -side, -side,
        -side,  side, -side,
         side,  side, -side,
        -side, -side,  side,
         side, -side,  side,
        -side,  side,  side,
         side,  side,  side
    };

    static auto indices = std::vector<unsigned int>{
        0, 2, 1, 2, 3, 1,
        1, 3, 5, 3, 7, 5,
        2, 6, 3, 3, 6, 7,
        4, 5, 7, 4, 7, 6,
        0, 4, 2, 2, 4, 6,
        0, 1, 4, 1, 5, 4
    };

    static mvVertexLayout vertexLayout = create_vertex_layout({ Position3D });
    static mvBuffer vertexBuffer = create_buffer(graphics, vertices.data(), vertices.size() * sizeof(float), D3D11_BIND_VERTEX_BUFFER);
    static mvBuffer indexBuffer = create_buffer(graphics, indices.data(), indices.size() * sizeof(unsigned int), D3D11_BIND_INDEX_BUFFER);

    auto ctx = graphics.imDeviceContext;

    ctx->PSSetSamplers(0, 1, &sampler);
    ctx->PSSetShaderResources(0, 1, cubemap.textureView.GetAddressOf());

    // pipeline
    set_pipeline_state(graphics, rendererCtx.skyboxPipeline);

    mvTransforms transforms{};
    transforms.model = identity_mat4() * scale(identity_mat4(), mvVec3{ 1.0f, 1.0f, -1.0f });
    transforms.modelView = cam * transforms.model;
    transforms.modelViewProjection = proj * cam * transforms.model;

    D3D11_MAPPED_SUBRESOURCE mappedSubresource;
    ctx->Map(graphics.tranformCBuf.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &mappedSubresource);
    memcpy(mappedSubresource.pData, &transforms, sizeof(mvTransforms));
    ctx->Unmap(graphics.tranformCBuf.Get(), 0u);

    // mesh
    static const UINT offset = 0u;
    ctx->VSSetConstantBuffers(0u, 1u, graphics.tranformCBuf.GetAddressOf());
    ctx->IASetIndexBuffer(indexBuffer.buffer.Get(), DXGI_FORMAT_R32_UINT, 0u);
    ctx->IASetVertexBuffers(0u, 1u, vertexBuffer.buffer.GetAddressOf(), &vertexLayout.size, &offset);

    // draw
    ctx->DrawIndexed(indexBuffer.size / sizeof(unsigned int), 0u, 0u);
}

mvBuffer
create_buffer(mvGraphics& graphics, void* data, unsigned int size, D3D11_BIND_FLAG flags, unsigned int stride, unsigned int miscFlags)
{
	mvBuffer buffer{};
	buffer.size = size;

    // Fill in a buffer description.
    D3D11_BUFFER_DESC bufferDesc{};
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.CPUAccessFlags = 0u;

    bufferDesc.ByteWidth = size;
    bufferDesc.BindFlags = flags;
    bufferDesc.StructureByteStride = stride;
    bufferDesc.MiscFlags = miscFlags;

    // Define the resource data.
    D3D11_SUBRESOURCE_DATA InitData = {};
    InitData.pSysMem = data;

    // Create the buffer with the device.
    HRESULT hresult = graphics.device->CreateBuffer(&bufferDesc, &InitData, nullptr);

    if (hresult != S_FALSE)
    {
        assert(false && "something is wrong");
    }

    else
    {
        hresult = graphics.device->CreateBuffer(&bufferDesc, &InitData, buffer.buffer.GetAddressOf());
        assert(SUCCEEDED(hresult));
    }

    if (stride != 0u)
    {
        D3D11_BUFFER_DESC descBuf;
        buffer.buffer->GetDesc(&descBuf);

        D3D11_UNORDERED_ACCESS_VIEW_DESC descView = {};
        descView.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
        descView.Buffer.FirstElement = 0;

        descView.Format = DXGI_FORMAT_UNKNOWN;
        descView.Buffer.NumElements = descBuf.ByteWidth / descBuf.StructureByteStride;

        hresult = graphics.device->CreateUnorderedAccessView(buffer.buffer.Get(),
            &descView, buffer.unorderedAccessView.GetAddressOf());
        assert(SUCCEEDED(hresult));
    }

	return buffer;
}

mvConstBuffer
create_const_buffer(mvGraphics& graphics, void* data, unsigned int size)
{
    mvConstBuffer buffer{};
    buffer.size = size;

    D3D11_BUFFER_DESC cbd;
    cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbd.Usage = D3D11_USAGE_DYNAMIC;
    cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    cbd.MiscFlags = 0u;
    cbd.ByteWidth = size;
    cbd.StructureByteStride = 0u;

    if (data != nullptr)
    {
        D3D11_SUBRESOURCE_DATA csd = {};
        csd.pSysMem = &data;
        graphics.device->CreateBuffer(&cbd, &csd, buffer.buffer.GetAddressOf());
    }
    else
        graphics.device->CreateBuffer(&cbd, nullptr, buffer.buffer.GetAddressOf());

    return buffer;
}

void
update_const_buffer(mvGraphics& graphics, mvConstBuffer& buffer, void* data)
{
    D3D11_MAPPED_SUBRESOURCE mappedSubresource;
    graphics.imDeviceContext->Map(buffer.buffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &mappedSubresource);
    memcpy(mappedSubresource.pData, data, buffer.size);
    graphics.imDeviceContext->Unmap(buffer.buffer.Get(), 0u);
}

mvMesh
create_cube(mvGraphics& graphics, float size)
{

    mvVertexLayout layout = create_vertex_layout(
        {
            Position3D
        }
    );

    const float side = size;
    auto vertices = std::vector<float>{
        -side, -side, -side,  // 0 near side
         side, -side, -side,  // 1
        -side,  side, -side,  // 2
         side,  side, -side,  // 3
        -side, -side,  side,  // 4 far side
         side, -side,  side,  // 5
        -side,  side,  side,  // 6
         side,  side,  side,  // 7
        -side, -side, -side,  // 8 left side
        -side,  side, -side,  // 9
        -side, -side,  side,  // 10
        -side,  side,  side,  // 11
         side, -side, -side,  // 12 right side
         side,  side, -side,  // 13
         side, -side,  side,  // 14
         side,  side,  side,  // 15
        -side, -side, -side,  // 16 bottom side
         side, -side, -side,  // 17
        -side, -side,  side,  // 18
         side, -side,  side,  // 19
        -side,  side, -side,  // 20 top side
         side,  side, -side,  // 21
        -side,  side,  side,  // 22
         side,  side,  side   // 23
    };

    static auto indices = std::vector<unsigned int>{
        1,  2,  0,  1,  3,  2,
        7,  5,  4,  6,  7,  4,
        9, 10,  8, 9, 11,  10,
        15, 13, 12, 14, 15, 12,
        18, 17, 16, 19, 17, 18,
        21, 23, 20, 23, 22, 20
    };

    mvMesh mesh{};
    mesh.name = "cube";
    mesh.primitives.push_back({});
    mesh.primitives.back().layout = layout;
    mesh.primitives.back().vertexBuffer = create_buffer(graphics, vertices.data(), vertices.size() * sizeof(float), D3D11_BIND_VERTEX_BUFFER);
    mesh.primitives.back().indexBuffer = create_buffer(graphics, indices.data(), indices.size() * sizeof(unsigned int), D3D11_BIND_INDEX_BUFFER);

    return mesh;
}

mvMesh
create_textured_cube(mvGraphics& graphics, float size)
{

    mvVertexLayout layout = create_vertex_layout(
        {
            Position3D,
            Normal,
            TexCoord0,
            Tangent
        }
    );

    const float side = size;
    auto vertices = std::vector<float>{
        -side, -side, -side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 0 near side
         side, -side, -side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 1
        -side,  side, -side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 2
         side,  side, -side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 3

        -side, -side,  side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 4 far side
         side, -side,  side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 5
        -side,  side,  side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 6
         side,  side,  side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 7

        -side, -side, -side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 8 left side
        -side,  side, -side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 9
        -side, -side,  side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 10
        -side,  side,  side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 11

         side, -side, -side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 12 right side
         side,  side, -side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 13
         side, -side,  side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 14
         side,  side,  side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 15

        -side, -side, -side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 16 bottom side
         side, -side, -side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 17
        -side, -side,  side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 18
         side, -side,  side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 19

        -side,  side, -side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 20 top side
         side,  side, -side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 21
        -side,  side,  side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 22
         side,  side,  side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f   // 23
    };

    static auto indices = std::vector<unsigned int>{
        1,  2,  0,  1,  3,  2,
        7,  5,  4,  6,  7,  4,
        9, 10,  8, 9, 11,  10,
        15, 13, 12, 14, 15, 12,
        18, 17, 16, 19, 17, 18,
        21, 23, 20, 23, 22, 20
    };

    for (size_t i = 0; i < indices.size(); i += 3)
    {

        mvVec3 p0 = { vertices[11 * indices[i]], vertices[14 * indices[i] + 1], vertices[14 * indices[i] + 2] };
        mvVec3 p1 = { vertices[11 * indices[i+1]], vertices[14 * indices[i + 1] + 1], vertices[14 * indices[i + 1] + 2] };
        mvVec3 p2 = { vertices[11 * indices[i+2]], vertices[14 * indices[i + 2] + 1], vertices[14 * indices[i + 2] + 2] };

        mvVec3 n = normalize(cross(p1 - p0, p2 - p0));
        vertices[11 * indices[i] + 3] = n[0];
        vertices[11 * indices[i] + 4] = n[1];
        vertices[11 * indices[i] + 5] = n[2];
        vertices[11 * indices[i + 1] + 3] = n[0];
        vertices[11 * indices[i + 1] + 4] = n[1];
        vertices[11 * indices[i + 1] + 5] = n[2];
        vertices[11 * indices[i + 2] + 3] = n[0];
        vertices[11 * indices[i + 2] + 4] = n[1];
        vertices[11 * indices[i + 2] + 5] = n[2];
    }

    mvMesh mesh{};
    mesh.name = "textured cube";
    mesh.primitives.push_back({});
    mesh.primitives.back().layout = layout;
    mesh.primitives.back().vertexBuffer = create_buffer(graphics, vertices.data(), vertices.size() * sizeof(float), D3D11_BIND_VERTEX_BUFFER);
    mesh.primitives.back().indexBuffer = create_buffer(graphics, indices.data(), indices.size() * sizeof(unsigned int), D3D11_BIND_INDEX_BUFFER);

    return mesh;
}

mvMesh
create_textured_quad(mvGraphics& graphics, float size)
{

    mvVertexLayout layout = create_vertex_layout(
        {
            Position3D,
            Normal,
            TexCoord0,
            Tangent
        }
    );

    const float side = size;
    auto vertices = std::vector<float>{
        -side,  side, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
         side, -side, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        -side, -side, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
         side,  side, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    };

    static auto indices = std::vector<unsigned int>{
        1, 0, 2,
        3, 0, 1
    };

    for (size_t i = 0; i < indices.size(); i += 3)
    {
        mvVec3 p0 = { vertices[11 * indices[i]], vertices[11 * indices[i] + 1], vertices[11 * indices[i] + 2] };
        mvVec3 p1 = { vertices[11 * indices[i + 1]], vertices[11 * indices[i + 1] + 1], vertices[11 * indices[i + 1] + 2] };
        mvVec3 p2 = { vertices[11 * indices[i + 2]], vertices[11 * indices[i + 2] + 1], vertices[11 * indices[i + 2] + 2] };

        mvVec3 n = normalize(cross(p1 - p0, p2 - p0));
        vertices[11 * indices[i] + 3] = n[0];
        vertices[11 * indices[i] + 4] = n[1];
        vertices[11 * indices[i] + 5] = n[2];
        vertices[11 * indices[i + 1] + 3] = n[0];
        vertices[11 * indices[i + 1] + 4] = n[1];
        vertices[11 * indices[i + 1] + 5] = n[2];
        vertices[11 * indices[i + 2] + 3] = n[0];
        vertices[11 * indices[i + 2] + 4] = n[1];
        vertices[11 * indices[i + 2] + 5] = n[2];
    }

    mvMesh mesh{};

    mesh.name = "textured quad";
    mesh.primitives.push_back({});
    mesh.primitives.back().layout = layout;
    mesh.primitives.back().vertexBuffer = create_buffer(graphics, vertices.data(), vertices.size() * sizeof(float), D3D11_BIND_VERTEX_BUFFER);
    mesh.primitives.back().indexBuffer = create_buffer(graphics, indices.data(), indices.size() * sizeof(unsigned int), D3D11_BIND_INDEX_BUFFER);

    return mesh;
}

mvMesh
create_frustum(mvGraphics& graphics, float width, float height, float nearZ, float farZ)
{
    static unsigned int id = 0;
    id++;

    mvVertexLayout layout = create_vertex_layout(
        {
            Position3D
        }
    );

    float smallWidth = atan(width / (2.0f * farZ));
    float smallHeight = atan(height / (2.0f * farZ));

    auto vertices = std::vector<float>{
         smallWidth,  smallHeight, nearZ,
        -smallWidth,  smallHeight, nearZ,
        -smallWidth, -smallHeight, nearZ,
         smallWidth, -smallHeight, nearZ,
         width / 2.0f,  height / 2.0f, farZ,
        -width / 2.0f,  height / 2.0f, farZ,
        -width / 2.0f, -height / 2.0f, farZ,
         width / 2.0f, -height / 2.0f, farZ,
    };

    static auto indices = std::vector<unsigned int>{
        0, 1,
        1, 2,
        2, 3,
        3, 0,
        4, 5,
        5, 6,
        6, 7,
        7, 4,
        0, 4,
        3, 7,
        1, 5,
        2, 6
    };

    mvMesh mesh{};

    mesh.name = "frustum1_" + std::to_string(id);
    mesh.primitives.push_back({});
    mesh.primitives.back().layout = layout;
    mesh.primitives.back().vertexBuffer = create_buffer(graphics, vertices.data(), vertices.size() * sizeof(float), D3D11_BIND_VERTEX_BUFFER);
    mesh.primitives.back().indexBuffer = create_buffer(graphics, indices.data(), indices.size() * sizeof(unsigned int), D3D11_BIND_INDEX_BUFFER);

    return mesh;
}

mvMesh
create_frustum2(mvGraphics& graphics, float fov, float aspect, float nearZ, float farZ)
{
    static unsigned int id = 0;
    id++;
    mvVertexLayout layout = create_vertex_layout(
        {
            Position3D
        }
    );

    float smallWidth = tan(fov) * nearZ;
    float smallHeight = smallWidth / aspect;

    float bigWidth = tan(fov) * farZ;
    float bigHeight = bigWidth / aspect;


    auto vertices = std::vector<float>{
         smallWidth,  smallHeight, nearZ,
        -smallWidth,  smallHeight, nearZ,
        -smallWidth, -smallHeight, nearZ,
         smallWidth, -smallHeight, nearZ,
         bigWidth,  bigHeight, farZ,
        -bigWidth,  bigHeight, farZ,
        -bigWidth, -bigHeight, farZ,
         bigWidth, -bigHeight, farZ,
    };

    static auto indices = std::vector<unsigned int>{
        0, 1,
        1, 2,
        2, 3,
        3, 0,
        4, 5,
        5, 6,
        6, 7,
        7, 4,
        0, 4,
        3, 7,
        1, 5,
        2, 6
    };

    mvMesh mesh{};

    mesh.name = "frustum2_" + std::to_string(id);
    mesh.primitives.push_back({});
    mesh.primitives.back().layout = layout;
    mesh.primitives.back().vertexBuffer = create_buffer(graphics, vertices.data(), vertices.size() * sizeof(float), D3D11_BIND_VERTEX_BUFFER);
    mesh.primitives.back().indexBuffer = create_buffer(graphics, indices.data(), indices.size() * sizeof(unsigned int), D3D11_BIND_INDEX_BUFFER);

    return mesh;
}

mvMesh
create_ortho_frustum(mvGraphics& graphics, float width, float height, float nearZ, float farZ)
{
    static unsigned int id = 0;
    id++;

    mvVertexLayout layout = create_vertex_layout(
        {
            Position3D
        }
    );

    auto vertices = std::vector<float>{
         width,  height, nearZ,
        -width,  height, nearZ,
        -width, -height, nearZ,
         width, -height, nearZ,
         width / 2.0f,  height / 2.0f, farZ,
        -width / 2.0f,  height / 2.0f, farZ,
        -width / 2.0f, -height / 2.0f, farZ,
         width / 2.0f, -height / 2.0f, farZ,
    };

    static auto indices = std::vector<unsigned int>{
        0, 1,
        1, 2,
        2, 3,
        3, 0,
        4, 5,
        5, 6,
        6, 7,
        7, 4,
        0, 4,
        3, 7,
        1, 5,
        2, 6
    };

    mvMesh mesh{};

    mesh.name = "frustum3_" + std::to_string(id);
    mesh.primitives.push_back({});
    mesh.primitives.back().layout = layout;
    mesh.primitives.back().vertexBuffer = create_buffer(graphics, vertices.data(), vertices.size() * sizeof(float), D3D11_BIND_VERTEX_BUFFER);
    mesh.primitives.back().indexBuffer = create_buffer(graphics, indices.data(), indices.size() * sizeof(unsigned int), D3D11_BIND_INDEX_BUFFER);

    return mesh;
}

mvTexture 
create_texture(mvGraphics& graphics, unsigned int width, unsigned int height, unsigned int arraySize, float* data)
{
	mvTexture texture{};

	// Create Texture
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = arraySize;
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	//HRESULT hResult = graphics.device->CreateTexture2D(&textureDesc, nullptr, &texture.texture);
	//assert(SUCCEEDED(hResult));
	int texBytesPerRow = 4 * width * sizeof(float);

	// subresource data
	Microsoft::WRL::ComPtr<ID3D11Texture2D> textureResource;
	D3D11_SUBRESOURCE_DATA* sdata = new D3D11_SUBRESOURCE_DATA[arraySize];
	for (int i = 0; i < arraySize; i++)
	{
		sdata[i].pSysMem = &data[i* width * height * 4];
		//sdata[i].pSysMem = data;
		sdata[i].SysMemPitch = texBytesPerRow;
		sdata[i].SysMemSlicePitch = width * height * 4 * sizeof(float);
	}
	// create the texture resource
	HRESULT hResult = graphics.device->CreateTexture2D(&textureDesc, sdata, textureResource.GetAddressOf());
	assert(SUCCEEDED(hResult));
	delete[] sdata;

	// create the resource view on the texture
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = textureDesc.MipLevels;
	srvDesc.Texture2DArray.ArraySize = arraySize;
	srvDesc.Texture2DArray.MostDetailedMip = 0;
	srvDesc.Texture2DArray.MipLevels = textureDesc.MipLevels;

	hResult = graphics.device->CreateShaderResourceView(textureResource.Get(), &srvDesc, texture.textureView.GetAddressOf());
	assert(SUCCEEDED(hResult));

	// Create Sampler State
	D3D11_SAMPLER_DESC samplerDesc{};
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = samplerDesc.AddressV;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.BorderColor[0] = 0.0f;
	samplerDesc.MaxAnisotropy = D3D11_REQ_MAXANISOTROPY;
	samplerDesc.MinLOD = -FLT_MAX;
	samplerDesc.MaxLOD = FLT_MAX;
	hResult = graphics.device->CreateSamplerState(&samplerDesc, texture.sampler.GetAddressOf());
	assert(SUCCEEDED(hResult));

	return texture;
}

mvTexture
create_dynamic_texture(mvGraphics& graphics, unsigned int width, unsigned int height, unsigned int arraySize)
{
	mvTexture texture{};
	Microsoft::WRL::ComPtr<ID3D11Texture2D> textureResource;

	// Create Texture
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = arraySize;
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DYNAMIC;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	HRESULT hResult = graphics.device->CreateTexture2D(&textureDesc, nullptr, textureResource.GetAddressOf());
	assert(SUCCEEDED(hResult));

	// create the resource view on the texture
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = textureDesc.MipLevels;
	srvDesc.Texture2DArray.ArraySize = 1;
	srvDesc.Texture2DArray.MostDetailedMip = 0;
	srvDesc.Texture2DArray.MipLevels = textureDesc.MipLevels;

	hResult = graphics.device->CreateShaderResourceView(textureResource.Get(), &srvDesc, texture.textureView.GetAddressOf());
	assert(SUCCEEDED(hResult));

	// Create Sampler State
	D3D11_SAMPLER_DESC samplerDesc{};
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = samplerDesc.AddressV;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.BorderColor[0] = 0.0f;
	samplerDesc.MaxAnisotropy = D3D11_REQ_MAXANISOTROPY;
	samplerDesc.MinLOD = -FLT_MAX;
	samplerDesc.MaxLOD = FLT_MAX;
	hResult = graphics.device->CreateSamplerState(&samplerDesc, texture.sampler.GetAddressOf());
	assert(SUCCEEDED(hResult));

	return texture;
}

void
update_dynamic_texture(mvGraphics& graphics, mvTexture& texture, unsigned int width, unsigned int height, float* data)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

	//  Disable GPU access to the vertex buffer data.
	ID3D11Resource* resource;
	texture.textureView->GetResource(&resource);
	graphics.imDeviceContext->Map(resource, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	BYTE* mappedData = reinterpret_cast<BYTE*>(mappedResource.pData);
	BYTE* buffer = reinterpret_cast<BYTE*>(data);
	for (UINT i = 0; i < height; i++)
	{
		memcpy(mappedData, buffer, width * 4 * sizeof(int));
		mappedData += mappedResource.RowPitch;
		buffer += width * 4 * sizeof(int);
	}
	graphics.imDeviceContext->Unmap(resource, 0);

	resource->Release();
}

mvTexture
create_texture(mvGraphics& graphics, unsigned char* data, unsigned int dataSize)
{
	mvTexture texture{};
	Microsoft::WRL::ComPtr<ID3D11Texture2D> textureResource;

	float gamma = 1.0f;
	float gamma_scale = 1.0f;

	//if (stbi_is_hdr_from_memory(data.data(), data.size()))
	//{
	//	stbi_hdr_to_ldr_gamma(gamma);
	//	stbi_hdr_to_ldr_scale(gamma_scale);
	//}
	//else
	//{
	//	stbi_ldr_to_hdr_gamma(gamma);
	//	stbi_ldr_to_hdr_scale(gamma_scale);
	//}

	// Load Image
	int texWidth, texHeight, texNumChannels;
	int texForceNumChannels = 4;
	unsigned char* testTextureBytes = stbi_load_from_memory(data, dataSize, &texWidth, &texHeight,
		&texNumChannels, texForceNumChannels);
	assert(testTextureBytes);
	

	if (texNumChannels > 3)
		texture.alpha = true;

	//int texWidth, texHeight, texNumChannels;
	//stbi_info_from_memory(data.data(), data.size(), &texWidth, &texHeight, &texNumChannels);
	//if (texNumChannels > 3)
	//	texture.alpha = true;
	int texBytesPerRow = texForceNumChannels * texWidth;

	// Create Texture
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = texWidth;
	textureDesc.Height = texHeight;
	textureDesc.MipLevels = 0;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

	graphics.device->CreateTexture2D(&textureDesc, nullptr, textureResource.GetAddressOf());
	graphics.imDeviceContext->UpdateSubresource(textureResource.Get(), 0u, nullptr, testTextureBytes, texBytesPerRow, 0u);

	// create the resource view on the texture
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = -1;

	graphics.device->CreateShaderResourceView(textureResource.Get(), &srvDesc, texture.textureView.GetAddressOf());
	graphics.imDeviceContext->GenerateMips(texture.textureView.Get());

	free(testTextureBytes);

	return texture;
}

mvTexture
create_texture(mvGraphics& graphics, const std::string& path)
{
    mvTexture texture{};
	Microsoft::WRL::ComPtr<ID3D11Texture2D> textureResource;

    std::filesystem::path fpath = path;

    if (!std::filesystem::exists(path))
    {
        assert(false && "File not found.");
        return texture;
    }

	float gamma = 2.2f;
	float gamma_scale = 1.0f;

	if (stbi_is_hdr(path.c_str()))
	{
		stbi_hdr_to_ldr_gamma(gamma);
		stbi_hdr_to_ldr_scale(gamma_scale);
	}
	else
	{
		stbi_ldr_to_hdr_gamma(gamma);
		stbi_ldr_to_hdr_scale(gamma_scale);
	}

    // Load Image
    int texWidth, texHeight, texNumChannels;
    int texForceNumChannels = 4;
    unsigned char* testTextureBytes = stbi_load(path.c_str(), &texWidth, &texHeight,
        &texNumChannels, texForceNumChannels);
    assert(testTextureBytes);
    int texBytesPerRow = 4 * texWidth;

    if (texNumChannels > 3)
        texture.alpha = true;

    // Create Texture
    D3D11_TEXTURE2D_DESC textureDesc = {};
    textureDesc.Width = texWidth;
    textureDesc.Height = texHeight;
    textureDesc.MipLevels = 0;
    textureDesc.ArraySize = 1;
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    textureDesc.CPUAccessFlags = 0;
    textureDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

    graphics.device->CreateTexture2D(&textureDesc, nullptr, textureResource.GetAddressOf());
    graphics.imDeviceContext->UpdateSubresource(textureResource.Get(), 0u, nullptr, testTextureBytes, texBytesPerRow, 0u);

    // create the resource view on the texture
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = textureDesc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = -1;

    graphics.device->CreateShaderResourceView(textureResource.Get(), &srvDesc, texture.textureView.GetAddressOf());
    graphics.imDeviceContext->GenerateMips(texture.textureView.Get());

    free(testTextureBytes);

    return texture;
}

mvCubeTexture
create_cube_texture(mvGraphics& graphics, const std::string& path)
{
	mvCubeTexture texture{};
	Microsoft::WRL::ComPtr<ID3D11Texture2D> textureResource;

	// Load Image
	int texWidth, texHeight, texNumChannels, textBytesPerRow;

	// load 6 surfaces for cube faces
	unsigned char* surfaces[6];
	int texForceNumChannels = 4;

	float gamma = 1.0f;
	float gamma_scale = 1.0f;

	// right
	{
		std::string file = path + "\\right.png";

		if (stbi_is_hdr(path.c_str()))
		{
			stbi_hdr_to_ldr_gamma(gamma);
			stbi_hdr_to_ldr_scale(gamma_scale);
		}
		else
		{
			stbi_ldr_to_hdr_gamma(gamma);
			stbi_ldr_to_hdr_scale(gamma_scale);
		}

		unsigned char* testTextureBytes = stbi_load(file.c_str(), &texWidth, &texHeight,
			&texNumChannels, texForceNumChannels);
		assert(testTextureBytes);
		textBytesPerRow = 4 * texWidth;

		surfaces[0] = testTextureBytes;
	}

	// left
	{
		std::string file = path + "\\left.png";

		if (stbi_is_hdr(path.c_str()))
		{
			stbi_hdr_to_ldr_gamma(gamma);
			stbi_hdr_to_ldr_scale(gamma_scale);
		}
		else
		{
			stbi_ldr_to_hdr_gamma(gamma);
			stbi_ldr_to_hdr_scale(gamma_scale);
		}

		unsigned char* testTextureBytes = stbi_load(file.c_str(), &texWidth, &texHeight,
			&texNumChannels, texForceNumChannels);
		assert(testTextureBytes);
		textBytesPerRow = 4 * texWidth;

		surfaces[1] = testTextureBytes;
	}

	// top
	{
		std::string file = path + "\\top.png";

		if (stbi_is_hdr(path.c_str()))
		{
			stbi_hdr_to_ldr_gamma(gamma);
			stbi_hdr_to_ldr_scale(gamma_scale);
		}
		else
		{
			stbi_ldr_to_hdr_gamma(gamma);
			stbi_ldr_to_hdr_scale(gamma_scale);
		}

		unsigned char* testTextureBytes = stbi_load(file.c_str(), &texWidth, &texHeight,
			&texNumChannels, texForceNumChannels);
		assert(testTextureBytes);
		textBytesPerRow = 4 * texWidth;

		surfaces[2] = testTextureBytes;
	}

	// bottom
	{
		std::string file = path + "\\bottom.png";

		if (stbi_is_hdr(path.c_str()))
		{
			stbi_hdr_to_ldr_gamma(gamma);
			stbi_hdr_to_ldr_scale(gamma_scale);
		}
		else
		{
			stbi_ldr_to_hdr_gamma(gamma);
			stbi_ldr_to_hdr_scale(gamma_scale);
		}

		unsigned char* testTextureBytes = stbi_load(file.c_str(), &texWidth, &texHeight,
			&texNumChannels, texForceNumChannels);
		assert(testTextureBytes);
		textBytesPerRow = 4 * texWidth;

		surfaces[3] = testTextureBytes;
	}

	// front
	{
		std::string file = path + "\\front.png";

		if (stbi_is_hdr(path.c_str()))
		{
			stbi_hdr_to_ldr_gamma(gamma);
			stbi_hdr_to_ldr_scale(gamma_scale);
		}
		else
		{
			stbi_ldr_to_hdr_gamma(gamma);
			stbi_ldr_to_hdr_scale(gamma_scale);
		}

		unsigned char* testTextureBytes = stbi_load(file.c_str(), &texWidth, &texHeight,
			&texNumChannels, texForceNumChannels);
		assert(testTextureBytes);
		textBytesPerRow = 4 * texWidth;

		surfaces[4] = testTextureBytes;
	}

	// back
	{
		std::string file = path + "\\back.png";

		if (stbi_is_hdr(path.c_str()))
		{
			stbi_hdr_to_ldr_gamma(gamma);
			stbi_hdr_to_ldr_scale(gamma_scale);
		}
		else
		{
			stbi_ldr_to_hdr_gamma(gamma);
			stbi_ldr_to_hdr_scale(gamma_scale);
		}

		unsigned char* testTextureBytes = stbi_load(file.c_str(), &texWidth, &texHeight,
			&texNumChannels, texForceNumChannels);
		assert(testTextureBytes);
		textBytesPerRow = 4 * texWidth;

		surfaces[5] = testTextureBytes;
	}

	// texture descriptor
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = texWidth;
	textureDesc.Height = texHeight;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 6;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
	// subresource data
	D3D11_SUBRESOURCE_DATA data[6];
	for (int i = 0; i < 6; i++)
	{
		data[i].pSysMem = surfaces[i];
		data[i].SysMemPitch = textBytesPerRow;
		data[i].SysMemSlicePitch = 0;
	}
	// create the texture resource
	graphics.device->CreateTexture2D(&textureDesc, data, textureResource.GetAddressOf());

	// create the resource view on the texture
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	graphics.device->CreateShaderResourceView(textureResource.Get(), &srvDesc, texture.textureView.GetAddressOf());

	return texture;
}

static mvCubeTexture
create_cube_map_from_hdr(mvGraphics& graphics, const std::string& path)
{
	mvCubeTexture texture{};
	Microsoft::WRL::ComPtr<ID3D11Texture2D> textureResource;

	mvVec4* surfaces[6];

	float gamma = 1.0f;
	float gamma_scale = 1.0f;

	// Load Image
	int texWidth, texHeight, texNumChannels;
	int texForceNumChannels = 4;
	float* testTextureBytes = stbi_loadf(path.c_str(), &texWidth, &texHeight, &texNumChannels, texForceNumChannels);
	assert(testTextureBytes);

	int pixels = texWidth * texHeight * 4 * 4;

	mvVec2 inUV = { 0.0f, 0.0f };
	int currentPixel = 0;

	int res = 512;
	float xinc = 1.0f / (float)res;
	float yinc = 1.0f / (float)res;
	for (int i = 0; i < 6; i++)
		surfaces[i] = new mvVec4[res * res];

	mvComputeShader shader = create_compute_shader(graphics, std::string(graphics.shaderDirectory) + "panorama_to_cube.hlsl");
	mvBuffer inputBuffer = create_buffer(graphics, testTextureBytes, texWidth * texHeight * 4 * sizeof(float), (D3D11_BIND_FLAG)(D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE), sizeof(float) * 4, D3D11_RESOURCE_MISC_BUFFER_STRUCTURED);
	mvBuffer faces[6];
	for (int i = 0; i < 6; i++)
		faces[i] = create_buffer(graphics, surfaces[i], res * res * sizeof(float) * 4, (D3D11_BIND_FLAG)(D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE), sizeof(float) * 4, D3D11_RESOURCE_MISC_BUFFER_STRUCTURED);

	struct MetaData
	{
		int resolution;
		unsigned int width;
		unsigned int height;
		unsigned int padding;
	};

	MetaData mdata{};
	mdata.resolution = res;
	mdata.width = texWidth;
	mdata.height = texHeight;

	mvConstBuffer cbuffer = create_const_buffer(graphics, &mdata, sizeof(MetaData));
	update_const_buffer(graphics, cbuffer, &mdata);
	ID3D11DeviceContext* ctx = graphics.imDeviceContext.Get();

	ctx->CSSetConstantBuffers(0u, 1u, cbuffer.buffer.GetAddressOf());
	ctx->CSSetUnorderedAccessViews(0u, 1u, inputBuffer.unorderedAccessView.GetAddressOf(), nullptr);
	for (int i = 0; i < 6; i++)
		ctx->CSSetUnorderedAccessViews(i + 1u, 1u, faces[i].unorderedAccessView.GetAddressOf(), nullptr);

	ctx->CSSetShader(shader.shader.Get(), nullptr, 0);
	ctx->Dispatch(res / 16, res / 16, 2u);

	for (int i = 0; i < 6; i++)
	{
		ID3D11Buffer* stagingBuffer;

		D3D11_BUFFER_DESC cbd;
		faces[i].buffer->GetDesc(&cbd);
		cbd.BindFlags = 0;
		cbd.MiscFlags = 0;
		cbd.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		cbd.Usage = D3D11_USAGE_STAGING;


		HRESULT hResult = graphics.device->CreateBuffer(&cbd, nullptr, &stagingBuffer);
		assert(SUCCEEDED(hResult));

		graphics.imDeviceContext->CopyResource(stagingBuffer, faces[i].buffer.Get());

		D3D11_MAPPED_SUBRESOURCE MappedResource;
		graphics.imDeviceContext->Map(stagingBuffer, 0, D3D11_MAP_READ, 0, &MappedResource);

		memcpy(surfaces[i], MappedResource.pData, res * res * sizeof(float) * 4);

		stagingBuffer->Release();
	}

	// texture descriptor
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = res;
	textureDesc.Height = res;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 6;
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	// subresource data
	D3D11_SUBRESOURCE_DATA data[6];
	for (int i = 0; i < 6; i++)
	{
		data[i].pSysMem = surfaces[i];
		data[i].SysMemPitch = res * 4 * 4;
		data[i].SysMemSlicePitch = 0;
	}
	// create the texture resource
	graphics.device->CreateTexture2D(&textureDesc, data, textureResource.GetAddressOf());

	// create the resource view on the texture
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;

	graphics.device->CreateShaderResourceView(textureResource.Get(), &srvDesc, texture.textureView.GetAddressOf());

	for (int i = 0; i < 6; i++)
		delete[] surfaces[i];
	return texture;

}

static void
copy_resource_to_cubemap(mvGraphics& graphics, ID3D11Texture2D* specularTextureResource, mvCubeTexture& dst, std::vector<mvVec4*>& surfaces, int width, int height, int mipSlice, int mipLevels)
{
	D3D11_BOX sourceRegion;
	for (int i = 0; i < 6; ++i)
	{
		sourceRegion.left = 0;
		sourceRegion.right = width;
		sourceRegion.top = 0;
		sourceRegion.bottom = height;
		sourceRegion.front = 0;
		sourceRegion.back = 1;

		// Create Texture
		D3D11_TEXTURE2D_DESC textureDesc = {};
		textureDesc.Width = width;
		textureDesc.Height = height;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		textureDesc.CPUAccessFlags = 0;

		Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
		graphics.device->CreateTexture2D(&textureDesc, nullptr, texture.GetAddressOf());
		graphics.imDeviceContext->UpdateSubresource(texture.Get(), 0u, nullptr, surfaces[i], 4 * width * sizeof(float), 0u);
		graphics.imDeviceContext->CopySubresourceRegion(specularTextureResource, D3D11CalcSubresource(mipSlice, i, mipLevels), 0, 0, 0, texture.Get(), 0, &sourceRegion);

	}
}

static std::vector<mvVec4*>*
create_single_specular_map(mvGraphics& graphics, mvCubeTexture& cubemap, int resolution, int width, int sampleCount, float lodBias, int currentMipLevel, int outputMipLevels)
{

	std::vector<mvVec4*>* surfaces = new std::vector<mvVec4*>(7);
	for (int i = 0; i < 6; i++)
		(*surfaces)[i] = new mvVec4[width * width];

	(*surfaces)[6] = new mvVec4[width * width];

	mvBuffer faces[6];
	for (int i = 0; i < 6; i++)
		faces[i] = create_buffer(graphics, (*surfaces)[i], width * width * sizeof(float) * 4, (D3D11_BIND_FLAG)(D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE), sizeof(float) * 4, D3D11_RESOURCE_MISC_BUFFER_STRUCTURED);

	mvBuffer LutBuffer = create_buffer(graphics, (*surfaces)[6], width * width * sizeof(float) * 4, (D3D11_BIND_FLAG)(D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE), sizeof(float) * 4, D3D11_RESOURCE_MISC_BUFFER_STRUCTURED);

	struct MetaData
	{
		int resolution;
		int width;
		float roughness;
		unsigned int sampleCount;

		unsigned int currentMipLevel;
		float lodBias;
		unsigned int distribution;
		unsigned int padding;
	};

	MetaData mdata{};
	mdata.resolution = resolution;
	mdata.width = width;
	mdata.sampleCount = sampleCount;
	mdata.distribution = 1;
	mdata.roughness = (float)currentMipLevel / (float)(outputMipLevels - 1);
	mdata.currentMipLevel = currentMipLevel;
	mdata.lodBias = lodBias;

	mvConstBuffer cbuffer = create_const_buffer(graphics, &mdata, sizeof(MetaData));
	update_const_buffer(graphics, cbuffer, &mdata);

	// texture descriptor
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = width;
	textureDesc.Height = width;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 6;
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	// filtering
	mvComputeShader filtershader = create_compute_shader(graphics, std::string(graphics.shaderDirectory) + "filter_environment.hlsl");
	D3D11_SAMPLER_DESC samplerDesc{};
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.BorderColor[0] = 0.0f;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.MaxAnisotropy = D3D11_REQ_MAXANISOTROPY;
	ID3D11SamplerState* sampler;
	graphics.device->CreateSamplerState(&samplerDesc, &sampler);

	ID3D11DeviceContext* ctx = graphics.imDeviceContext.Get();

	ctx->CSSetConstantBuffers(0u, 1u, cbuffer.buffer.GetAddressOf());
	for (int i = 0; i < 6; i++)
		ctx->CSSetUnorderedAccessViews(i, 1u, faces[i].unorderedAccessView.GetAddressOf(), nullptr);
	ctx->CSSetUnorderedAccessViews(6, 1u, LutBuffer.unorderedAccessView.GetAddressOf(), nullptr);
	ctx->CSSetShaderResources(0u, 1, cubemap.textureView.GetAddressOf());
	ctx->CSSetSamplers(0u, 1, &sampler);
	ctx->CSSetShader(filtershader.shader.Get(), nullptr, 0);
	unsigned int dispatchCount = width / 16;
	assert(dispatchCount > 0u);
	ctx->Dispatch(dispatchCount, dispatchCount, 2u);

	for (int i = 0; i < 6; i++)
	{
		ID3D11Buffer* stagingBuffer;

		D3D11_BUFFER_DESC cbd;
		faces[i].buffer->GetDesc(&cbd);
		cbd.BindFlags = 0;
		cbd.MiscFlags = 0;
		cbd.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		cbd.Usage = D3D11_USAGE_STAGING;

		HRESULT hResult = graphics.device->CreateBuffer(&cbd, nullptr, &stagingBuffer);
		assert(SUCCEEDED(hResult));

		graphics.imDeviceContext->CopyResource(stagingBuffer, faces[i].buffer.Get());

		D3D11_MAPPED_SUBRESOURCE MappedResource;
		hResult = graphics.imDeviceContext->Map(stagingBuffer, 0, D3D11_MAP_READ, 0, &MappedResource);
		assert(SUCCEEDED(hResult));

		memcpy((*surfaces)[i], MappedResource.pData, width * width * sizeof(float) * 4);

		stagingBuffer->Release();
	}

	if (currentMipLevel == 0)
	{
		ID3D11Buffer* stagingBuffer;

		D3D11_BUFFER_DESC cbd;
		LutBuffer.buffer->GetDesc(&cbd);
		cbd.BindFlags = 0;
		cbd.MiscFlags = 0;
		cbd.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		cbd.Usage = D3D11_USAGE_STAGING;

		HRESULT hResult = graphics.device->CreateBuffer(&cbd, nullptr, &stagingBuffer);
		assert(SUCCEEDED(hResult));

		graphics.imDeviceContext->CopyResource(stagingBuffer, LutBuffer.buffer.Get());

		D3D11_MAPPED_SUBRESOURCE MappedResource;
		hResult = graphics.imDeviceContext->Map(stagingBuffer, 0, D3D11_MAP_READ, 0, &MappedResource);
		assert(SUCCEEDED(hResult));

		memcpy((*surfaces)[6], MappedResource.pData, width * width * sizeof(float) * 4);

		stagingBuffer->Release();
	}

	sampler->Release();

	return surfaces;

}

static mvCubeTexture
create_irradiance_map(mvGraphics& graphics, mvCubeTexture& cubemap, int resolution, int sampleCount, float lodBias)
{

	mvCubeTexture texture{};

	mvVec4* surfaces[6];
	for (int i = 0; i < 6; i++)
		surfaces[i] = new mvVec4[resolution * resolution];

	mvBuffer faces[6];
	for (int i = 0; i < 6; i++)
		faces[i] = create_buffer(graphics, surfaces[i], resolution * resolution * sizeof(float) * 4, (D3D11_BIND_FLAG)(D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE), sizeof(float) * 4, D3D11_RESOURCE_MISC_BUFFER_STRUCTURED);

	struct MetaData
	{
		int resolution;
		unsigned int width;
		float roughness;
		unsigned int sampleCount;

		unsigned int currentMipLevel;
		float lodBias;
		unsigned int distribution;
		unsigned int padding;
	};

	MetaData mdata{};
	mdata.resolution = resolution;
	mdata.width = resolution;
	mdata.sampleCount = sampleCount;
	mdata.distribution = 0;
	mdata.roughness = 0.0f;
	mdata.currentMipLevel = 0;
	mdata.lodBias = lodBias;

	mvConstBuffer cbuffer = create_const_buffer(graphics, &mdata, sizeof(MetaData));
	update_const_buffer(graphics, cbuffer, &mdata);

	// texture descriptor
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = resolution;
	textureDesc.Height = resolution;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 6;
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	// filtering
	mvComputeShader filtershader = create_compute_shader(graphics, std::string(graphics.shaderDirectory) + "filter_environment.hlsl");
	D3D11_SAMPLER_DESC samplerDesc{};
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = samplerDesc.AddressU;
	samplerDesc.AddressW = samplerDesc.AddressU;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	//samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.MaxAnisotropy = D3D11_REQ_MAXANISOTROPY;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler;
	graphics.device->CreateSamplerState(&samplerDesc, sampler.GetAddressOf());

	ID3D11DeviceContext* ctx = graphics.imDeviceContext.Get();

	ctx->CSSetConstantBuffers(0u, 1u, cbuffer.buffer.GetAddressOf());
	for (int i = 0; i < 6; i++)
		ctx->CSSetUnorderedAccessViews(i, 1u, faces[i].unorderedAccessView.GetAddressOf(), nullptr);
	ctx->CSSetShaderResources(0u, 1, cubemap.textureView.GetAddressOf());
	ctx->CSSetSamplers(0u, 1, sampler.GetAddressOf());
	ctx->CSSetShader(filtershader.shader.Get(), nullptr, 0);
	unsigned int dispatchCount = resolution / 16;
	assert(dispatchCount > 0u);
	ctx->Dispatch(dispatchCount, dispatchCount, 2u);

	for (int i = 0; i < 6; i++)
	{
		ID3D11Buffer* stagingBuffer;

		D3D11_BUFFER_DESC cbd;
		faces[i].buffer->GetDesc(&cbd);
		cbd.BindFlags = 0;
		cbd.MiscFlags = 0;
		cbd.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		cbd.Usage = D3D11_USAGE_STAGING;

		HRESULT hResult = graphics.device->CreateBuffer(&cbd, nullptr, &stagingBuffer);
		assert(SUCCEEDED(hResult));

		graphics.imDeviceContext->CopyResource(stagingBuffer, faces[i].buffer.Get());

		D3D11_MAPPED_SUBRESOURCE MappedResource;
		hResult = graphics.imDeviceContext->Map(stagingBuffer, 0, D3D11_MAP_READ, 0, &MappedResource);
		assert(SUCCEEDED(hResult));

		memcpy(surfaces[i], MappedResource.pData, resolution * resolution * sizeof(float) * 4);

		stagingBuffer->Release();
	}

	// subresource data
	D3D11_SUBRESOURCE_DATA data[6];
	for (int i = 0; i < 6; i++)
	{
		data[i].pSysMem = surfaces[i];
		data[i].SysMemPitch = resolution * 4 * 4;
		data[i].SysMemSlicePitch = 0;
	}

	// create the resource view on the texture
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = textureDesc.MipLevels;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> textureResource;
	graphics.device->CreateTexture2D(&textureDesc, data, textureResource.GetAddressOf());
	graphics.device->CreateShaderResourceView(textureResource.Get(), &srvDesc, texture.textureView.GetAddressOf());

	for (int i = 0; i < 6; i++)
		delete[] surfaces[i];
	return texture;

}

mvEnvironment 
create_environment(mvGraphics& graphics, const std::string& path, int resolution, int sampleCount, float lodBias, int mipLevels)
{

    mvEnvironment environment{};

	environment.skyMap = create_cube_map_from_hdr(graphics, path);

    // create environment sampler
    D3D11_SAMPLER_DESC envSamplerDesc{};
    envSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    envSamplerDesc.AddressV = envSamplerDesc.AddressU;
    envSamplerDesc.AddressW = envSamplerDesc.AddressU;
    envSamplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
    envSamplerDesc.MaxAnisotropy = D3D11_REQ_MAXANISOTROPY;
    envSamplerDesc.MinLOD = 0.0f;
    envSamplerDesc.MaxLOD = (float)mipLevels + 1.0f;
    graphics.device->CreateSamplerState(&envSamplerDesc, environment.sampler.GetAddressOf());

	// create brdfLUT sampler
	D3D11_SAMPLER_DESC brdfLutSamplerDesc{};
	brdfLutSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	brdfLutSamplerDesc.AddressV = brdfLutSamplerDesc.AddressU;
	brdfLutSamplerDesc.AddressW = brdfLutSamplerDesc.AddressU;
	brdfLutSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	brdfLutSamplerDesc.MaxAnisotropy = D3D11_REQ_MAXANISOTROPY;
	graphics.device->CreateSamplerState(&brdfLutSamplerDesc, environment.brdfSampler.GetAddressOf());

	// create irradianceMap
	environment.irradianceMap = create_irradiance_map(graphics, environment.skyMap, resolution, sampleCount, 0.0f);

	// initial specular map
	D3D11_TEXTURE2D_DESC texDesc = {};
	texDesc.Width = resolution;
	texDesc.Height = resolution;
	texDesc.MipLevels = mipLevels;
	texDesc.ArraySize = 6;
	texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	HRESULT hResult = graphics.device->CreateTexture2D(&texDesc, nullptr, environment.specularTextureResource.GetAddressOf());
	assert(SUCCEEDED(hResult));

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MipLevels = texDesc.MipLevels;
	srvDesc.TextureCube.MostDetailedMip = 0;

	hResult = graphics.device->CreateShaderResourceView(environment.specularTextureResource.Get(), &srvDesc, environment.specularMap.textureView.GetAddressOf());
	assert(SUCCEEDED(hResult));

	for (int i = mipLevels - 1; i != -1; i--)
	{
		int currentWidth = resolution >> i;
		std::vector<mvVec4*>* faces = create_single_specular_map(graphics, environment.skyMap, resolution, currentWidth, sampleCount, lodBias, i, mipLevels);
		copy_resource_to_cubemap(graphics, environment.specularTextureResource.Get(), environment.specularMap, *faces, currentWidth, currentWidth, i, mipLevels);

		if (i == 0)
		{
			// Create Texture
			D3D11_TEXTURE2D_DESC textureDesc = {};
			textureDesc.Width = currentWidth;
			textureDesc.Height = currentWidth;
			textureDesc.MipLevels = 1;
			textureDesc.ArraySize = 1;
			textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			textureDesc.SampleDesc.Count = 1;
			textureDesc.SampleDesc.Quality = 0;
			textureDesc.Usage = D3D11_USAGE_DEFAULT;
			textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			textureDesc.CPUAccessFlags = 0;

			Microsoft::WRL::ComPtr<ID3D11Texture2D> textureResource2;
			HRESULT hResult = graphics.device->CreateTexture2D(&textureDesc, nullptr, textureResource2.GetAddressOf());
			assert(SUCCEEDED(hResult));
			graphics.imDeviceContext->UpdateSubresource(textureResource2.Get(), 0u, nullptr, (*faces)[6], 4 * currentWidth * sizeof(float), 0u);

			// create the resource view on the texture
			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Format = textureDesc.Format;
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Texture2D.MipLevels = -1;

			hResult = graphics.device->CreateShaderResourceView(textureResource2.Get(), &srvDesc, environment.brdfLUT.textureView.GetAddressOf());
			assert(SUCCEEDED(hResult));
		}

		for (int i = 0; i < 7; i++)
		{
			delete[](*faces)[i];
		}

		delete faces;

	}

    return environment;
}

void
cleanup_environment(mvEnvironment& environment)
{

	environment.skyMap.textureView = nullptr;
	environment.irradianceMap.textureView = nullptr;
	environment.specularMap.textureView = nullptr;
	environment.brdfLUT.textureView = nullptr;
	environment.sampler = nullptr;
	environment.brdfSampler = nullptr;
	environment.specularTextureResource = nullptr;
}

static std::wstring
ToWide(const std::string& narrow)
{
	wchar_t wide[1024];
	mbstowcs_s(nullptr, wide, narrow.c_str(), _TRUNCATE);
	return wide;
}

mvComputeShader
create_compute_shader(mvGraphics& graphics, const std::string& path, std::vector<D3D_SHADER_MACRO>* macros)
{
	mvComputeShader shader{};
	shader.path = path;

	Microsoft::WRL::ComPtr<ID3DBlob> shaderCompileErrorsBlob;
	HRESULT hResult = D3DCompileFromFile(ToWide(path).c_str(),
		macros ? macros->data() : nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "cs_5_0", 0, 0,
		shader.blob.GetAddressOf(), shaderCompileErrorsBlob.GetAddressOf());

	if (FAILED(hResult))
	{
		const char* errorString = NULL;
		if (hResult == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
			errorString = "Could not compile shader; file not found";
		else if (shaderCompileErrorsBlob)
			errorString = (const char*)shaderCompileErrorsBlob->GetBufferPointer();

		MessageBoxA(0, errorString, "Shader Compiler Error", MB_ICONERROR | MB_OK);
	}

	hResult = graphics.device->CreateComputeShader(shader.blob->GetBufferPointer(), shader.blob->GetBufferSize(), nullptr, shader.shader.GetAddressOf());
	assert(SUCCEEDED(hResult));
	return shader;
}

mvPixelShader
create_pixel_shader(mvGraphics& graphics, const std::string& path, std::vector<D3D_SHADER_MACRO>* macros)
{
	mvPixelShader shader{};
	shader.path = path;

	Microsoft::WRL::ComPtr<ID3DBlob> shaderCompileErrorsBlob;
	HRESULT hResult = D3DCompileFromFile(
		ToWide(path).c_str(),
		macros ? macros->data() : nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_0", 0, 0,
		shader.blob.GetAddressOf(), shaderCompileErrorsBlob.GetAddressOf());

	if (FAILED(hResult))
	{
		const char* errorString = NULL;
		if (hResult == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
			errorString = "Could not compile shader; file not found";
		else if (shaderCompileErrorsBlob)
			errorString = (const char*)shaderCompileErrorsBlob->GetBufferPointer();

		MessageBoxA(0, errorString, "Shader Compiler Error", MB_ICONERROR | MB_OK);
	}

	hResult = graphics.device->CreatePixelShader(shader.blob->GetBufferPointer(), shader.blob->GetBufferSize(), nullptr, shader.shader.GetAddressOf());
	assert(SUCCEEDED(hResult));
	return shader;
}

mvVertexShader
create_vertex_shader(mvGraphics& graphics, const std::string& path, mvVertexLayout& layout, std::vector<D3D_SHADER_MACRO>* macros)
{
	mvVertexShader shader{};
	shader.path = path;

	Microsoft::WRL::ComPtr<ID3DBlob> shaderCompileErrorsBlob;
	HRESULT hResult = D3DCompileFromFile(ToWide(path).c_str(),
		macros ? macros->data() : nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_0",
		0, 0, shader.blob.GetAddressOf(), shaderCompileErrorsBlob.GetAddressOf());

	if (FAILED(hResult))
	{
		const char* errorString = NULL;
		if (hResult == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
			errorString = "Could not compile shader; file not found";
		else if (shaderCompileErrorsBlob)
			errorString = (const char*)shaderCompileErrorsBlob->GetBufferPointer();

		MessageBoxA(0, errorString, "Shader Compiler Error", MB_ICONERROR | MB_OK);
	}

	hResult = graphics.device->CreateVertexShader(shader.blob->GetBufferPointer(),
		shader.blob->GetBufferSize(), nullptr, shader.shader.GetAddressOf());

	assert(SUCCEEDED(hResult));

	for (int i = 0; i < layout.semantics.size(); i++)
	{
		layout.d3dLayout.push_back(D3D11_INPUT_ELEMENT_DESC{
			layout.semantics[i].c_str(),
			layout.indices[i],
			layout.formats[i],
			0,
			D3D11_APPEND_ALIGNED_ELEMENT,
			D3D11_INPUT_PER_VERTEX_DATA,
			0
			});
	}

	hResult = graphics.device->CreateInputLayout(layout.d3dLayout.data(),
		(uint32_t)layout.d3dLayout.size(),
		shader.blob->GetBufferPointer(),
		shader.blob->GetBufferSize(),
		shader.inputLayout.GetAddressOf());

	assert(SUCCEEDED(hResult));

	return shader;
}

struct mvVertexElementTemp
{
    int            itemCount = 0;
    bool           normalize = false;
    size_t         size = 0;
    size_t         index = 0;
    size_t         offset = 0;
    DXGI_FORMAT    format = DXGI_FORMAT_R32G32_FLOAT;
    std::string    semantic;
    mvVertexElement type;
};

static mvVertexElementTemp
mvGetVertexElementInfo(mvVertexElement element)
{
	mvVertexElementTemp newelement{};

	switch (element)
	{

	case Position2D:
		newelement.format = DXGI_FORMAT_R32G32_FLOAT;
		newelement.itemCount = 2;
		newelement.normalize = false;
		newelement.size = sizeof(float) * newelement.itemCount;
		newelement.semantic = "Position";
		break;

	case Position3D:
		newelement.format = DXGI_FORMAT_R32G32B32_FLOAT;
		newelement.itemCount = 3;
		newelement.normalize = false;
		newelement.size = sizeof(float) * newelement.itemCount;
		newelement.semantic = "Position";
		break;

	case TexCoord0:
		newelement.format = DXGI_FORMAT_R32G32_FLOAT;
		newelement.itemCount = 2;
		newelement.index = 0;
		newelement.normalize = false;
		newelement.size = sizeof(float) * newelement.itemCount;
		newelement.semantic = "TexCoord";
		break;

	case TexCoord1:
		newelement.format = DXGI_FORMAT_R32G32_FLOAT;
		newelement.itemCount = 2;
		newelement.index = 1;
		newelement.normalize = false;
		newelement.size = sizeof(float) * newelement.itemCount;
		newelement.semantic = "TexCoord";
		break;

	case Color3_0:
		newelement.format = DXGI_FORMAT_R32G32B32_FLOAT;
		newelement.itemCount = 3;
		newelement.index = 0;
		newelement.normalize = false;
		newelement.size = sizeof(float) * newelement.itemCount;
		newelement.semantic = "Color";
		break;

	case Color4_0:
		newelement.format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		newelement.itemCount = 4;
		newelement.index = 0;
		newelement.normalize = false;
		newelement.size = sizeof(float) * newelement.itemCount;
		newelement.semantic = "Color";
		break;

	case Color3_1:
		newelement.format = DXGI_FORMAT_R32G32B32_FLOAT;
		newelement.itemCount = 3;
		newelement.index = 1;
		newelement.normalize = false;
		newelement.size = sizeof(float) * newelement.itemCount;
		newelement.semantic = "Color";
		break;

	case Color4_1:
		newelement.format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		newelement.itemCount = 4;
		newelement.index = 1;
		newelement.normalize = false;
		newelement.size = sizeof(float) * newelement.itemCount;
		newelement.semantic = "Color";
		break;

	case Normal:
		newelement.format = DXGI_FORMAT_R32G32B32_FLOAT;
		newelement.itemCount = 3;
		newelement.normalize = false;
		newelement.size = sizeof(float) * newelement.itemCount;
		newelement.semantic = "Normal";
		break;

	case Tangent:
		newelement.format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		newelement.itemCount = 4;
		newelement.normalize = false;
		newelement.size = sizeof(float) * newelement.itemCount;
		newelement.semantic = "Tangent";
		break;

	case Joints0:
		newelement.format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		newelement.itemCount = 4;
		newelement.index = 0;
		newelement.normalize = false;
		newelement.size = sizeof(float) * newelement.itemCount;
		newelement.semantic = "Joints";
		break;

	case Joints1:
		newelement.format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		newelement.itemCount = 4;
		newelement.index = 1;
		newelement.normalize = false;
		newelement.size = sizeof(float) * newelement.itemCount;
		newelement.semantic = "Joints";
		break;

	case Weights0:
		newelement.format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		newelement.itemCount = 4;
		newelement.index = 0;
		newelement.normalize = false;
		newelement.size = sizeof(float) * newelement.itemCount;
		newelement.semantic = "Weights";
		break;

	case Weights1:
		newelement.format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		newelement.itemCount = 4;
		newelement.index = 1;
		newelement.normalize = false;
		newelement.size = sizeof(float) * newelement.itemCount;
		newelement.semantic = "Weights";
		break;

	}

	newelement.type = element;
	return newelement;
}

mvVertexLayout
create_vertex_layout(std::vector<mvVertexElement> elements)
{
	mvVertexLayout layout{};

	std::vector<mvVertexElementTemp> newelements;

	uint32_t stride = 0u;
	uint32_t size = 0u;

	for (auto& element : elements)
	{
		newelements.push_back(mvGetVertexElementInfo(element));
		newelements.back().offset = stride;
		layout.indices.push_back(newelements.back().index);
		layout.semantics.push_back(newelements.back().semantic);
		layout.formats.push_back(newelements.back().format);
		stride += newelements.back().size;
		size += newelements.back().size;
		layout.elementCount += newelements.back().itemCount;
	}

	layout.size = size;
	//layout.stride = stride;

	return layout;
}

mvVertexElement
get_element_from_gltf_semantic(const char* semantic)
{
	if (strcmp(semantic, "POSITION") == 0)   return Position3D;
	if (strcmp(semantic, "NORMAL") == 0)     return Normal;
	if (strcmp(semantic, "TANGENT") == 0)    return Tangent;
	if (strcmp(semantic, "JOINTS_0") == 0)   return Joints0;
	if (strcmp(semantic, "JOINTS_1") == 0)   return Joints1;
	if (strcmp(semantic, "WEIGHTS_0") == 0)  return Weights0;
	if (strcmp(semantic, "WEIGHTS_1") == 0)  return Weights0;
	if (strcmp(semantic, "TEXCOORD_0") == 0) return TexCoord0;
	if (strcmp(semantic, "TEXCOORD_1") == 0) return TexCoord0;
	if (strcmp(semantic, "COLOR_03") == 0)   return Color3_0;
	if (strcmp(semantic, "COLOR_04") == 0)   return Color4_0;
	if (strcmp(semantic, "COLOR_13") == 0)   return Color3_1;
	if (strcmp(semantic, "COLOR_14") == 0)   return Color4_1;
	assert(false && "Undefined semantic");
	return Position3D;
}

bool
operator==(mvVertexLayout& left, mvVertexLayout& right)
{
	// TODO: use bit flags for formats so this can be quicker

	if (left.size != right.size)
		return false;
	if (left.formats.size() != right.formats.size())
		return false;

	for (size_t i = 0; i < left.formats.size(); i++)
	{
		if (left.formats[i] != right.formats[i])
			return false;
	}

	return true;
}

bool
operator!=(mvVertexLayout& left, mvVertexLayout& right)
{
	return !(left == right);
}

mvPipeline
finalize_pipeline(mvGraphics& graphics, mvPipelineInfo& info)
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

    graphics.device->CreateRasterizerState(&rasterDesc, pipeline.rasterizationState.GetAddressOf());

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

    graphics.device->CreateDepthStencilState(&dsDesc, pipeline.depthStencilState.GetAddressOf());

    D3D11_BLEND_DESC blendDesc = CD3D11_BLEND_DESC{ CD3D11_DEFAULT{} };
    auto& brt = blendDesc.RenderTarget[0];
    brt.BlendEnable = TRUE;
    brt.SrcBlend = D3D11_BLEND_SRC_ALPHA;
    brt.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    graphics.device->CreateBlendState(&blendDesc, pipeline.blendState.GetAddressOf());

    std::vector<D3D_SHADER_MACRO> macros;
    for (auto& mac : info.macros)
        macros.push_back({ mac.macro.c_str(), mac.value.c_str()});

    macros.push_back({ NULL, NULL });

    if (!info.pixelShader.empty())
    {
        mvPixelShader pixelShader = create_pixel_shader(graphics, std::string(graphics.shaderDirectory) + info.pixelShader, macros.empty() ? nullptr : &macros);
        pipeline.pixelShader = pixelShader.shader;
        pipeline.pixelBlob = pixelShader.blob;
    }

    mvVertexShader vertexShader = create_vertex_shader(graphics, std::string(graphics.shaderDirectory) + info.vertexShader, info.layout, macros.empty() ? nullptr : &macros);

    pipeline.vertexShader = vertexShader.shader;
    pipeline.vertexBlob = vertexShader.blob;
    pipeline.inputLayout = vertexShader.inputLayout;

    return pipeline;
}
