#include "mvMaterials.h"
#include <assert.h>

std::string hash_material(const mvMaterial& material, const mvVertexLayout& layout, const std::string& pixelShader, const std::string& vertexShader)
{
	std::string hash = pixelShader + vertexShader +
		std::to_string(material.data.albedo.x) +
		std::to_string(material.data.albedo.y) +
		std::to_string(material.data.albedo.z) +
		std::to_string(material.data.albedo.w) +
		std::to_string(material.data.metalness) +
		std::to_string(material.data.roughness) +
		std::to_string(material.data.alphaCutoff) +
		std::to_string(material.data.emisiveFactor.x) +
		std::to_string(material.data.emisiveFactor.y) +
		std::to_string(material.data.emisiveFactor.z) +
		std::to_string(material.data.radiance) +
		std::to_string(material.data.fresnel) +
		std::to_string(material.alphaMode) +
		std::to_string(material.data.clearcoatFactor) +
		std::to_string(material.data.clearcoatRoughnessFactor) +
		std::to_string(material.data.clearcoatNormalScale) +
		std::to_string(material.data.normalScale) +
		std::string(material.hasClearcoatMap ? "T" : "F") +
		std::string(material.hasClearcoatRoughnessMap ? "T" : "F") +
		std::string(material.hasClearcoatNormalMap ? "T" : "F") +
		std::string(material.data.doubleSided ? "T" : "F") +
		std::string(material.hasAlbedoMap ? "T" : "F") +
		std::string(material.hasNormalMap ? "T" : "F") +
		std::string(material.hasMetallicRoughnessMap ? "T" : "F") +
		std::string(material.hasOcculusionMap ? "T" : "F") +
		std::string(material.hasEmmissiveMap ? "T" : "F");

	for (auto& semantic : layout.semantics)
		hash.append(semantic);

	return hash;
}

mvMaterial
create_material(mvGraphics& graphics, const std::string& vs, const std::string& ps, mvMaterial materialInfo)
{
	mvMaterial material = materialInfo;

	// regular pipeline
	{
		mvPipelineInfo pipelineInfo{};
		pipelineInfo.pixelShader = ps;
		pipelineInfo.vertexShader = vs;
		pipelineInfo.depthBias = 0;
		pipelineInfo.slopeBias = 0.0f;
		pipelineInfo.clamp = 0.0f;
		pipelineInfo.cull = !materialInfo.data.doubleSided;
		pipelineInfo.macros = materialInfo.macros;

		if (graphics.imageBasedLighting) pipelineInfo.macros.push_back({ "USE_IBL" , "0"});
		if (materialInfo.extensionClearcoat && graphics.clearcoat) pipelineInfo.macros.push_back({ "MATERIAL_CLEARCOAT", "0" });
		if (materialInfo.pbrMetallicRoughness) pipelineInfo.macros.push_back({ "MATERIAL_METALLICROUGHNESS", "0" });
		if (materialInfo.alphaMode == 0) pipelineInfo.macros.push_back({ "ALPHAMODE", "0" });
		else if (materialInfo.alphaMode == 1) pipelineInfo.macros.push_back({ "ALPHAMODE", "1" });
		else if (materialInfo.alphaMode == 2) pipelineInfo.macros.push_back({ "ALPHAMODE", "2" });
		if (materialInfo.hasAlbedoMap)pipelineInfo.macros.push_back({ "HAS_BASE_COLOR_MAP", "0" });
		if (materialInfo.hasNormalMap)pipelineInfo.macros.push_back({ "HAS_NORMAL_MAP", "0" });
		if (materialInfo.hasMetallicRoughnessMap)pipelineInfo.macros.push_back({ "HAS_METALLIC_ROUGHNESS_MAP", "0" });
		if (materialInfo.hasEmmissiveMap)pipelineInfo.macros.push_back({ "HAS_EMISSIVE_MAP", "0" });
		if (materialInfo.hasOcculusionMap)pipelineInfo.macros.push_back({ "HAS_OCCLUSION_MAP", "0" });
		if (materialInfo.hasClearcoatMap)pipelineInfo.macros.push_back({ "HAS_CLEARCOAT_MAP", "0" });
		if (materialInfo.hasClearcoatRoughnessMap)pipelineInfo.macros.push_back({ "HAS_CLEARCOAT_ROUGHNESS_MAP", "0" });
		if (materialInfo.hasClearcoatNormalMap)pipelineInfo.macros.push_back({ "HAS_CLEARCOAT_NORMAL_MAP", "0" });
		if (graphics.punctualLighting) pipelineInfo.macros.push_back({ "USE_PUNCTUAL", "0" });

		for (auto& macro : materialInfo.extramacros)
			pipelineInfo.macros.push_back(macro);

		pipelineInfo.layout = material.layout;
		material.pipeline = finalize_pipeline(graphics, pipelineInfo);
		material.buffer = create_const_buffer(graphics, &material.data, sizeof(mvMaterialData));

	}

	return material;
}

