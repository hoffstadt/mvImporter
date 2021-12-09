#include "mvMaterials.h"

mvMaterial
mvCreateMaterial(const std::string& vs, const std::string& ps, mvMaterialData& materialData)
{
	mvMaterial material{};

	mvPipelineInfo pipelineInfo{};
	pipelineInfo.pixelShader = ps;
	pipelineInfo.vertexShader = vs;
	pipelineInfo.depthBias = 0;
	pipelineInfo.slopeBias = 0.0f;
	pipelineInfo.clamp = 0.0f;
	pipelineInfo.cull = !materialData.hasAlpha;

	pipelineInfo.layout = mvCreateVertexLayout(
		{
			mvVertexElement::Position3D,
			mvVertexElement::Normal,
			mvVertexElement::Texture2D,
			mvVertexElement::Tangent
		}
	);

	mvPipelineInfo shadowPipelineInfo{};
	shadowPipelineInfo.pixelShader = materialData.hasAlpha ? "Shadow_PS.hlsl" : "";
	shadowPipelineInfo.vertexShader = "Shadow_VS.hlsl";
	shadowPipelineInfo.depthBias = 50;
	shadowPipelineInfo.slopeBias = 2.0f;
	shadowPipelineInfo.clamp = 0.1f;
	shadowPipelineInfo.cull = false;

	shadowPipelineInfo.layout = mvCreateVertexLayout(
		{
			mvVertexElement::Position3D,
			mvVertexElement::Normal,
			mvVertexElement::Texture2D,
			mvVertexElement::Tangent
		}
	);

	material.pipeline = mvFinalizePipeline(pipelineInfo);
	material.shadowPipeline = mvFinalizePipeline(shadowPipelineInfo);
	material.colorSampler = mvCreateSampler();
	material.data = materialData;
	material.buffer = mvCreateConstBuffer(&material.data, sizeof(mvMaterialData));

	return material;
}