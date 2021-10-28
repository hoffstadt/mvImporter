#include "mvAssetManager.h"

void 
mvInitializeAssetManager(mvAssetManager* manager)
{
	manager->textures = new mvTextureAsset[manager->maxTextureCount];
	manager->samplers = new mvSamplerAsset[manager->maxSamplerCount];
	manager->phongMaterials = new mvPhongMaterialAsset[manager->maxPhongMaterialCount];
	manager->pbrMaterials = new mvPBRMaterialAsset[manager->maxPbrMaterialCount];
	manager->buffers = new mvBufferAsset[manager->maxBufferCount];
	manager->meshes = new mvMeshAsset[manager->maxMeshCount];
	manager->cubeTextures = new mvCubeTextureAsset[manager->maxCubeTextureCount];
}

void 
mvCleanupAssetManager(mvAssetManager* manager)
{
	delete[] manager->textures;
	delete[] manager->cubeTextures;
	delete[] manager->samplers;
	delete[] manager->phongMaterials;
	delete[] manager->pbrMaterials;
	delete[] manager->buffers;
	delete[] manager->meshes;
}

s32 
mvGetPhongMaterialAsset(mvAssetManager* manager, const std::string& vs, const std::string& ps, b8 cull, b8 useDiffusemap, b8 useNormalmap, b8 useSpecularMap)
{
	std::string hash = ps + vs + 
		std::string(cull ? "T" : "F") +
		std::string(useDiffusemap ? "T" : "F") +
		std::string(useNormalmap ? "T" : "F") +
		std::string(useSpecularMap ? "T" : "F");

	for (s32 i = 0; i < manager->phongMaterialCount; i++)
	{
		if (manager->phongMaterials[i].hash == hash)
			return i;
	}

	manager->phongMaterials[manager->phongMaterialCount].hash = hash;
	manager->phongMaterials[manager->phongMaterialCount].material = mvCreatePhongMaterial(vs, ps, cull, useDiffusemap, useNormalmap, useSpecularMap);
	manager->phongMaterials[manager->phongMaterialCount].material.data.hasAlpha = !cull;
	manager->phongMaterialCount++;
	return manager->phongMaterialCount - 1;
}

s32 
mvGetPBRMaterialAsset(mvAssetManager* manager, const std::string& vs, const std::string& ps, b8 cull, b8 useAlbedomap, b8 useNormalmap, b8 useRoughnessMap, b8 useMetalMap)
{
	std::string hash = ps + vs +
		std::string(cull ? "T" : "F") +
		std::string(useAlbedomap ? "T" : "F") +
		std::string(useNormalmap ? "T" : "F") +
		std::string(useRoughnessMap ? "T" : "F") +
		std::string(useMetalMap ? "T" : "F");

	for (s32 i = 0; i < manager->pbrMaterialCount; i++)
	{
		if (manager->pbrMaterials[i].hash == hash)
			return i;
	}

	manager->pbrMaterials[manager->pbrMaterialCount].hash = hash;
	manager->pbrMaterials[manager->pbrMaterialCount].material = mvCreatePBRMaterial(vs, ps, cull, useAlbedomap, useNormalmap, useRoughnessMap, useMetalMap);
	manager->pbrMaterials[manager->pbrMaterialCount].material.data.hasAlpha = !cull;
	manager->pbrMaterialCount++;
	return manager->pbrMaterialCount - 1;
}

s32 
mvGetTextureAsset(mvAssetManager* manager, const std::string& path)
{
	for (s32 i = 0; i < manager->textureCount; i++)
	{
		if (manager->textures[i].hash == path)
			return i;
	}

	manager->textures[manager->textureCount].hash = path;
	manager->textures[manager->textureCount].texture = mvCreateTexture(path);
	manager->textureCount++;
	return manager->textureCount - 1;
}

s32
mvGetCubeTextureAsset(mvAssetManager* manager, const std::string& path)
{
	for (s32 i = 0; i < manager->cubeTextureCount; i++)
	{
		if (manager->cubeTextures[i].hash == path)
			return i;
	}

	manager->cubeTextures[manager->cubeTextureCount].hash = path;
	manager->cubeTextures[manager->cubeTextureCount].texture = mvCreateCubeTexture(path);
	manager->cubeTextureCount++;
	return manager->cubeTextureCount - 1;
}

s32 
mvGetBufferAsset(mvAssetManager* manager, void* data, u32 size, D3D11_BIND_FLAG flags, const std::string& tag)
{
	for (s32 i = 0; i < manager->bufferCount; i++)
	{
		if (manager->buffers[i].hash == tag)
			return i;
	}

	manager->buffers[manager->bufferCount].hash = tag;
	manager->buffers[manager->bufferCount].buffer = mvCreateBuffer(data, size, flags);
	manager->bufferCount++;
	return manager->bufferCount - 1;
}

s32 
mvRegistryMeshAsset(mvAssetManager* manager, mvMesh mesh)
{
	manager->meshes[manager->meshCount].mesh = mesh;
	manager->meshCount++;
	return manager->meshCount - 1;
}

s32 
mvGetSamplerAsset(mvAssetManager* manager, D3D11_FILTER mode, D3D11_TEXTURE_ADDRESS_MODE addressing, b8 hwPcf)
{

	std::string hash =
		std::to_string(mode) +
		std::to_string(addressing) +
		std::string(hwPcf ? "T" : "F");

	for (u32 i = 0; i < manager->samplerCount; i++)
	{
		if (manager->samplers[i].hash == hash)
			return i;
	}

	manager->samplers[manager->samplerCount].hash = hash;
	manager->samplers[manager->samplerCount].sampler = mvCreateSampler(mode, addressing, hwPcf);
	manager->samplerCount++;
	return manager->samplerCount - 1;
}