#include "mvShader.h"
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
        &shader.blob, shaderCompileErrorsBlob.GetAddressOf());

    if (FAILED(hResult))
    {
        const char* errorString = NULL;
        if (hResult == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
            errorString = "Could not compile shader; file not found";
        else if (shaderCompileErrorsBlob)
            errorString = (const char*)shaderCompileErrorsBlob->GetBufferPointer();

        MessageBoxA(0, errorString, "Shader Compiler Error", MB_ICONERROR | MB_OK);
    }

    hResult = GContext->graphics.device->CreateComputeShader(shader.blob->GetBufferPointer(), shader.blob->GetBufferSize(), nullptr, &shader.shader);
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
        &shader.blob, shaderCompileErrorsBlob.GetAddressOf());

    if (FAILED(hResult))
    {
        const char* errorString = NULL;
        if (hResult == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
            errorString = "Could not compile shader; file not found";
        else if (shaderCompileErrorsBlob)
            errorString = (const char*)shaderCompileErrorsBlob->GetBufferPointer();

        MessageBoxA(0, errorString, "Shader Compiler Error", MB_ICONERROR | MB_OK);
    }

    hResult = GContext->graphics.device->CreatePixelShader(shader.blob->GetBufferPointer(), shader.blob->GetBufferSize(), nullptr, &shader.shader);
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
        0, 0, &shader.blob, shaderCompileErrorsBlob.GetAddressOf());

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
        shader.blob->GetBufferSize(), nullptr, &shader.shader);

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
        &shader.inputLayout);

    assert(SUCCEEDED(hResult));

    return shader;
}