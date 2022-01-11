#pragma once

#include <string>
#include <vector>
#include "mvVertexLayout.h"


struct mvPixelShader;
struct mvVertexShader;
struct mvComputeShader;
struct mvPipelineInfo;
struct mvShaderOptions;
struct mvShaderMacro;

mvPixelShader   create_pixel_shader  (const std::string& path, std::vector<D3D_SHADER_MACRO>* macros = nullptr);
mvVertexShader  create_vertex_shader (const std::string& path, mvVertexLayout& layout, std::vector<D3D_SHADER_MACRO>* macros = nullptr);
mvComputeShader create_compute_shader(const std::string& path, std::vector<D3D_SHADER_MACRO>* macros = nullptr);

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
    ID3D11PixelShader* shader;
    ID3DBlob*          blob;
    std::string        path;
};

struct mvVertexShader
{
    ID3D11VertexShader* shader;
    ID3D11InputLayout*  inputLayout;
    ID3DBlob*           blob;
    std::string         path;
};

struct mvComputeShader
{
    ID3D11ComputeShader* shader;
    ID3DBlob*            blob;
    std::string          path;
};