#include "mvAssetManager.h"
#include <assert.h>
#include "mvSandbox.h"

void 
initialize_asset_manager(mvAssetManager* manager)
{
	manager->textures = new mvTextureAsset[manager->maxTextureCount];
	manager->samplers = new mvSamplerAsset[manager->maxSamplerCount];
	manager->materials = new mvMaterialAsset[manager->maxMaterialCount];
	manager->buffers = new mvBufferAsset[manager->maxBufferCount];
	manager->cbuffers = new mvCBufferAsset[manager->maxCBufferCount];
	manager->meshes = new mvMeshAsset[manager->maxMeshCount];
	manager->cubeTextures = new mvCubeTextureAsset[manager->maxCubeTextureCount];
	manager->nodes = new mvNodeAsset[manager->maxNodeCount];
	manager->scenes = new mvSceneAsset[manager->maxSceneCount];
	manager->cameras = new mvCameraAsset[manager->maxCameraCount];
	manager->pipelines = new mvPipelineAsset[manager->maxPipelineCount];
	manager->targetViews = new mvTargetViewAsset[manager->maxTargetViewCount];
	manager->depthViews = new mvDepthViewAsset[manager->maxDepthViewCount];
	manager->animations = new mvAnimationAsset[manager->maxAnimationCount];
	manager->skins = new mvSkinAsset[manager->maxSkinCount];

	manager->freetextures = new b8[manager->maxTextureCount];
	for (i32 i = 0; i < manager->maxTextureCount; i++)
		manager->freetextures[i] = true;

	manager->freebuffers = new b8[manager->maxBufferCount];
	for (i32 i = 0; i < manager->maxBufferCount; i++)
		manager->freebuffers[i] = true;

	manager->freemeshes = new b8[manager->maxMeshCount];
	for (i32 i = 0; i < manager->maxMeshCount; i++)
		manager->freemeshes[i] = true;

	manager->freecameras = new b8[manager->maxCameraCount];
	for (i32 i = 0; i < manager->maxCameraCount; i++)
		manager->freecameras[i] = true;

	manager->freenodes = new b8[manager->maxNodeCount];
	for (i32 i = 0; i < manager->maxNodeCount; i++)
		manager->freenodes[i] = true;

	manager->freematerials = new b8[manager->maxMaterialCount];
	for (i32 i = 0; i < manager->maxMaterialCount; i++)
		manager->freematerials[i] = true;

	manager->freescenes = new b8[manager->maxSceneCount];
	for (i32 i = 0; i < manager->maxSceneCount; i++)
		manager->freescenes[i] = true;
}

