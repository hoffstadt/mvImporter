#pragma once

#include <vector>
#include "mvWindows.h"
#include "mvGraphics.h"
#include "mvTypes.h"

struct mvBuffer
{
	u32                                 size;
	mvComPtr<ID3D11Buffer>              buffer;
	mvComPtr<ID3D11ShaderResourceView>  shaderResourceView = nullptr;
	mvComPtr<ID3D11UnorderedAccessView> unorderedAccessView = nullptr;
};

struct mvConstBuffer
{
	u32                    size;
	mvComPtr<ID3D11Buffer> buffer;
};

mvBuffer      create_buffer      (void* data, u32 size, D3D11_BIND_FLAG flags, u32 stride = 0u, u32 miscFlags = 0u);
mvConstBuffer create_const_buffer(void* data, u32 size);
void          update_const_buffer(mvConstBuffer& buffer, void* data);
