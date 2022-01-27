#pragma once

#include <string>
#include <vector>
#include <d3d11.h>
#include "mvTypes.h"

// forward declarations
enum class mvVertexElement;
struct mvVertexLayout;
struct mvPipelineInfo;
struct mvPipeline;
struct mvPixelShader;
struct mvVertexShader;
struct mvComputeShader;
struct mvPipelineInfo;
struct mvShaderOptions;
struct mvShaderMacro;

mvPipeline      finalize_pipeline             (mvPipelineInfo& info);
mvVertexLayout  create_vertex_layout          (std::vector<mvVertexElement> elements);
mvVertexElement get_element_from_gltf_semantic(const char* semantic);
mvPixelShader   create_pixel_shader           (const std::string& path, std::vector<D3D_SHADER_MACRO>* macros = nullptr);
mvVertexShader  create_vertex_shader          (const std::string& path, mvVertexLayout& layout, std::vector<D3D_SHADER_MACRO>* macros = nullptr);
mvComputeShader create_compute_shader         (const std::string& path, std::vector<D3D_SHADER_MACRO>* macros = nullptr);

enum class mvVertexElement
{
	Position2D,
	Position3D,
	TexCoord0,
	TexCoord1,
	Color3_0,
	Color3_1,
	Color4_0,
	Color4_1,
	Normal,
	Tangent,
	Joints0,
	Joints1,
	Weights0,
	Weights1,
};

struct mvVertexLayout
{
	u32                                   elementCount;
	u32                                   size;
	std::vector<u32>                      indices;
	std::vector<std::string>              semantics;
	std::vector<DXGI_FORMAT>              formats;
	std::vector<D3D11_INPUT_ELEMENT_DESC> d3dLayout;
};

struct mvShaderMacro
{
    std::string macro;
    std::string value;
};

struct mvShaderOptions
{
    std::vector<D3D_SHADER_MACRO> macros;
    b8 hasAlbedoMap = false;
    b8 hasNormalMap = false;
    b8 hasMetallicRoughnessMap = false;
    b8 hasEmmissiveMap = false;
    b8 hasOcculusionMap = false;
    b8 hasClearcoatMap = false;
    b8 hasClearcoatNormalMap = false;
    b8 hasClearcoatRoughnessMap = false;
    b8 hasSpecularMap = false;
    b8 hasSpecularColorMap = false;
    b8 hasSpecularGlossinessMap = false;
    b8 hasSheenColorMap = false;
    b8 hasSheenRoughnessMap = false;
    b8 hasTransmissionMap = false;
    b8 hasThicknessMap = false;

    b8 materialClearCoat = false;
    b8 materialMetallicRoughness = false;
    b8 materialSpecularGlossiness = false;
    b8 materialSheen = false;
    b8 materialSpecular = false;
    b8 materialTransmission = false;
    b8 materialVolume = false;
    b8 materialIor = false;

    b8 materialUnlit = false;
    b8 linearOutput = false;
    b8 useIBL = false;
    b8 usePunctual = false;

    i32 alphaMode;

};

struct mvPixelShader
{
    mvComPtr<ID3D11PixelShader> shader;
    mvComPtr<ID3DBlob>          blob;
    std::string                 path;
};

struct mvVertexShader
{
    mvComPtr<ID3D11VertexShader> shader;
    mvComPtr<ID3D11InputLayout>  inputLayout;
    mvComPtr<ID3DBlob>           blob;
    std::string         path;
};

struct mvComputeShader
{
    mvComPtr<ID3D11ComputeShader> shader;
    mvComPtr<ID3DBlob>            blob;
    std::string                   path;
};

struct mvPipelineInfo
{
    std::string                pixelShader;
    std::string                vertexShader;
    mvVertexLayout             layout;
    i32                        depthBias;
    f32                        slopeBias;
    f32                        clamp;
    b8                         cull = true;
    std::vector<mvShaderMacro> macros;
};

struct mvPipeline
{
    mvComPtr<ID3D11PixelShader>       pixelShader = nullptr;
    mvComPtr<ID3D11VertexShader>      vertexShader = nullptr;
    mvComPtr<ID3DBlob>                pixelBlob = nullptr;
    mvComPtr<ID3DBlob>                vertexBlob = nullptr;
    mvComPtr<ID3D11InputLayout>       inputLayout = nullptr;
    mvComPtr<ID3D11BlendState>        blendState = nullptr;
    mvComPtr<ID3D11DepthStencilState> depthStencilState = nullptr;
    mvComPtr<ID3D11RasterizerState>   rasterizationState = nullptr;
    D3D11_PRIMITIVE_TOPOLOGY          topology;
    mvPipelineInfo                    info;
};

bool operator==(mvVertexLayout& left, mvVertexLayout& right);
bool operator!=(mvVertexLayout& left, mvVertexLayout& right);