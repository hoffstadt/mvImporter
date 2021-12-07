#pragma once

#include "mvMath.h"
#include "mvTimer.h"
#include "mvTypes.h"
#include "mvViewport.h"
#include "mvAssetManager.h"
#include "mvBuffers.h"
#include "mvCamera.h"
#include "mvGraphics.h"
#include "mvMaterials.h"
#include "mvMesh.h"
#include "mvPipeline.h"
#include "mvRenderer.h"
#include "mvScene.h"
#include "mvTextures.h"
#include "mvVertexLayout.h"
#include "mvImporter.h"
#include <imgui.h>
#include <implot.h>
#include "mvWindows.h"
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>

//-----------------------------------------------------------------------------
// constant buffer data structs
//-----------------------------------------------------------------------------

struct GlobalInfo
{

    mvVec3 ambientColor = { 0.04f, 0.04f, 0.04f };
    b32    useShadows = true;
    //-------------------------- ( 16 bytes )

    b32  useSkybox = true;
    char _pad[12];
    //-------------------------- ( 2*16 = 32 bytes )
};

struct PointLightInfo
{

    mvVec4 viewLightPos = { 0.0f, 15.0f, 0.0f, 1.0f };
    //-------------------------- ( 16 bytes )

    mvVec3 diffuseColor = { 1.0f, 1.0f, 1.0f };
    f32    diffuseIntensity = 1.0f;
    //-------------------------- ( 16 bytes )

    f32  attConst = 1.0f;
    f32  attLin = 0.045f;
    f32  attQuad = 0.0075f;
    char _pad1[4];
    //-------------------------- ( 16 bytes )

    //-------------------------- ( 4*16 = 64 bytes )
};

struct DirectionLightInfo
{

    f32    diffuseIntensity = 1.0f;
    mvVec3 viewLightDir = { 0.0f, -1.0f, 0.0f };
    //-------------------------- ( 16 bytes )

    mvVec3 diffuseColor = { 1.0f, 1.0f, 1.0f };
    f32    padding = 0.0f;
    //-------------------------- ( 16 bytes )

    //-------------------------- ( 2*16 = 32 bytes )
};

struct DirectionalShadowTransformInfo
{
    mvMat4 view = mvIdentityMat4();
    mvMat4 projection = mvIdentityMat4();
};

struct OmniShadowTransformInfo
{
    mvMat4 view = mvIdentityMat4();
};

//-----------------------------------------------------------------------------
// shadow helpers
//-----------------------------------------------------------------------------

struct mvSkybox
{
    ID3D11SamplerState* cubeSampler = nullptr;
    mvBuffer            vertexBuffer;
    mvBuffer            indexBuffer;
    mvVertexLayout      vertexLayout;
    mvCubeTexture       cubeTexture;
    mvPipeline          pipeline;

    mvSkybox()
    {
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

        GContext->graphics.device->CreateDepthStencilState(&dsDesc, &pipeline.depthStencilState);
        GContext->graphics.device->CreateRasterizerState(&skyboxRasterDesc, &pipeline.rasterizationState);
        GContext->graphics.device->CreateBlendState(&blendDesc, &pipeline.blendState);

        mvPixelShader pixelShader = mvCreatePixelShader(GContext->IO.shaderDirectory + "Skybox_PS.hlsl");
        mvVertexShader vertexShader = mvCreateVertexShader(GContext->IO.shaderDirectory + "Skybox_VS.hlsl", mvCreateVertexLayout({ mvVertexElement::Position3D }));

        pipeline.pixelShader = pixelShader.shader;
        pipeline.pixelBlob = pixelShader.blob;
        pipeline.vertexShader = vertexShader.shader;
        pipeline.vertexBlob = vertexShader.blob;
        pipeline.inputLayout = vertexShader.inputLayout;
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

        vertexBuffer = mvCreateBuffer(vertices.data(), vertices.size() * sizeof(f32), D3D11_BIND_VERTEX_BUFFER);
        indexBuffer = mvCreateBuffer(indices.data(), indices.size() * sizeof(u32), D3D11_BIND_INDEX_BUFFER);
        vertexLayout = mvCreateVertexLayout({ mvVertexElement::Position3D });
        cubeTexture = mvCreateCubeTexture("../../Resources/Skybox");

        D3D11_SAMPLER_DESC samplerDesc{};
        samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
        samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
        samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
        samplerDesc.BorderColor[0] = 0.0f;
        samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        samplerDesc.MaxAnisotropy = D3D11_REQ_MAXANISOTROPY;
        GContext->graphics.device->CreateSamplerState(&samplerDesc, &cubeSampler);
    }
};