void 
cleanup_asset_manager(mvAssetManager* manager)
{
	for (int i = 0; i < manager->maxBufferCount; i++)
	{
		if(!manager->freebuffers[i])
			manager->buffers[i].asset.buffer->Release();
	}

	for (int i = 0; i < manager->cbufferCount; i++)
		manager->cbuffers[i].asset.buffer->Release();

	for (int i = 0; i < manager->maxMaterialCount; i++)
	{
		if (!manager->freematerials[i])
		{
			manager->materials[i].asset.buffer.buffer->Release();
		}
	}

	for (int i = 0; i < manager->cubeTextureCount; i++)
	{
		manager->cubeTextures[i].asset.textureView->Release();
	}

	for (int i = 0; i < manager->samplerCount; i++)
	{
		manager->samplers[i].asset->Release();
	}

	for (int i = 0; i < manager->targetViewCount; i++)
	{
		manager->targetViews[i].asset->Release();
	}

	for (int i = 0; i < manager->depthViewCount; i++)
	{
		manager->depthViews[i].asset->Release();
	}

	for (int i = 0; i < manager->pipelineCount; i++)
	{
		if (manager->pipelines[i].asset.vertexShader)
		{
			manager->pipelines[i].asset.vertexShader->Release();
			manager->pipelines[i].asset.vertexBlob->Release();
		}
		if(manager->pipelines[i].asset.inputLayout)
			manager->pipelines[i].asset.inputLayout->Release();
		if(manager->pipelines[i].asset.blendState)
			manager->pipelines[i].asset.blendState->Release();
		if(manager->pipelines[i].asset.depthStencilState)
			manager->pipelines[i].asset.depthStencilState->Release();
		if(manager->pipelines[i].asset.rasterizationState)
			manager->pipelines[i].asset.rasterizationState->Release();
		if (manager->pipelines[i].asset.pixelShader)
		{
			manager->pipelines[i].asset.pixelBlob->Release();
			manager->pipelines[i].asset.pixelShader->Release();
		}
	}

	for (int i = 0; i < manager->textureCount; i++)
	{
		if (manager->textures[i].asset.texture)
		{
			manager->textures[i].asset.texture->Release();

			if (manager->textures[i].asset.sampler)
			{
				manager->textures[i].asset.sampler->Release();
				manager->textures[i].asset.sampler = nullptr;
			}
		}

		if (manager->textures[i].asset.textureView)
		{
			manager->textures[i].asset.textureView->Release();
		}
	}

	for (int i = 0; i < manager->animationCount; i++)
	{
		delete[] manager->animations[i].asset.channels;
	}

	for (int i = 0; i < manager->skinCount; i++)
	{
		manager->skins[i].asset.jointTexture.sampler->Release();
		manager->skins[i].asset.jointTexture.textureView->Release();
		manager->skins[i].asset.jointTexture.texture->Release();
		delete[] manager->skins[i].asset.textureData;
	}

	for (int i = 0; i < manager->meshCount; i++)
	{
		if(manager->meshes[i].asset.morphBuffer.buffer)
			manager->meshes[i].asset.morphBuffer.buffer->Release();
	}

	// assets
	delete[] manager->buffers;
	delete[] manager->textures;
	delete[] manager->cubeTextures;
	delete[] manager->samplers;
	delete[] manager->materials;
	delete[] manager->meshes;
	delete[] manager->nodes;
	delete[] manager->scenes;
	delete[] manager->cameras;
	delete[] manager->pipelines;
	delete[] manager->cbuffers;
	delete[] manager->targetViews;
	delete[] manager->depthViews;
	delete[] manager->animations;
	delete[] manager->skins;
	
	// free slots
	delete[] manager->freetextures;
	delete[] manager->freebuffers;
	delete[] manager->freemeshes;
	delete[] manager->freecameras;
	delete[] manager->freenodes;
	delete[] manager->freematerials;
	delete[] manager->freescenes;
}

void
reload_materials(mvAssetManager* manager)
{

	for (int i = 0; i < manager->maxMaterialCount; i++)
	{
		if (!manager->freematerials[i])
		{
			mvMaterial& material = manager->materials[i].asset;
			mvPipeline& pipeline = manager->pipelines[material.pipeline].asset;

			// clear old pipeline
			if (manager->pipelines[material.pipeline].asset.vertexShader)
			{
				manager->pipelines[material.pipeline].asset.vertexShader->Release();
				manager->pipelines[material.pipeline].asset.vertexBlob->Release();
			}
			if (manager->pipelines[material.pipeline].asset.inputLayout)
				manager->pipelines[material.pipeline].asset.inputLayout->Release();
			if (manager->pipelines[material.pipeline].asset.blendState)
				manager->pipelines[material.pipeline].asset.blendState->Release();
			if (manager->pipelines[material.pipeline].asset.depthStencilState)
				manager->pipelines[material.pipeline].asset.depthStencilState->Release();
			if (manager->pipelines[material.pipeline].asset.rasterizationState)
				manager->pipelines[material.pipeline].asset.rasterizationState->Release();
			if (manager->pipelines[material.pipeline].asset.pixelShader)
			{
				manager->pipelines[material.pipeline].asset.pixelBlob->Release();
				manager->pipelines[material.pipeline].asset.pixelShader->Release();
			}

			pipeline.info.macros.clear();
			if (GContext->IO.imageBasedLighting) pipeline.info.macros.push_back({ "USE_IBL", "0" });
			if (GContext->IO.punctualLighting)
			{
				pipeline.info.macros.push_back({ "USE_PUNCTUAL", "0" });
				if (GContext->IO.directionalShadows) pipeline.info.macros.push_back({ "SHADOWS_DIRECTIONAL", "0" });
				if (GContext->IO.omniShadows) pipeline.info.macros.push_back({ "SHADOWS_OMNI", "0" });
			}

			if (material.extensionClearcoat && GContext->IO.clearcoat) pipeline.info.macros.push_back({ "MATERIAL_CLEARCOAT", "0" });
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

			pipeline.info.macros.push_back({ NULL, NULL });

			manager->pipelines[material.pipeline].asset = finalize_pipeline(pipeline.info);
		}
	}


}

