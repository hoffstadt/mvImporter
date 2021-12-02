#include "mv3D.h"
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>

struct Mesh
{
    mvBuffer       indexBuffer;
    mvBuffer       vertexBuffer;
    mvVertexLayout vertexLayout;
};

struct TransformInfo
{
    mvMat4 modelView = mvIdentityMat4();
    mvMat4 modelViewProj = mvIdentityMat4();
};

struct PointLightInfo
{

    mvVec4 viewLightPos = { -3.0f, 5.0f, -3.0f, 1.0f };
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

mvPipeline create_light_pipeline()
{
    mvPipeline pipeline{};

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

    return pipeline;
}

mvPipeline create_phong_pipeline()
{
    mvPipeline pipeline{};

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

    mvPixelShader pixelShader = mvCreatePixelShader(GContext->IO.shaderDirectory + "Example1_PS.hlsl");
    mvVertexShader vertexShader = mvCreateVertexShader(GContext->IO.shaderDirectory + "Example1_VS.hlsl", mvCreateVertexLayout({ mvVertexElement::Position3D,mvVertexElement::Normal }));

    pipeline.pixelShader = pixelShader.shader;
    pipeline.pixelBlob = pixelShader.blob;
    pipeline.vertexShader = vertexShader.shader;
    pipeline.vertexBlob = vertexShader.blob;
    pipeline.inputLayout = vertexShader.inputLayout;
    pipeline.topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    return pipeline;
}

Mesh create_plane_mesh()
{
    Mesh mesh{};

    mesh.vertexLayout = mvCreateVertexLayout(
        {
            mvVertexElement::Position3D,
            mvVertexElement::Normal
        }
    );

    const float side = 10.0f;
    auto vertices = std::vector<f32>{
        -side, 0.0f, side, 0.0f, 1.0f, 0.0f,
         side, 0.0f, side, 0.0f, 1.0f, 0.0f,
         side, 0.0f, -side, 0.0f, 1.0f, 0.0f,
         -side, 0.0f, -side, 0.0f, 1.0f, 0.0f,
    };

    static auto indices = std::vector<u32>{
        0, 1, 2,
        0, 2, 3
    };

    mesh.vertexBuffer = mvCreateBuffer(vertices.data(), vertices.size() * sizeof(f32), D3D11_BIND_VERTEX_BUFFER);
    mesh.indexBuffer = mvCreateBuffer(indices.data(), indices.size() * sizeof(u32), D3D11_BIND_INDEX_BUFFER);
    return mesh;
}

Mesh create_cube_mesh()
{
    Mesh mesh{};

    mesh.vertexLayout = mvCreateVertexLayout(
        {
            mvVertexElement::Position3D,
            mvVertexElement::Normal
        }
    );

    const float side = 1.0f;
    auto vertices = std::vector<f32>{
        -side, -side, -side, 0.0f, 0.0f, 0.0f,  // 0 near side
         side, -side, -side, 0.0f, 0.0f, 0.0f,  // 1
        -side,  side, -side, 0.0f, 0.0f, 0.0f,  // 2
         side,  side, -side, 0.0f, 0.0f, 0.0f,  // 3
        -side, -side,  side, 0.0f, 0.0f, 0.0f,  // 4 far side
         side, -side,  side, 0.0f, 0.0f, 0.0f,  // 5
        -side,  side,  side, 0.0f, 0.0f, 0.0f,  // 6
         side,  side,  side, 0.0f, 0.0f, 0.0f,  // 7
        -side, -side, -side, 0.0f, 0.0f, 0.0f,  // 8 left side
        -side,  side, -side, 0.0f, 0.0f, 0.0f,  // 9
        -side, -side,  side, 0.0f, 0.0f, 0.0f,  // 10
        -side,  side,  side, 0.0f, 0.0f, 0.0f,  // 11
         side, -side, -side, 0.0f, 0.0f, 0.0f,  // 12 right side
         side,  side, -side, 0.0f, 0.0f, 0.0f,  // 13
         side, -side,  side, 0.0f, 0.0f, 0.0f,  // 14
         side,  side,  side, 0.0f, 0.0f, 0.0f,  // 15
        -side, -side, -side, 0.0f, 0.0f, 0.0f,  // 16 bottom side
         side, -side, -side, 0.0f, 0.0f, 0.0f,  // 17
        -side, -side,  side, 0.0f, 0.0f, 0.0f,  // 18
         side, -side,  side, 0.0f, 0.0f, 0.0f,  // 19
        -side,  side, -side, 0.0f, 0.0f, 0.0f,  // 20 top side
         side,  side, -side, 0.0f, 0.0f, 0.0f,  // 21
        -side,  side,  side, 0.0f, 0.0f, 0.0f,  // 22
         side,  side,  side, 0.0f, 0.0f, 0.0f   // 23
    };

    static auto indices = std::vector<u32>{
        0, 2, 1, 2, 3, 1,
        4, 5, 7, 4, 7, 6,
        8, 10, 9, 10, 11, 9,
        12, 13, 15, 12, 15, 14,
        16, 17, 18, 18, 17, 19,
        20, 23, 21, 20, 22, 23
    };

    for (size_t i = 0; i < indices.size(); i += 3)
    {

        mvVec3 p0 = { vertices[6 * indices[i]], vertices[6 * indices[i] + 1], vertices[6 * indices[i] + 2] };
        mvVec3 p1 = { vertices[6 * indices[i + 1]], vertices[6 * indices[i + 1] + 1], vertices[6 * indices[i + 1] + 2] };
        mvVec3 p2 = { vertices[6 * indices[i + 2]], vertices[6 * indices[i + 2] + 1], vertices[6 * indices[i + 2] + 2] };

        mvVec3 n = mvNormalize(mvCross(p1 - p0, p2 - p0));
        vertices[6 * indices[i] + 3] = n[0];
        vertices[6 * indices[i] + 4] = n[1];
        vertices[6 * indices[i] + 5] = n[2];
        vertices[6 * indices[i + 1] + 3] = n[0];
        vertices[6 * indices[i + 1] + 4] = n[1];
        vertices[6 * indices[i + 1] + 5] = n[2];
        vertices[6 * indices[i + 2] + 3] = n[0];
        vertices[6 * indices[i + 2] + 4] = n[1];
        vertices[6 * indices[i + 2] + 5] = n[2];
    }

    mesh.vertexBuffer = mvCreateBuffer(vertices.data(), vertices.size() * sizeof(f32), D3D11_BIND_VERTEX_BUFFER);
    mesh.indexBuffer = mvCreateBuffer(indices.data(), indices.size() * sizeof(u32), D3D11_BIND_INDEX_BUFFER);
    return mesh;
}

Mesh create_light_mesh()
{
    Mesh mesh{};

    mesh.vertexLayout = mvCreateVertexLayout(
        {
            mvVertexElement::Position3D
        }
    );

    const float side = 0.25f;
    auto vertices = std::vector<f32>{
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

    static auto indices = std::vector<u32>{
        1,  2,  0,  1,  3,  2,
        7,  5,  4,  6,  7,  4,
        9, 10,  8, 9, 11,  10,
        15, 13, 12, 14, 15, 12,
        18, 17, 16, 19, 17, 18,
        21, 23, 20, 23, 22, 20
    };

    mesh.vertexBuffer = mvCreateBuffer(vertices.data(), vertices.size() * sizeof(f32), D3D11_BIND_VERTEX_BUFFER);
    mesh.indexBuffer = mvCreateBuffer(indices.data(), indices.size() * sizeof(u32), D3D11_BIND_INDEX_BUFFER);
    return mesh;
}

int main()
{

    mvCreateContext();
    GContext->IO.shaderDirectory = "../../Examples/Example1/";
    GContext->IO.resourceDirectory = "../../Resources/";

    int initialWidth = 1850;
    int initialHeight = 900;
    mvViewport* window = mvInitializeViewport(initialWidth, initialHeight);
    mvSetupGraphics(*window);
    ID3D11DeviceContext* ctx = GContext->graphics.imDeviceContext.Get();

    // Setup Dear ImGui context
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui_ImplWin32_Init(window->hWnd);
    ImGui_ImplDX11_Init(GContext->graphics.device.Get(), GContext->graphics.imDeviceContext.Get());

    Mesh planeMesh = create_plane_mesh();
    Mesh cubeMesh = create_cube_mesh();
    Mesh lightMesh = create_light_mesh();

    mvCamera camera{};
    camera.pos = { 5.0f, 6.0f, 5.0f };
    camera.front = { 0.0f, 0.0f, -1.0f };
    camera.pitch = 0.0f;
    camera.yaw = 0.0f;
    camera.aspect = initialWidth / initialHeight;

    PointLightInfo pointLightInfo{};
    DirectionLightInfo directionLightInfo{};
    TransformInfo transformInfo{};

    mvConstBuffer directionLightBuffer = mvCreateConstBuffer(&directionLightInfo, sizeof(DirectionLightInfo));
    mvConstBuffer pointLightBuffer = mvCreateConstBuffer(&pointLightInfo, sizeof(PointLightInfo));
    mvConstBuffer transformCBuffer = mvCreateConstBuffer(&transformInfo, sizeof(TransformInfo));

    D3D11_VIEWPORT viewport = { 0.0f, 0.0f, (f32)initialWidth, (f32)initialHeight, 0.0f, 1.0f };

    mvPipeline lightPipeline = create_light_pipeline();
    mvPipeline phongPipeline = create_phong_pipeline();

    mvTimer timer;
    while (true)
    {
        const auto dt = timer.mark() * 1.0f;

        if (const auto ecode = mvProcessViewportEvents()) break;

        if (window->resized)
        {
            mvRecreateSwapChain(window->width, window->height);
            window->resized = false;
            camera.aspect = (float)window->width / (float)window->height;
            viewport.Width = window->width;
            viewport.Height = window->height;
        }

        //-----------------------------------------------------------------------------
        // clear targets
        //-----------------------------------------------------------------------------
        static float backgroundColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
        ctx->ClearRenderTargetView(*GContext->graphics.target.GetAddressOf(), backgroundColor);
        ctx->ClearDepthStencilView(*GContext->graphics.targetDepth.GetAddressOf(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0u);

        //-----------------------------------------------------------------------------
        // begin frame
        //-----------------------------------------------------------------------------
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        ImGui::GetForegroundDrawList()->AddText(ImVec2(45, 45),
            ImColor(0.0f, 1.0f, 0.0f), std::string(std::to_string(ImGui::GetIO().Framerate) + " FPS").c_str());

        // controls
        ImGui::Begin("Point Light");
        ImGui::SliderFloat3("Position", &pointLightInfo.viewLightPos.x, -25.0f, 50.0f);
        ImGui::End();

        //-----------------------------------------------------------------------------
        // main pass
        //-----------------------------------------------------------------------------
        ctx->OMSetRenderTargets(1, GContext->graphics.target.GetAddressOf(), *GContext->graphics.targetDepth.GetAddressOf());
        ctx->RSSetViewports(1u, &viewport);

        mvUpdateCameraFPSCamera(camera, dt, 12.0f, 0.004f);
        mvMat4 viewMatrix = mvCreateFPSView(camera);
        mvMat4 projMatrix = mvCreateLookAtProjection(camera);

        {
            mvVec4 posCopy = pointLightInfo.viewLightPos;

            mvVec4 out = viewMatrix * pointLightInfo.viewLightPos;
            pointLightInfo.viewLightPos.x = out.x;
            pointLightInfo.viewLightPos.y = out.y;
            pointLightInfo.viewLightPos.z = out.z;

            mvUpdateConstBuffer(pointLightBuffer, &pointLightInfo);
            pointLightInfo.viewLightPos = posCopy;
        }

        {
            mvVec3 posCopy = directionLightInfo.viewLightDir;

            mvVec4 out = viewMatrix * mvVec4{
                directionLightInfo.viewLightDir.x,
                directionLightInfo.viewLightDir.y,
                directionLightInfo.viewLightDir.z,
                0.0f };
            directionLightInfo.viewLightDir.x = out.x;
            directionLightInfo.viewLightDir.y = out.y;
            directionLightInfo.viewLightDir.z = out.z;

            mvUpdateConstBuffer(directionLightBuffer, &directionLightInfo);
            directionLightInfo.viewLightDir = posCopy;
        }

        // pixel constant buffers
        ctx->PSSetConstantBuffers(0u, 1u, &pointLightBuffer.buffer);
        ctx->PSSetConstantBuffers(1u, 1u, &directionLightBuffer.buffer);

        static const UINT offset = 0u;
        mvSetPipelineState(lightPipeline);

        // light mesh
        transformInfo.modelView = mvTranslate(mvIdentityMat4(), pointLightInfo.viewLightPos.xyz()) * viewMatrix;
        transformInfo.modelViewProj = projMatrix * viewMatrix * mvTranslate(mvIdentityMat4(), pointLightInfo.viewLightPos.xyz());
        mvUpdateConstBuffer(transformCBuffer, &transformInfo);

        ctx->VSSetConstantBuffers(0u, 1u, &transformCBuffer.buffer);
        ctx->IASetIndexBuffer(cubeMesh.indexBuffer.buffer, DXGI_FORMAT_R32_UINT, 0u);
        ctx->IASetVertexBuffers(0u, 1u,&lightMesh.vertexBuffer.buffer,&lightMesh.vertexLayout.size, &offset);
        ctx->DrawIndexed(lightMesh.indexBuffer.size / sizeof(u32), 0u, 0u);

        mvSetPipelineState(phongPipeline);

        // plane mesh
        transformInfo.modelView = viewMatrix;
        transformInfo.modelViewProj = projMatrix * viewMatrix;
        mvUpdateConstBuffer(transformCBuffer, &transformInfo);

        ctx->VSSetConstantBuffers(0u, 1u, &transformCBuffer.buffer);
        ctx->IASetIndexBuffer(planeMesh.indexBuffer.buffer, DXGI_FORMAT_R32_UINT, 0u);
        ctx->IASetVertexBuffers(0u, 1u, &planeMesh.vertexBuffer.buffer, &planeMesh.vertexLayout.size, &offset);
        ctx->DrawIndexed(planeMesh.indexBuffer.size / sizeof(u32), 0u, 0u);

        // cube mesh
        transformInfo.modelView = viewMatrix;
        transformInfo.modelViewProj = projMatrix * viewMatrix;
        mvUpdateConstBuffer(transformCBuffer, &transformInfo);

        ctx->VSSetConstantBuffers(0u, 1u, &transformCBuffer.buffer);
        ctx->IASetIndexBuffer(cubeMesh.indexBuffer.buffer, DXGI_FORMAT_R32_UINT, 0u);
        ctx->IASetVertexBuffers(0u, 1u, &cubeMesh.vertexBuffer.buffer, &cubeMesh.vertexLayout.size, &offset);
        ctx->DrawIndexed(cubeMesh.indexBuffer.size / sizeof(u32), 0u, 0u);

        //-----------------------------------------------------------------------------
        // end frame & present
        //-----------------------------------------------------------------------------

        // render imgui
        ImGui::Render();
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        // present
        GContext->graphics.swapChain->Present(1, 0);
    }


    // Cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
    mvCleanupGraphics();

    mvDestroyContext();
}

