#include "mvMaterials.h"

mvMaterial
mvCreateMaterial(const std::string& vs, const std::string& ps, mvMaterialData& materialData)
{
	mvMaterial material{};

	mvPipelineInfo pipelineInfo{};
	pipelineInfo.pixelShader = ps;
	pipelineInfo.vertexShader = vs;
	pipelineInfo.depthBias = 50;
	pipelineInfo.slopeBias = 2.0f;
	pipelineInfo.clamp = 0.1f;
	pipelineInfo.cull = !materialData.hasAlpha;

	pipelineInfo.layout = mvCreateVertexLayout(
		{
			mvVertexElement::Position3D,
			mvVertexElement::Normal,
			mvVertexElement::Texture2D,
			mvVertexElement::Tangent,
			mvVertexElement::Bitangent
		}
	);

	material.pipeline = mvFinalizePipeline(pipelineInfo);
	material.colorSampler = mvCreateSampler();
	material.data = materialData;
	material.buffer = mvCreateConstBuffer(&material.data, sizeof(mvMaterialData));

	return material;
}