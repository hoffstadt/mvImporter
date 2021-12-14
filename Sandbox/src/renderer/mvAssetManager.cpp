#include "mvAssetManager.h"

void 
mvInitializeAssetManager(mvAssetManager* manager)
{
	manager->textures = new mvTextureAsset[manager->maxTextureCount];
	manager->samplers = new mvSamplerAsset[manager->maxSamplerCount];
	manager->materials = new mvMaterialAsset[manager->maxMaterialCount];
	manager->buffers = new mvBufferAsset[manager->maxBufferCount];
	manager->meshes = new mvMeshAsset[manager->maxMeshCount];
	manager->cubeTextures = new mvCubeTextureAsset[manager->maxCubeTextureCount];
	manager->nodes = new mvNodeAsset[manager->maxNodeCount];
	manager->scenes = new mvSceneAsset[manager->maxSceneCount];
	manager->cameras = new mvCameraAsset[manager->maxCameraCount];
	manager->pipelines = new mvPipelineAsset[manager->maxPipelineCount];
}

void 
mvCleanupAssetManager(mvAssetManager* manager)
{
	for (int i = 0; i < manager->bufferCount; i++)
		manager->buffers[i].asset.buffer->Release();

	for (int i = 0; i < manager->materialCount; i++)
	{
		manager->materials[i].asset.buffer.buffer->Release();
	}

	for (int i = 0; i < manager->pipelineCount; i++)
	{
		manager->pipelines[i].asset.vertexShader->Release();
		manager->pipelines[i].asset.vertexBlob->Release();
		manager->pipelines[i].asset.inputLayout->Release();
		manager->pipelines[i].asset.blendState->Release();
		manager->pipelines[i].asset.depthStencilState->Release();
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
}

//-----------------------------------------------------------------------------
// pipelines
//-----------------------------------------------------------------------------
mvAssetID
mvRegisterAsset(mvAssetManager* manager, const std::string& tag, mvPipeline asset)
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
mvRegisterAsset(mvAssetManager* manager, const std::string& tag, mvScene asset)
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
mvRegisterAsset(mvAssetManager* manager, const std::string& tag, mvTexture asset)
{
	manager->textures[manager->textureCount].asset = asset;
	manager->textures[manager->textureCount].hash = tag;
	manager->textureCount++;
	return manager->textureCount - 1;
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
	manager->textures[manager->textureCount].asset = mvCreateTexture(path);
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
	manager->textures[manager->textureCount].asset = mvCreateTexture(data);
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
	manager->cubeTextures[manager->cubeTextureCount].asset = mvCreateCubeTexture(path);
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
	manager->textures[manager->textureCount].asset = mvCreateTexture(path);
	manager->textureCount++;
	return &manager->textures[manager->textureCount - 1].asset;
}

//-----------------------------------------------------------------------------
// meshes
//-----------------------------------------------------------------------------

mvAssetID
mvRegisterAsset(mvAssetManager* manager, const std::string& tag, mvMesh asset)
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
// uniform buffers
//-----------------------------------------------------------------------------

mvAssetID
mvRegisterAsset(mvAssetManager* manager, const std::string& tag, mvBuffer asset)
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
	manager->buffers[manager->bufferCount].asset = mvCreateBuffer(data, size, flags);
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
// materials
//-----------------------------------------------------------------------------

mvAssetID
mvRegisterAsset(mvAssetManager* manager, const std::string& tag, mvMaterial asset)
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
mvRegisterAsset(mvAssetManager* manager, const std::string& tag, mvNode asset)
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
mvRegisterAsset(mvAssetManager* manager, const std::string& tag, mvCamera asset)
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
mvRegisterAsset(mvAssetManager* manager, const std::string& tag, mvSampler asset)
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