struct mvShadowMap
{
    ID3D11Texture2D*               texture;
    ID3D11DepthStencilView*        depthView;
    ID3D11ShaderResourceView*      resourceView;
    ID3D11RasterizerState*         rasterizationState;
    ID3D11SamplerState*            sampler;
    mvOrthoCamera                  camera;
    D3D11_VIEWPORT                 viewport;
    mvConstBuffer                  buffer;
    DirectionalShadowTransformInfo info;

    mvShadowMap(u32 resolution, f32 width)
    {
        viewport = { 0.0f, 0.0f, (f32)resolution, (f32)resolution, 0.0f, 1.0f };

        buffer = mvCreateConstBuffer(&info, sizeof(DirectionalShadowTransformInfo));

        // setup camera
        camera.dir = { 0.0f, 1.0f, 0.0f };
        camera.up = { 1.0f, 0.0f, 0.0f };
        camera.pos.y = width / 2.0f;
        camera.left = -width;
        camera.right = width;
        camera.bottom = -width;
        camera.top = width;
        camera.nearZ = -width - 1.0f;
        camera.farZ = width + 1.0f;

        D3D11_TEXTURE2D_DESC shadowMapDesc;
        ZeroMemory(&shadowMapDesc, sizeof(D3D11_TEXTURE2D_DESC));
        shadowMapDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
        shadowMapDesc.MipLevels = 1;
        shadowMapDesc.ArraySize = 1;
        shadowMapDesc.SampleDesc.Count = 1;
        shadowMapDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
        shadowMapDesc.Height = static_cast<UINT>(resolution);
        shadowMapDesc.Width = static_cast<UINT>(resolution);

        HRESULT hr = GContext->graphics.device->CreateTexture2D(
            &shadowMapDesc,
            nullptr,
            &texture
        );

        D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
        ZeroMemory(&depthStencilViewDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
        depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        depthStencilViewDesc.Texture2D.MipSlice = 0;

        D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
        ZeroMemory(&shaderResourceViewDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
        shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        shaderResourceViewDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
        shaderResourceViewDesc.Texture2D.MipLevels = 1;

        GContext->graphics.device->CreateDepthStencilView(
            texture,
            &depthStencilViewDesc,
            &depthView
        );

        GContext->graphics.device->CreateShaderResourceView(
            texture,
            &shaderResourceViewDesc,
            &resourceView
        );

        D3D11_RASTERIZER_DESC shadowRenderStateDesc;
        ZeroMemory(&shadowRenderStateDesc, sizeof(D3D11_RASTERIZER_DESC));
        shadowRenderStateDesc.CullMode = D3D11_CULL_FRONT;
        shadowRenderStateDesc.FrontCounterClockwise = TRUE;
        shadowRenderStateDesc.FillMode = D3D11_FILL_SOLID;
        shadowRenderStateDesc.DepthClipEnable = true;
        shadowRenderStateDesc.DepthBias = 50;
        shadowRenderStateDesc.DepthBiasClamp = 0.1f;
        shadowRenderStateDesc.SlopeScaledDepthBias = 2.0f;

        GContext->graphics.device->CreateRasterizerState(&shadowRenderStateDesc, &rasterizationState);

        D3D11_SAMPLER_DESC comparisonSamplerDesc;
        ZeroMemory(&comparisonSamplerDesc, sizeof(D3D11_SAMPLER_DESC));
        comparisonSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
        comparisonSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
        comparisonSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        comparisonSamplerDesc.BorderColor[0] = 1.0f;
        comparisonSamplerDesc.BorderColor[1] = 1.0f;
        comparisonSamplerDesc.BorderColor[2] = 1.0f;
        comparisonSamplerDesc.BorderColor[3] = 1.0f;
        comparisonSamplerDesc.MinLOD = 0.f;
        comparisonSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
        comparisonSamplerDesc.MipLODBias = 0.f;
        comparisonSamplerDesc.MaxAnisotropy = 0;
        comparisonSamplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
        comparisonSamplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;

        GContext->graphics.device->CreateSamplerState(&comparisonSamplerDesc, &sampler);

        info.view = getViewMatrix();
        info.projection = getProjectionMatrix();
    }

    mvMat4 getViewMatrix()
    {
        return mvLookAtRH(camera.pos, camera.pos + camera.dir, camera.up);
    }

    mvMat4 getProjectionMatrix()
    {
        return mvOrthoRH(camera.left, camera.right, camera.bottom, camera.top, camera.nearZ, camera.farZ);
    }

    void showControls()
    {
        ImGui::Begin("Direction Light");
        if (ImGui::SliderFloat3("DLight", &camera.dir.x, -1.0f, 1.0f))
        {
            info.view = mvLookAtRH(camera.pos, camera.pos + camera.dir, camera.up);
            info.projection = mvOrthoRH(camera.left, camera.right, camera.bottom, camera.top, camera.nearZ, camera.farZ);
        }
        ImGui::End();
    }
};

struct mvShadowCubeMap
{
    ID3D11Texture2D*          texture = nullptr;
    ID3D11DepthStencilView*   depthView[6]{};
    ID3D11ShaderResourceView* resourceView = nullptr;
    mvVec3                    cameraDirections[6];
    mvVec3                    cameraUps[6];
    ID3D11SamplerState*       sampler = nullptr;
    D3D11_VIEWPORT            viewport;
    mvConstBuffer             buffer;
    OmniShadowTransformInfo   info;

    mvShadowCubeMap(u32 resolution)
    {
        viewport = { 0.0f, 0.0f, (f32)resolution, (f32)resolution, 0.0f, 1.0f };

        // texture descriptor
        D3D11_TEXTURE2D_DESC textureDesc = {};
        textureDesc.Width = resolution;
        textureDesc.Height = resolution;
        textureDesc.MipLevels = 1;
        textureDesc.ArraySize = 6;
        textureDesc.Format = DXGI_FORMAT_R32_TYPELESS;
        textureDesc.SampleDesc.Count = 1;
        textureDesc.SampleDesc.Quality = 0;
        textureDesc.Usage = D3D11_USAGE_DEFAULT;
        textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
        textureDesc.CPUAccessFlags = 0;
        textureDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

        // create the texture resource
        mvComPtr<ID3D11Texture2D> pTexture;
        GContext->graphics.device->CreateTexture2D(&textureDesc, nullptr, &pTexture);

        // create the resource view on the texture
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.MipLevels = 1;
        GContext->graphics.device->CreateShaderResourceView(pTexture.Get(), &srvDesc, &resourceView);

        // make render target resources for capturing shadow map
        for (u32 face = 0; face < 6; face++)
        {
            // create target view of depth stensil texture
            D3D11_DEPTH_STENCIL_VIEW_DESC descView = {};
            descView.Format = DXGI_FORMAT_D32_FLOAT;
            descView.Flags = 0;
            descView.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
            descView.Texture2DArray.MipSlice = 0;
            descView.Texture2DArray.ArraySize = 1;
            descView.Texture2DArray.FirstArraySlice = face;
            GContext->graphics.device->CreateDepthStencilView(pTexture.Get(), &descView, &(depthView[face]));
        }

        cameraDirections[0] = { 0.0f,  0.0f,  -1.0f };
        cameraDirections[1] = { 0.0f,  0.0f,  1.0f };
        cameraDirections[2] = { 0.0f,  1.0f,  0.0f };
        cameraDirections[3] = { 0.0f,  -1.0f,  0.0f };
        cameraDirections[4] = { 1.0f,  0.0f,  0.0f };
        cameraDirections[5] = { -1.0f,  0.0f,   0.0f };

        cameraUps[0] = { 0.0f,  1.0f,  0.0f };
        cameraUps[1] = { 0.0f,  1.0f,  0.0f };
        cameraUps[2] = { 1.0f, 0.0f,   0.0f };
        cameraUps[3] = { 1.0f, 0.0f,   0.0f };
        cameraUps[4] = { 0.0f,  1.0f,  0.0f };
        cameraUps[5] = { 0.0f,  1.0f,  0.0f };

        D3D11_SAMPLER_DESC comparisonSamplerDesc;
        ZeroMemory(&comparisonSamplerDesc, sizeof(D3D11_SAMPLER_DESC));
        comparisonSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
        comparisonSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
        comparisonSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        comparisonSamplerDesc.BorderColor[0] = 1.0f;
        comparisonSamplerDesc.BorderColor[1] = 1.0f;
        comparisonSamplerDesc.BorderColor[2] = 1.0f;
        comparisonSamplerDesc.BorderColor[3] = 1.0f;
        comparisonSamplerDesc.MinLOD = 0.f;
        comparisonSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
        comparisonSamplerDesc.MipLODBias = 0.f;
        comparisonSamplerDesc.MaxAnisotropy = 0;
        comparisonSamplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
        comparisonSamplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
        GContext->graphics.device->CreateSamplerState(&comparisonSamplerDesc, &sampler);

        buffer = mvCreateConstBuffer(&info, sizeof(OmniShadowTransformInfo));

    }
};

//-----------------------------------------------------------------------------
// helper functions
//-----------------------------------------------------------------------------

mvGLTFModel 
LoadTestModel(const char* name)
{
    static const char* gltfPath = "../../data/glTF-Sample-Models/2.0/";
    std::string root = gltfPath + std::string(name) + "//glTF-Embedded//";
    std::string file = root + std::string(name) + ".gltf";
    return mvLoadGLTF(root.c_str(), file.c_str());
}

//-----------------------------------------------------------------------------
// helper classes
//-----------------------------------------------------------------------------

struct mvOffscreen
{
    ID3D11Texture2D* texture;
    ID3D11Texture2D* depthTexture;
    ID3D11RenderTargetView*   targetView = nullptr;
    ID3D11DepthStencilView*   depthView = nullptr;
    ID3D11ShaderResourceView* resourceView = nullptr;
    D3D11_VIEWPORT            viewport;

    mvOffscreen(float width, float height)
    {
        viewport = { 0.0f, 0.0f, width, height, 0.0f, 1.0f };

        resize(width, height);
    }

    void resize(float width, float height)
    {


        // render target texture
        {
            D3D11_TEXTURE2D_DESC textureDesc{};
            textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            textureDesc.MipLevels = 1;
            textureDesc.ArraySize = 1;
            textureDesc.SampleDesc.Count = 1;
            textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
            textureDesc.Height = static_cast<UINT>(height);
            textureDesc.Width = static_cast<UINT>(width);

            GContext->graphics.device->CreateTexture2D(
                &textureDesc,
                nullptr,
                &texture
            );
        }

        // depth stencil texture
        {
            D3D11_TEXTURE2D_DESC textureDesc{};
            textureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
            textureDesc.MipLevels = 1;
            textureDesc.ArraySize = 1;
            textureDesc.SampleDesc.Count = 1;
            textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
            textureDesc.Height = static_cast<UINT>(height);
            textureDesc.Width = static_cast<UINT>(width);

            GContext->graphics.device->CreateTexture2D(
                &textureDesc,
                nullptr,
                &depthTexture
            );
        }

        // render target view
        {
            D3D11_RENDER_TARGET_VIEW_DESC depthStencilViewDesc{};
            depthStencilViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            depthStencilViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
            depthStencilViewDesc.Texture2D.MipSlice = 0;

            GContext->graphics.device->CreateRenderTargetView(
                texture,
                &depthStencilViewDesc,
                &targetView
            );
        }

        // depth stencil view
        {
            D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
            depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
            depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
            depthStencilViewDesc.Texture2D.MipSlice = 0;

            GContext->graphics.device->CreateDepthStencilView(
                depthTexture,
                &depthStencilViewDesc,
                &depthView
            );
        }

        // shader resource view for render target
        {
            D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc{};
            shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            shaderResourceViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            shaderResourceViewDesc.Texture2D.MipLevels = 1;

            GContext->graphics.device->CreateShaderResourceView(
                texture,
                &shaderResourceViewDesc,
                &resourceView
            );
        }
    }
};

struct mvPointLight
{
    mvCamera       camera{};
    mvConstBuffer  buffer{};
    PointLightInfo info{};
    mvMesh         mesh{};
    mvPipeline     pipeline{};

    mvPointLight(mvAssetManager& am)
    {
        // setup camera
        camera.aspect = 1.0f;
        camera.yaw = 0.0f;
        camera.pitch = 0.0f;
        camera.pos = info.viewLightPos.xyz();

        // create mesh
        mesh = mvCreateCube(am, 0.25f);

        // create constant buffer
        buffer = mvCreateConstBuffer(&info, sizeof(PointLightInfo));

        // create pipeline
        pipeline.topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

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
    }

    bool showControls()
    {
        bool modified = false;
        ImGui::Begin("Point Light");
        if (ImGui::SliderFloat3("Position", &info.viewLightPos.x, -25.0f, 50.0f))
        {
            camera.pos = { info.viewLightPos.x, info.viewLightPos.y, info.viewLightPos.z };
            modified = true;
        }
        ImGui::End();

        return modified;
    }

};