//-----------------------------------------------------------------------------
// skins
//-----------------------------------------------------------------------------

mvAssetID
register_asset(mvAssetManager* manager, const std::string& tag, mvSkin asset)
{
	manager->skins[manager->skinCount].asset = asset;
	manager->skins[manager->skinCount].hash = tag;
	manager->skinCount++;
	return manager->skinCount - 1;
}

mvAssetID
mvGetSkinAssetID(mvAssetManager* manager, const std::string& tag)
{
	for (s32 i = 0; i < manager->skinCount; i++)
	{
		if (manager->skins[i].hash == tag)
			return i;
	}

	return -1;
}

mvSkin*
mvGetRawSkinAsset(mvAssetManager* manager, const std::string& tag)
{
	for (s32 i = 0; i < manager->skinCount; i++)
	{
		if (manager->skins[i].hash == tag)
			return &manager->skins[i].asset;
	}

	return nullptr;
}

//-----------------------------------------------------------------------------
// animations
//-----------------------------------------------------------------------------

mvAssetID
register_asset(mvAssetManager* manager, const std::string& tag, mvAnimation asset)
{
	manager->animations[manager->animationCount].asset = asset;
	manager->animations[manager->animationCount].hash = tag;
	manager->animationCount++;
	return manager->animationCount - 1;
}

mvAssetID
mvGetAnimationAssetID(mvAssetManager* manager, const std::string& tag)
{
	for (s32 i = 0; i < manager->animationCount; i++)
	{
		if (manager->animations[i].hash == tag)
			return i;
	}

	return -1;
}

mvAnimation*
mvGetRawAnimationAsset(mvAssetManager* manager, const std::string& tag)
{
	for (s32 i = 0; i < manager->animationCount; i++)
	{
		if (manager->animations[i].hash == tag)
			return &manager->animations[i].asset;
	}

	return nullptr;
}


//-----------------------------------------------------------------------------
// render target views
//-----------------------------------------------------------------------------

mvAssetID
register_asset(mvAssetManager* manager, const std::string& tag, ID3D11RenderTargetView* asset)
{
	manager->targetViews[manager->targetViewCount].asset = asset;
	manager->targetViews[manager->targetViewCount].hash = tag;
	manager->targetViewCount++;
	return manager->targetViewCount - 1;
}

mvAssetID
mvGetTargetViewAssetID(mvAssetManager* manager, const std::string& tag)
{
	for (s32 i = 0; i < manager->targetViewCount; i++)
	{
		if (manager->targetViews[i].hash == tag)
			return i;
	}

	return -1;
}

ID3D11RenderTargetView*
mvGetRawTargetViewAsset(mvAssetManager* manager, const std::string& tag)
{
	for (s32 i = 0; i < manager->targetViewCount; i++)
	{
		if (manager->targetViews[i].hash == tag)
			return manager->targetViews[i].asset;
	}

	return nullptr;
}

//-----------------------------------------------------------------------------
// depth views
//-----------------------------------------------------------------------------

mvAssetID
register_asset(mvAssetManager* manager, const std::string& tag, ID3D11DepthStencilView* asset)
{
	manager->depthViews[manager->depthViewCount].asset = asset;
	manager->depthViews[manager->depthViewCount].hash = tag;
	manager->depthViewCount++;
	return manager->depthViewCount - 1;
}

mvAssetID
mvGetDepthViewAssetID(mvAssetManager* manager, const std::string& tag)
{
	for (s32 i = 0; i < manager->depthViewCount; i++)
	{
		if (manager->depthViews[i].hash == tag)
			return i;
	}

	return -1;
}

