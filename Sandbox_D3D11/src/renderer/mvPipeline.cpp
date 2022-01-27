#include "mvPipeline.h"
#include <assert.h>
#include <d3dcompiler.h>
#include "mvSandbox.h"

static std::wstring
ToWide(const std::string& narrow)
{
	wchar_t wide[1024];
	mbstowcs_s(nullptr, wide, narrow.c_str(), _TRUNCATE);
	return wide;
}

mvComputeShader
create_compute_shader(const std::string& path, std::vector<D3D_SHADER_MACRO>* macros)
{
	mvComputeShader shader{};
	shader.path = path;

	mvComPtr<ID3DBlob> shaderCompileErrorsBlob;
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

	hResult = GContext->graphics.device->CreateComputeShader(shader.blob->GetBufferPointer(), shader.blob->GetBufferSize(), nullptr, shader.shader.GetAddressOf());
	assert(SUCCEEDED(hResult));
	return shader;
}

mvPixelShader
create_pixel_shader(const std::string& path, std::vector<D3D_SHADER_MACRO>* macros)
{
	mvPixelShader shader{};
	shader.path = path;

	mvComPtr<ID3DBlob> shaderCompileErrorsBlob;
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

	hResult = GContext->graphics.device->CreatePixelShader(shader.blob->GetBufferPointer(), shader.blob->GetBufferSize(), nullptr, shader.shader.GetAddressOf());
	assert(SUCCEEDED(hResult));
	return shader;
}

