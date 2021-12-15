#include "mvAssetManager.h"

void 
mvInitializeAssetManager(mvAssetManager* manager)
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
}

void 
mvCleanupAssetManager(mvAssetManager* manager)
{
	for (int i = 0; i < manager->bufferCount; i++)
		manager->buffers[i].asset.buffer->Release();

	for (int i = 0; i < manager->cbufferCount; i++)
		manager->cbuffers[i].asset.buffer->Release();

	for (int i = 0; i < manager->materialCount; i++)
	{
		manager->materials[i].asset.buffer.buffer->Release();
	}

	for (int i = 0; i < manager->cubeTextureCount; i++)
	{
		manager->cubeTextures[i].asset.textureView->Release();
	}

	for (int i = 0; i < manager->samplerCount; i++)
	{
		manager->samplers[i].asset.state->Release();
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
	manager->scenes[manager->sceneCount].asset = asset;
	manager->scenes[manager->sceneCount].hash = tag;
	manager->sceneCount++;
	return manager->sceneCount - 1;
}

mvAssetID 
mvGetSceneAssetID(mvAssetManager* manager, const std::string& tag)
{
	for (s32 i = 0; i < manager->sceneCount; i++)
	{
		if (manager->scenes[i].hash == tag)
			return i;
	}

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

	return nullptr;
}

//-----------------------------------------------------------------------------
// textures
//-----------------------------------------------------------------------------

mvAssetID
register_asset(mvAssetManager* manager, const std::string& tag, mvTexture asset)
{
	manager->textures[manager->textureCount].asset = asset;
	manager->textures[manager->textureCount].hash = tag;
	manager->textureCount++;
	return manager->textureCount - 1;
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
mvGetTextureAssetID(mvAssetManager* manager, const std::string& path)
{
	for (s32 i = 0; i < manager->textureCount; i++)
	{
		if (manager->textures[i].hash == path)
			return i;
	}

	manager->textures[manager->textureCount].hash = path;
	manager->textures[manager->textureCount].asset = create_texture(path);
	manager->textureCount++;
	return manager->textureCount - 1;
}

mvAssetID
mvGetTextureAssetID(mvAssetManager* manager, const std::string& path, std::vector<unsigned char> data)
{
	for (s32 i = 0; i < manager->textureCount; i++)
	{
		if (manager->textures[i].hash == path)
			return i;
	}

	manager->textures[manager->textureCount].hash = path;
	manager->textures[manager->textureCount].asset = create_texture(data);
	manager->textureCount++;
	return manager->textureCount - 1;
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

//-----------------------------------------------------------------------------
// meshes
//-----------------------------------------------------------------------------

mvAssetID
register_asset(mvAssetManager* manager, const std::string& tag, mvMesh asset)
{
	manager->meshes[manager->meshCount].asset = asset;
	manager->meshes[manager->meshCount].hash = tag;
	manager->meshCount++;
	return manager->meshCount - 1;
}

mvMesh*
mvGetRawMeshAsset(mvAssetManager* manager, const std::string& tag)
{
	for (s32 i = 0; i < manager->meshCount; i++)
	{
		if (manager->meshes[i].hash == tag)
			return &manager->meshes[i].asset;
	}

	return nullptr;
}

//-----------------------------------------------------------------------------
// buffers
//-----------------------------------------------------------------------------

mvAssetID
register_asset(mvAssetManager* manager, const std::string& tag, mvBuffer asset)
{
	manager->buffers[manager->bufferCount].asset = asset;
	manager->buffers[manager->bufferCount].hash = tag;
	manager->bufferCount++;
	return manager->bufferCount - 1;
}

mvAssetID
mvGetBufferAssetID(mvAssetManager* manager, const std::string& tag, void* data, u32 size, D3D11_BIND_FLAG flags)
{
	for (s32 i = 0; i < manager->bufferCount; i++)
	{
		if (manager->buffers[i].hash == tag)
			return i;
	}

	manager->buffers[manager->bufferCount].hash = tag;
	manager->buffers[manager->bufferCount].asset = create_buffer(data, size, flags);
	manager->bufferCount++;
	return manager->bufferCount - 1;
}

mvBuffer*
mvGetRawBufferAsset(mvAssetManager* manager, const std::string& tag)
{
	for (s32 i = 0; i < manager->bufferCount; i++)
	{
		if (manager->buffers[i].hash == tag)
			return &manager->buffers[i].asset;
	}

	return nullptr;
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
	manager->materials[manager->materialCount].asset = asset;
	manager->materials[manager->materialCount].hash = tag;
	manager->materialCount++;
	return manager->materialCount - 1;
}

mvAssetID
mvGetMaterialAssetID(mvAssetManager* manager, const std::string& tag)
{

	for (s32 i = 0; i < manager->materialCount; i++)
	{
		if (manager->materials[i].hash == tag)
			return i;
	}

	return -1;
}

mvMaterial*
mvGetRawMaterialAsset(mvAssetManager* manager, const std::string& tag)
{
	for (s32 i = 0; i < manager->materialCount; i++)
	{
		if (manager->materials[i].hash == tag)
			return &manager->materials[i].asset;
	}

	return nullptr;
}

//-----------------------------------------------------------------------------
// nodes
//-----------------------------------------------------------------------------

mvAssetID
register_asset(mvAssetManager* manager, const std::string& tag, mvNode asset)
{
	manager->nodes[manager->nodeCount].asset = asset;
	manager->nodes[manager->nodeCount].hash = tag;
	manager->nodeCount++;
	return manager->nodeCount - 1;
}

mvAssetID
mvGetNodeAssetID(mvAssetManager* manager, const std::string& tag)
{
	for (s32 i = 0; i < manager->nodeCount; i++)
	{
		if (manager->nodes[i].hash == tag)
			return i;
	}

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

	return nullptr;
}

//-----------------------------------------------------------------------------
// camera
//-----------------------------------------------------------------------------

mvAssetID
register_asset(mvAssetManager* manager, const std::string& tag, mvCamera asset)
{
	manager->cameras[manager->cameraCount].asset = asset;
	manager->cameras[manager->cameraCount].hash = tag;
	manager->cameraCount++;
	return manager->cameraCount - 1;
}

mvAssetID
mvGetCameraAssetID(mvAssetManager* manager, const std::string& tag)
{
	for (s32 i = 0; i < manager->cameraCount; i++)
	{
		if (manager->cameras[i].hash == tag)
			return i;
	}

	return -1;
}

mvCamera*
mvGetRawCameraAsset(mvAssetManager* manager, const std::string& tag)
{
	for (s32 i = 0; i < manager->cameraCount; i++)
	{
		if (manager->cameras[i].hash == tag)
			return &manager->cameras[i].asset;
	}

	return nullptr;
}

//-----------------------------------------------------------------------------
// samplers
//-----------------------------------------------------------------------------

mvAssetID
register_asset(mvAssetManager* manager, const std::string& tag, mvSampler asset)
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

mvSampler*
mvGetRawSamplerAsset(mvAssetManager* manager, const std::string& tag)
{
	for (s32 i = 0; i < manager->samplerCount; i++)
	{
		if (manager->samplers[i].hash == tag)
			return &manager->samplers[i].asset;
	}

	return nullptr;
}
