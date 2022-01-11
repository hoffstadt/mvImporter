#include "mvMaterials.h"
#include "mvAssetManager.h"
#include <assert.h>
#include "mvSandbox.h"
#include "mvVertexLayout.h"

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
create_material(mvAssetManager& am, const std::string& vs, const std::string& ps, mvMaterial materialInfo)
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

		if (GContext->IO.imageBasedLighting) pipelineInfo.macros.push_back({ "USE_IBL" , "0"});
		if (GContext->IO.punctualLighting) pipelineInfo.macros.push_back({ "USE_PUNCTUAL", "0" });
		if (materialInfo.extensionClearcoat && GContext->IO.clearcoat) pipelineInfo.macros.push_back({ "MATERIAL_CLEARCOAT", "0" });
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

		for (auto& macro : materialInfo.extramacros)
			pipelineInfo.macros.push_back(macro);

		std::string hash = hash_material(material, material.layout, ps, vs);
		pipelineInfo.layout = material.layout;

		material.pipeline = mvGetMaterialAssetID(&am, hash);
		if (material.pipeline == -1)
		{
			material.pipeline = register_asset(&am, hash, finalize_pipeline(pipelineInfo));
		}

		material.buffer = create_const_buffer(&material.data, sizeof(mvMaterialData));

	}

	// shadow pipeline
	{
		mvPipelineInfo pipelineInfo{};
		pipelineInfo.pixelShader = material.alphaMode == 0 ? "Shadow_PS.hlsl" : "";
		pipelineInfo.vertexShader = "Shadow_VS.hlsl";
		pipelineInfo.depthBias = 50;
		pipelineInfo.slopeBias = 2.0f;
		pipelineInfo.clamp = 0.1f;
		pipelineInfo.cull = false;

		pipelineInfo.layout = material.layout;

		for (auto& macro : materialInfo.extramacros)
			pipelineInfo.macros.push_back(macro);

		std::string hash = hash_material(material, material.layout, pipelineInfo.pixelShader, pipelineInfo.vertexShader);

		if (material.alphaMode != 0)
			hash.append("alpha");

		material.spipeline = register_asset(&am, hash + "shadow", finalize_pipeline(pipelineInfo));
	}

	return material;
}