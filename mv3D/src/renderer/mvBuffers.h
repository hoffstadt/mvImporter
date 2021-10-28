#pragma once

#include <vector>
#include "mvWindows.h"
#include "mvGraphics.h"
#include "mvTypes.h"

struct mvBuffer
{
	u32                    size;
	mvComPtr<ID3D11Buffer> buffer;
};

struct mvConstBuffer
{
	mvComPtr<ID3D11Buffer> buffer;
	u32                    size;
};

mvBuffer      mvCreateBuffer     (void* data, u32 size, D3D11_BIND_FLAG flags);
mvConstBuffer mvCreateConstBuffer(void* data, u32 size);
void          mvUpdateConstBuffer(mvConstBuffer& buffer, void* data);