ID3D11DepthStencilView*
mvGetRawDepthViewAsset(mvAssetManager* manager, const std::string& tag)
{
	for (s32 i = 0; i < manager->depthViewCount; i++)
	{
		if (manager->depthViews[i].hash == tag)
			return manager->depthViews[i].asset;
	}

	return nullptr;
}

//-----------------------------------------------------------------------------
// pipelines
//-----------------------------------------------------------------------------

mvAssetID
register_asset(mvAssetManager* manager, const std::string& tag, mvPipeline asset)
{
	manager->pipelines[manager->pipelineCount].asset = asset;
	manager->pipelines[manager->pipelineCount].hash = tag;
	manager->pipelineCount++;
	return manager->pipelineCount - 1;
}

mvAssetID
mvGetPipelineAssetID(mvAssetManager* manager, const std::string& tag)
{
	for (s32 i = 0; i < manager->pipelineCount; i++)
	{
		if (manager->pipelines[i].hash == tag)
			return i;
	}

	return -1;
}

mvPipeline*
mvGetRawPipelineAsset(mvAssetManager* manager, const std::string& tag)
{
	for (s32 i = 0; i < manager->pipelineCount; i++)
	{
		if (manager->pipelines[i].hash == tag)
			return &manager->pipelines[i].asset;
	}

	return nullptr;
}

//-----------------------------------------------------------------------------
// scenes
//-----------------------------------------------------------------------------

mvAssetID
register_asset(mvAssetManager* manager, const std::string& tag, mvScene asset)
{
	s32 freeIndex = -1;
	for (s32 i = 0; i < manager->maxSceneCount; i++)
	{
		if (manager->freescenes[i])
		{
			freeIndex = i;
			manager->freescenes[i] = false;
			break;
		}
	}

	assert(freeIndex > -1 && "No free index available");

	manager->scenes[freeIndex].asset = asset;
	manager->scenes[freeIndex].hash = tag;
	manager->sceneCount++;
	return freeIndex;
}

mvAssetID 
mvGetSceneAssetID(mvAssetManager* manager, const std::string& tag)
{
	for (s32 i = 0; i < manager->maxSceneCount; i++)
	{
		if (manager->scenes[i].hash == tag)
			return i;
	}
	assert(false && "Scene not found.");
	return -1;
}

mvScene*
mvGetRawSceneAsset(mvAssetManager* manager, const std::string& tag)
{
	for (s32 i = 0; i < manager->sceneCount; i++)
	{
		if (manager->scenes[i].hash == tag)
			return &manager->scenes[i].asset;
	}
	assert(false && "Scene not found.");
	return nullptr;
}