mvVertexShader
create_vertex_shader(const std::string& path, mvVertexLayout& layout, std::vector<D3D_SHADER_MACRO>* macros)
{
	mvVertexShader shader{};
	shader.path = path;

	mvComPtr<ID3DBlob> shaderCompileErrorsBlob;
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

	hResult = GContext->graphics.device->CreateVertexShader(shader.blob->GetBufferPointer(),
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

	hResult = GContext->graphics.device->CreateInputLayout(layout.d3dLayout.data(),
		(uint32_t)layout.d3dLayout.size(),
		shader.blob->GetBufferPointer(),
		shader.blob->GetBufferSize(),
		shader.inputLayout.GetAddressOf());

	assert(SUCCEEDED(hResult));

	return shader;
}

struct mvVertexElementTemp
{
    i32            itemCount = 0;
    b8             normalize = false;
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

	case mvVertexElement::Position2D:
		newelement.format = DXGI_FORMAT_R32G32_FLOAT;
		newelement.itemCount = 2;
		newelement.normalize = false;
		newelement.size = sizeof(f32) * newelement.itemCount;
		newelement.semantic = "Position";
		break;

	case mvVertexElement::Position3D:
		newelement.format = DXGI_FORMAT_R32G32B32_FLOAT;
		newelement.itemCount = 3;
		newelement.normalize = false;
		newelement.size = sizeof(f32) * newelement.itemCount;
		newelement.semantic = "Position";
		break;

	case mvVertexElement::TexCoord0:
		newelement.format = DXGI_FORMAT_R32G32_FLOAT;
		newelement.itemCount = 2;
		newelement.index = 0;
		newelement.normalize = false;
		newelement.size = sizeof(f32) * newelement.itemCount;
		newelement.semantic = "TexCoord";
		break;

	case mvVertexElement::TexCoord1:
		newelement.format = DXGI_FORMAT_R32G32_FLOAT;
		newelement.itemCount = 2;
		newelement.index = 1;
		newelement.normalize = false;
		newelement.size = sizeof(f32) * newelement.itemCount;
		newelement.semantic = "TexCoord";
		break;

	case mvVertexElement::Color3_0:
		newelement.format = DXGI_FORMAT_R32G32B32_FLOAT;
		newelement.itemCount = 3;
		newelement.index = 0;
		newelement.normalize = false;
		newelement.size = sizeof(f32) * newelement.itemCount;
		newelement.semantic = "Color";
		break;

	case mvVertexElement::Color4_0:
		newelement.format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		newelement.itemCount = 4;
		newelement.index = 0;
		newelement.normalize = false;
		newelement.size = sizeof(f32) * newelement.itemCount;
		newelement.semantic = "Color";
		break;

	case mvVertexElement::Color3_1:
		newelement.format = DXGI_FORMAT_R32G32B32_FLOAT;
		newelement.itemCount = 3;
		newelement.index = 1;
		newelement.normalize = false;
		newelement.size = sizeof(f32) * newelement.itemCount;
		newelement.semantic = "Color";
		break;

	case mvVertexElement::Color4_1:
		newelement.format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		newelement.itemCount = 4;
		newelement.index = 1;
		newelement.normalize = false;
		newelement.size = sizeof(f32) * newelement.itemCount;
		newelement.semantic = "Color";
		break;

	case mvVertexElement::Normal:
		newelement.format = DXGI_FORMAT_R32G32B32_FLOAT;
		newelement.itemCount = 3;
		newelement.normalize = false;
		newelement.size = sizeof(f32) * newelement.itemCount;
		newelement.semantic = "Normal";
		break;

	case mvVertexElement::Tangent:
		newelement.format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		newelement.itemCount = 4;
		newelement.normalize = false;
		newelement.size = sizeof(f32) * newelement.itemCount;
		newelement.semantic = "Tangent";
		break;

	case mvVertexElement::Joints0:
		newelement.format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		newelement.itemCount = 4;
		newelement.index = 0;
		newelement.normalize = false;
		newelement.size = sizeof(f32) * newelement.itemCount;
		newelement.semantic = "Joints";
		break;

	case mvVertexElement::Joints1:
		newelement.format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		newelement.itemCount = 4;
		newelement.index = 1;
		newelement.normalize = false;
		newelement.size = sizeof(f32) * newelement.itemCount;
		newelement.semantic = "Joints";
		break;

	case mvVertexElement::Weights0:
		newelement.format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		newelement.itemCount = 4;
		newelement.index = 0;
		newelement.normalize = false;
		newelement.size = sizeof(f32) * newelement.itemCount;
		newelement.semantic = "Weights";
		break;

	case mvVertexElement::Weights1:
		newelement.format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		newelement.itemCount = 4;
		newelement.index = 1;
		newelement.normalize = false;
		newelement.size = sizeof(f32) * newelement.itemCount;
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
	if (strcmp(semantic, "POSITION") == 0)   return mvVertexElement::Position3D;
	if (strcmp(semantic, "NORMAL") == 0)     return mvVertexElement::Normal;
	if (strcmp(semantic, "TANGENT") == 0)    return mvVertexElement::Tangent;
	if (strcmp(semantic, "JOINTS_0") == 0)   return mvVertexElement::Joints0;
	if (strcmp(semantic, "JOINTS_1") == 0)   return mvVertexElement::Joints1;
	if (strcmp(semantic, "WEIGHTS_0") == 0)  return mvVertexElement::Weights0;
	if (strcmp(semantic, "WEIGHTS_1") == 0)  return mvVertexElement::Weights0;
	if (strcmp(semantic, "TEXCOORD_0") == 0) return mvVertexElement::TexCoord0;
	if (strcmp(semantic, "TEXCOORD_1") == 0) return mvVertexElement::TexCoord0;
	if (strcmp(semantic, "COLOR_03") == 0)   return mvVertexElement::Color3_0;
	if (strcmp(semantic, "COLOR_04") == 0)   return mvVertexElement::Color4_0;
	if (strcmp(semantic, "COLOR_13") == 0)   return mvVertexElement::Color3_1;
	if (strcmp(semantic, "COLOR_14") == 0)   return mvVertexElement::Color4_1;

	assert(false && "Undefined semantic");
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
finalize_pipeline(mvPipelineInfo& info)
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

    GContext->graphics.device->CreateRasterizerState(&rasterDesc, pipeline.rasterizationState.GetAddressOf());

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

    GContext->graphics.device->CreateDepthStencilState(&dsDesc, pipeline.depthStencilState.GetAddressOf());

    D3D11_BLEND_DESC blendDesc = CD3D11_BLEND_DESC{ CD3D11_DEFAULT{} };
    auto& brt = blendDesc.RenderTarget[0];
    brt.BlendEnable = TRUE;
    brt.SrcBlend = D3D11_BLEND_SRC_ALPHA;
    brt.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    GContext->graphics.device->CreateBlendState(&blendDesc, pipeline.blendState.GetAddressOf());

    std::vector<D3D_SHADER_MACRO> macros;
    for (auto& mac : info.macros)
        macros.push_back({ mac.macro.c_str(), mac.value.c_str()});

    macros.push_back({ NULL, NULL });

    if (!info.pixelShader.empty())
    {
        mvPixelShader pixelShader = create_pixel_shader(GContext->IO.shaderDirectory + info.pixelShader, macros.empty() ? nullptr : &macros);
        pipeline.pixelShader = pixelShader.shader;
        pipeline.pixelBlob = pixelShader.blob;
    }

    mvVertexShader vertexShader = create_vertex_shader(GContext->IO.shaderDirectory + info.vertexShader, info.layout, macros.empty() ? nullptr : &macros);

    pipeline.vertexShader = vertexShader.shader;
    pipeline.vertexBlob = vertexShader.blob;
    pipeline.inputLayout = vertexShader.inputLayout;

    return pipeline;
}
