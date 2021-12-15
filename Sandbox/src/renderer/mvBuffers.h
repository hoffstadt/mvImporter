#pragma once

#include <vector>
#include "mvWindows.h"
#include "mvGraphics.h"
#include "mvTypes.h"

struct mvBuffer
{
	u32           size;
	ID3D11Buffer* buffer;
};

struct mvConstBuffer
{
	u32           size;
	ID3D11Buffer* buffer;
};

mvBuffer      create_buffer      (void* data, u32 size, D3D11_BIND_FLAG flags);
mvConstBuffer create_const_buffer(void* data, u32 size);
void          update_const_buffer(mvConstBuffer& buffer, void* data);
