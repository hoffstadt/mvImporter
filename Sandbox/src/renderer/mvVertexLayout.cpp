#include "mvVertexLayout.h"
#include <string>
#include <assert.h>
#include "mvSandbox.h"

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
		newelement.normalize = false;
		newelement.size = sizeof(f32) * newelement.itemCount;
		newelement.semantic = "Tex0Coord";
		break;

	case mvVertexElement::TexCoord1:
		newelement.format = DXGI_FORMAT_R32G32_FLOAT;
		newelement.itemCount = 2;
		newelement.normalize = false;
		newelement.size = sizeof(f32) * newelement.itemCount;
		newelement.semantic = "Tex1Coord";
		break;

	case mvVertexElement::Color3:
		newelement.format = DXGI_FORMAT_R32G32B32_FLOAT;
		newelement.itemCount = 3;
		newelement.normalize = false;
		newelement.size = sizeof(f32) * newelement.itemCount;
		newelement.semantic = "Color";
		break;

	case mvVertexElement::Color4:
		newelement.format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		newelement.itemCount = 4;
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