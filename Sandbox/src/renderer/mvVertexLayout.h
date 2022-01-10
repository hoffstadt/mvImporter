#pragma once

#include <vector>
#include <string>
#include "mvWindows.h"
#include "mvGraphics.h"
#include "mvTypes.h"

enum class mvVertexElement;
struct mvVertexLayout;

mvVertexLayout create_vertex_layout(std::vector<mvVertexElement> elements);
mvVertexElement get_element_from_gltf_semantic(const char* semantic);

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

bool operator==(mvVertexLayout& left, mvVertexLayout& right);
bool operator!=(mvVertexLayout& left, mvVertexLayout& right);