b8
unregister_scene_asset(mvAssetManager* manager, mvAssetID asset)
{
	if(asset == -1) return false;
	assert(asset < manager->maxSceneCount && "Asset ID outside range");
	//assert(manager->freescenes[asset] && "Scene already freed.");

	if (!manager->freescenes[asset])
	{
		manager->freescenes[asset] = true;
		manager->sceneCount--;
		manager->scenes[asset].hash.clear();
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
// textures
//-----------------------------------------------------------------------------

mvAssetID
register_asset(mvAssetManager* manager, const std::string& tag, mvTexture asset)
{
	s32 freeIndex = -1;
	for (s32 i = 0; i < manager->maxTextureCount; i++)
	{
		if (manager->freetextures[i])
		{
			freeIndex = i;
			manager->freetextures[i] = false;
			break;
		}
	}

	assert(freeIndex > -1 && "No free index available");

	manager->textures[freeIndex].asset = asset;
	manager->textures[freeIndex].hash = tag;
	manager->textureCount++;
	return freeIndex;
}

mvAssetID
register_asset(mvAssetManager* manager, const std::string& tag, mvCubeTexture asset)
{
	manager->cubeTextures[manager->cubeTextureCount].asset = asset;
	manager->cubeTextures[manager->cubeTextureCount].hash = tag;
	manager->cubeTextureCount++;
	return manager->cubeTextureCount - 1;
}

mvAssetID
mvGetTextureAssetID(mvAssetManager* manager, const std::string& tag, const std::string& path)
{
	for (s32 i = 0; i < manager->maxTextureCount; i++)
	{
		if (manager->textures[i].hash == tag)
			return i;
	}

	s32 freeIndex = register_asset(manager, tag, create_texture(path));
	return freeIndex;
}

mvAssetID
mvGetTextureAssetID(mvAssetManager* manager, const std::string& tag, std::vector<unsigned char> data)
{
	for (s32 i = 0; i < manager->maxTextureCount; i++)
	{
		if (manager->textures[i].hash == tag)
			return i;
	}

	s32 freeIndex = register_asset(manager, tag, create_texture(data));
	return freeIndex;
}

mvAssetID
mvGetCubeTextureAssetID(mvAssetManager* manager, const std::string& path)
{
	for (s32 i = 0; i < manager->cubeTextureCount; i++)
	{
		if (manager->cubeTextures[i].hash == path)
			return i;
	}

	manager->cubeTextures[manager->cubeTextureCount].hash = path;
	manager->cubeTextures[manager->cubeTextureCount].asset = create_cube_texture(path);
	manager->cubeTextureCount++;
	return manager->cubeTextureCount - 1;
}

mvTexture*
mvGetRawTextureAsset(mvAssetManager* manager, const std::string& path)
{
	for (s32 i = 0; i < manager->textureCount; i++)
	{
		if (manager->textures[i].hash == path)
			return &manager->textures[i].asset;
	}

	manager->textures[manager->textureCount].hash = path;
	manager->textures[manager->textureCount].asset = create_texture(path);
	manager->textureCount++;
	return &manager->textures[manager->textureCount - 1].asset;
}

b8 
unregister_texture_asset(mvAssetManager* manager, mvAssetID asset)
{
	if (asset == -1) return false;
	assert(asset < manager->maxTextureCount && "Asset ID outside range");
	//assert(manager->freetextures[asset] && "Texture already freed.");
	
	if (!manager->freetextures[asset])
	{
		manager->freetextures[asset] = true;
		manager->textureCount--;
		manager->textures[asset].hash.clear();
		if (manager->textures[asset].asset.texture)
		{
			manager->textures[asset].asset.texture->Release();
			manager->textures[asset].asset.textureView->Release();

			if (manager->textures[asset].asset.sampler)
			{
				manager->textures[asset].asset.sampler->Release();
				manager->textures[asset].asset.sampler = nullptr;
			}
		}
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
// meshes
//-----------------------------------------------------------------------------

mvAssetID
register_asset(mvAssetManager* manager, const std::string& tag, mvMesh asset)
{
	s32 freeIndex = -1;
	for (s32 i = 0; i < manager->maxMeshCount; i++)
	{
		if (manager->freemeshes[i])
		{
			freeIndex = i;
			manager->freemeshes[i] = false;
			break;
		}
	}

	assert(freeIndex > -1 && "No free index available");

	manager->meshes[freeIndex].asset = asset;
	manager->meshes[freeIndex].hash = tag;
	manager->meshCount++;
	return freeIndex;
}

mvMesh*
mvGetRawMeshAsset(mvAssetManager* manager, const std::string& tag)
{
	for (s32 i = 0; i < manager->maxMeshCount; i++)
	{
		if (manager->meshes[i].hash == tag)
			return &manager->meshes[i].asset;
	}
	assert(false && "Mesh not found.");
	return nullptr;
}

b8
unregister_mesh_asset(mvAssetManager* manager, mvAssetID asset)
{
	if (asset == -1) return false;
	assert(asset < manager->maxMeshCount && "Asset ID outside range");
	//assert(manager->freemeshes[asset] && "Mesh already freed.");
	
	if (!manager->freemeshes[asset])
	{
		manager->freemeshes[asset] = true;
		manager->meshCount--;
		manager->meshes[asset].hash.clear();
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
// buffers
//-----------------------------------------------------------------------------

mvAssetID
register_asset(mvAssetManager* manager, const std::string& tag, mvBuffer asset)
{
	s32 freeIndex = -1;
	for (s32 i = 0; i < manager->maxBufferCount; i++)
	{
		if (manager->freebuffers[i])
		{
			freeIndex = i;
			manager->freebuffers[i] = false;
			break;
		}
	}

	assert(freeIndex > -1 && "No free index available");

	manager->buffers[freeIndex].asset = asset;
	manager->buffers[freeIndex].hash = tag;
	manager->bufferCount++;
	return freeIndex;
}

mvAssetID
mvGetBufferAssetID(mvAssetManager* manager, const std::string& tag, void* data, u32 size, D3D11_BIND_FLAG flags)
{
	for (s32 i = 0; i < manager->maxBufferCount; i++)
	{
		if (manager->buffers[i].hash == tag)
			return i;
	}

	mvBuffer asset = create_buffer(data, size, flags);
	s32 freeIndex = register_asset(manager, tag, asset);
	return freeIndex;
}

mvBuffer*
mvGetRawBufferAsset(mvAssetManager* manager, const std::string& tag)
{
	for (s32 i = 0; i < manager->maxBufferCount; i++)
	{
		if (manager->buffers[i].hash == tag)
			return &manager->buffers[i].asset;
	}

	return nullptr;
}

b8
unregister_buffer_asset(mvAssetManager* manager, mvAssetID asset)
{
	if (asset == -1) return false;
	assert(asset < manager->maxBufferCount && "Asset ID outside range");
	//assert(manager->freebuffers[asset] && "Buffer already freed.");
	
	if (!manager->freebuffers[asset])
	{
		manager->freebuffers[asset] = true;
		manager->bufferCount--;
		manager->buffers[asset].hash.clear();
		manager->buffers[asset].asset.buffer->Release();
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
// constant buffers
//-----------------------------------------------------------------------------

mvAssetID
register_asset(mvAssetManager* manager, const std::string& tag, mvConstBuffer asset)
{
	manager->cbuffers[manager->cbufferCount].asset = asset;
	manager->cbuffers[manager->cbufferCount].hash = tag;
	manager->cbufferCount++;
	return manager->cbufferCount - 1;
}

mvConstBuffer*
mvGetRawCBufferAsset(mvAssetManager* manager, const std::string& tag)
{
	for (s32 i = 0; i < manager->cbufferCount; i++)
	{
		if (manager->cbuffers[i].hash == tag)
			return &manager->cbuffers[i].asset;
	}

	return nullptr;
}

//-----------------------------------------------------------------------------
// materials
//-----------------------------------------------------------------------------

mvAssetID
register_asset(mvAssetManager* manager, const std::string& tag, mvMaterial asset)
{
	s32 freeIndex = -1;
	for (s32 i = 0; i < manager->maxMaterialCount; i++)
	{
		if (manager->freematerials[i])
		{
			freeIndex = i;
			manager->freematerials[i] = false;
			break;
		}
	}

	assert(freeIndex > -1 && "No free index available");

	manager->materials[freeIndex].asset = asset;
	manager->materials[freeIndex].hash = tag;
	manager->materialCount++;
	return freeIndex;
}

mvAssetID
mvGetMaterialAssetID(mvAssetManager* manager, const std::string& tag)
{

	for (s32 i = 0; i < manager->maxMaterialCount; i++)
	{
		if (manager->materials[i].hash == tag)
			return i;
	}
	return -1;
}

mvMaterial*
mvGetRawMaterialAsset(mvAssetManager* manager, const std::string& tag)
{
	for (s32 i = 0; i < manager->maxMaterialCount; i++)
	{
		if (manager->materials[i].hash == tag)
			return &manager->materials[i].asset;
	}
	assert(false && "Material not found.");
	return nullptr;
}

b8
unregister_material_asset(mvAssetManager* manager, mvAssetID asset)
{
	if (asset == -1) return false;
	assert(asset < manager->maxMaterialCount && "Asset ID outside range");
	//assert(manager->freematerials[asset] && "Material already freed.");

	if (!manager->freematerials[asset])
	{
		manager->freematerials[asset] = true;
		manager->materialCount--;
		manager->materials[asset].hash.clear();
		manager->materials[asset].asset.buffer.buffer->Release();
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
// nodes
//-----------------------------------------------------------------------------

mvAssetID
register_asset(mvAssetManager* manager, const std::string& tag, mvNode asset)
{
	s32 freeIndex = -1;
	for (s32 i = 0; i < manager->maxNodeCount; i++)
	{
		if (manager->freenodes[i])
		{
			freeIndex = i;
			manager->freenodes[i] = false;
			break;
		}
	}

	assert(freeIndex > -1 && "No free index available");

	manager->nodes[freeIndex].asset = asset;
	manager->nodes[freeIndex].hash = tag;
	manager->nodeCount++;
	return freeIndex;
}

mvAssetID
mvGetNodeAssetID(mvAssetManager* manager, const std::string& tag)
{
	for (s32 i = 0; i < manager->maxNodeCount; i++)
	{
		if (manager->nodes[i].hash == tag)
			return i;
	}

	assert(false && "Node not found.");
	return -1;
}

mvNode*
mvGetRawNodeAsset(mvAssetManager* manager, const std::string& tag)
{
	for (s32 i = 0; i < manager->nodeCount; i++)
	{
		if (manager->nodes[i].hash == tag)
			return &manager->nodes[i].asset;
	}

	assert(false && "Node not found.");
	return nullptr;
}

b8
unregister_node_asset(mvAssetManager* manager, mvAssetID asset)
{
	if (asset == -1) return false;
	assert(asset < manager->maxNodeCount && "Asset ID outside range");
	//assert(manager->freenodes[asset] && "Node already freed.");

	if (!manager->freenodes[asset])
	{
		manager->freenodes[asset] = true;
		manager->nodeCount--;
		manager->nodes[asset].hash.clear();
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
// camera
//-----------------------------------------------------------------------------

mvAssetID
register_asset(mvAssetManager* manager, const std::string& tag, mvCamera asset)
{
	s32 freeIndex = -1;
	for (s32 i = 0; i < manager->maxCameraCount; i++)
	{
		if (manager->freecameras[i])
		{
			freeIndex = i;
			manager->freecameras[i] = false;
			break;
		}
	}

	assert(freeIndex > -1 && "No free index available");

	manager->cameras[freeIndex].asset = asset;
	manager->cameras[freeIndex].hash = tag;
	manager->cameraCount++;
	return freeIndex;
}

mvAssetID
mvGetCameraAssetID(mvAssetManager* manager, const std::string& tag)
{
	for (s32 i = 0; i < manager->maxCameraCount; i++)
	{
		if (manager->cameras[i].hash == tag)
			return i;
	}
	assert(false && "Camera not found");
	return -1;
}

mvCamera*
mvGetRawCameraAsset(mvAssetManager* manager, const std::string& tag)
{
	for (s32 i = 0; i < manager->maxCameraCount; i++)
	{
		if (manager->cameras[i].hash == tag)
			return &manager->cameras[i].asset;
	}
	assert(false && "Camera not found");
	return nullptr;
}

b8
unregister_camera_asset(mvAssetManager* manager, mvAssetID asset)
{
	if (asset == -1) return false;
	assert(asset < manager->maxCameraCount && "Asset ID outside range");
	assert(manager->freecameras[asset] && "Camera already freed.");
	
	if (!manager->freecameras[asset])
	{
		manager->freecameras[asset] = true;
		manager->cameraCount--;
		manager->cameras[asset].hash.clear();
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
// samplers
//-----------------------------------------------------------------------------

mvAssetID
register_asset(mvAssetManager* manager, const std::string& tag, ID3D11SamplerState* asset)
{
	manager->samplers[manager->samplerCount].asset = asset;
	manager->samplers[manager->samplerCount].hash = tag;
	manager->samplerCount++;
	return manager->samplerCount - 1;
}

mvAssetID
mvGetSamplerAssetID(mvAssetManager* manager, const std::string& tag)
{
	for (s32 i = 0; i < manager->samplerCount; i++)
	{
		if (manager->samplers[i].hash == tag)
			return i;
	}

	return -1;
}

ID3D11SamplerState*
mvGetRawSamplerAsset(mvAssetManager* manager, const std::string& tag)
{
	for (s32 i = 0; i < manager->samplerCount; i++)
	{
		if (manager->samplers[i].hash == tag)
			return manager->samplers[i].asset;
	}

	return nullptr;
}
