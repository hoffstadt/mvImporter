#include "mvMaterials.h"
#include "mvObjLoader.h"

mvPhongMaterial
mvCreatePhongMaterial(const std::string& vs, const std::string& ps, b8 cull, b8 useDiffusemap, b8 useNormalmap, b8 useSpecularMap)
{
	mvPhongMaterial material{};

	mvPipelineInfo pipelineInfo{};
	pipelineInfo.pixelShader = ps;
	pipelineInfo.vertexShader = vs;
	pipelineInfo.depthBias = 50;
	pipelineInfo.slopeBias = 2.0f;
	pipelineInfo.clamp = 0.1f;
	pipelineInfo.cull = cull;

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
	material.data.useTextureMap = useDiffusemap;
	material.data.useSpecularMap = useSpecularMap;
	material.data.useNormalMap = useNormalmap;
	material.data.hasAlpha = !cull;
	material.buffer = mvCreateConstBuffer(&material.data, sizeof(mvPhongMaterialData));

	return material;
}

mvPBRMaterial
mvCreatePBRMaterial(const std::string& vs, const std::string& ps, b8 cull, b8 useAlbedomap, b8 useNormalmap, b8 useRoughnessMap, b8 useMetalMap)
{
	mvPBRMaterial material{};

	mvPipelineInfo pipelineInfo{};
	pipelineInfo.pixelShader = ps;
	pipelineInfo.vertexShader = vs;
	pipelineInfo.depthBias = 50;
	pipelineInfo.slopeBias = 2.0f;
	pipelineInfo.clamp = 0.1f;
	pipelineInfo.cull = cull;

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
	material.data.useAlbedoMap = useAlbedomap;
	material.data.useNormalMap = useNormalmap;
	material.data.useRoughnessMap = useRoughnessMap;
	material.data.useMetalMap = useMetalMap;
	material.data.hasAlpha = !cull;
	material.buffer = mvCreateConstBuffer(&material.data, sizeof(mvPBRMaterialData));

	return material;
}