mvAssetID
register_asset(mvMaterialManager* manager, const std::string& tag, mvMaterial asset)
{
	manager->materials.push_back({ tag, asset });
	return manager->materials.size()-1;
}

mvAssetID
mvGetMaterialAssetID(mvMaterialManager* manager, const std::string& tag)
{

	for (int i = 0; i < manager->materials.size(); i++)
	{
		if (manager->materials[i].hash == tag)
			return i;
	}
	return -1;
}

mvMaterial*
mvGetRawMaterialAsset(mvMaterialManager* manager, const std::string& tag)
{
	for (int i = 0; i < manager->materials.size(); i++)
	{
		if (manager->materials[i].hash == tag)
			return &manager->materials[i].asset;
	}
	assert(false && "Material not found.");
	return nullptr;
}

void
reload_materials(mvGraphics& graphics, mvMaterialManager* manager)
{

	for (int i = 0; i < manager->materials.size(); i++)
	{

		mvMaterial& material = manager->materials[i].asset;
		mvPipeline& pipeline = material.pipeline;

		// clear old pipeline
		material.pipeline.vertexShader = nullptr;
		material.pipeline.vertexBlob = nullptr;
		material.pipeline.inputLayout = nullptr;
		material.pipeline.blendState = nullptr;
		material.pipeline.depthStencilState = nullptr;
		material.pipeline.rasterizationState = nullptr;
		material.pipeline.pixelBlob = nullptr;
		material.pipeline.pixelShader = nullptr;

		pipeline.info.macros.clear();
		if (graphics.imageBasedLighting) pipeline.info.macros.push_back({ "USE_IBL", "0" });
		if (graphics.punctualLighting) pipeline.info.macros.push_back({ "USE_PUNCTUAL", "0" });
		if (material.extensionClearcoat && graphics.clearcoat) pipeline.info.macros.push_back({ "MATERIAL_CLEARCOAT", "0" });
		if (material.pbrMetallicRoughness) pipeline.info.macros.push_back({ "MATERIAL_METALLICROUGHNESS", "0" });
		if (material.alphaMode == 0) pipeline.info.macros.push_back({ "ALPHAMODE", "0" });
		else if (material.alphaMode == 1) pipeline.info.macros.push_back({ "ALPHAMODE", "1" });
		else if (material.alphaMode == 2) pipeline.info.macros.push_back({ "ALPHAMODE", "2" });
		if (material.hasAlbedoMap)pipeline.info.macros.push_back({ "HAS_BASE_COLOR_MAP", "0" });
		if (material.hasNormalMap)pipeline.info.macros.push_back({ "HAS_NORMAL_MAP", "0" });
		if (material.hasMetallicRoughnessMap)pipeline.info.macros.push_back({ "HAS_METALLIC_ROUGHNESS_MAP", "0" });
		if (material.hasEmmissiveMap)pipeline.info.macros.push_back({ "HAS_EMISSIVE_MAP", "0" });
		if (material.hasOcculusionMap)pipeline.info.macros.push_back({ "HAS_OCCLUSION_MAP", "0" });
		if (material.hasClearcoatMap)pipeline.info.macros.push_back({ "HAS_CLEARCOAT_MAP", "0" });
		if (material.hasClearcoatRoughnessMap)pipeline.info.macros.push_back({ "HAS_CLEARCOAT_ROUGHNESS_MAP", "0" });
		if (material.hasClearcoatNormalMap)pipeline.info.macros.push_back({ "HAS_CLEARCOAT_NORMAL_MAP", "0" });

		for (auto& macro : material.extramacros)
			pipeline.info.macros.push_back(macro);

		material.pipeline = finalize_pipeline(graphics, pipeline.info);
	}


}

void
clear_materials(mvMaterialManager* manager)
{
	manager->materials.clear();
}
