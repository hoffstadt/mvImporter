#pragma once

#include <vector>
#include <string>
#include "mvWindows.h"
#include "mvGraphics.h"
#include "mvTypes.h"

enum class mvVertexElement
{
	Position2D,
	Position3D,
	Texture2D,
	Color,
	Normal,
	Tangent,
	Bitangent
};

struct mvVertexLayout
{
	u32                                   size;
	std::vector<std::string>              semantics;
	std::vector<DXGI_FORMAT>              formats;
	std::vector<D3D11_INPUT_ELEMENT_DESC> d3dLayout;
};

mvVertexLayout mvCreateVertexLayout(std::vector<mvVertexElement> elements);

bool operator==(mvVertexLayout& left, mvVertexLayout& right);
bool operator!=(mvVertexLayout& left, mvVertexLayout& right);