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
}

void 
mvCleanupAssetManager(mvAssetManager* manager)
{
	delete[] manager->textures;
	delete[] manager->cubeTextures;
	delete[] manager->samplers;
	delete[] manager->materials;
	delete[] manager->buffers;
	delete[] manager->meshes;
	delete[] manager->nodes;
	delete[] manager->scenes;
}

mvAssetID
mvGetMaterialAsset(mvAssetManager* manager, const std::string& vs, const std::string& ps, mvMaterialData& materialData)
{
	std::string hash = ps + vs +
		std::to_string(materialData.albedo.x) +
		std::to_string(materialData.albedo.y) +
		std::to_string(materialData.albedo.z) +
		std::to_string(materialData.albedo.w) +
		std::to_string(materialData.metalness) +
		std::to_string(materialData.roughness) +
		std::to_string(materialData.radiance) +
		std::to_string(materialData.fresnel) +
		std::string(materialData.hasAlpha ? "T" : "F") +
		std::string(materialData.useAlbedoMap ? "T" : "F") +
		std::string(materialData.useNormalMap ? "T" : "F") +
		std::string(materialData.useRoughnessMap ? "T" : "F") +
		std::string(materialData.useMetalMap ? "T" : "F");

	for (s32 i = 0; i < manager->materialCount; i++)
	{
		if (manager->materials[i].hash == hash)
			return i;
	}

	manager->materials[manager->materialCount].hash = hash;
	manager->materials[manager->materialCount].material = mvCreateMaterial(vs, ps, materialData);
	manager->materialCount++;
	return manager->materialCount - 1;
}

mvAssetID
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

mvAssetID
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

mvAssetID
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

mvAssetID
mvRegistryMeshAsset(mvAssetManager* manager, mvMesh mesh)
{
	manager->meshes[manager->meshCount].mesh = mesh;
	manager->meshCount++;
	return manager->meshCount - 1;
}

mvAssetID
mvRegistryNodeAsset(mvAssetManager* manager, mvNode node)
{
	manager->nodes[manager->nodeCount].node = node;
	manager->nodeCount++;
	return manager->nodeCount - 1;
}

mvAssetID
mvRegistrySceneAsset(mvAssetManager* manager, mvScene scene)
{
	manager->scenes[manager->sceneCount].scene = scene;
	manager->sceneCount++;
	return manager->sceneCount - 1;
}

mvAssetID
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