#pragma once

#include <string>
#include "mvTypes.h"
#include "mvTextures.h"
#include "mvMaterials.h"
#include "mvBuffers.h"
#include "mvMesh.h"

struct mvMeshAsset;
struct mvBufferAsset;
struct mvTextureAsset;
struct mvCubeTextureAsset;
struct mvSamplerAsset;
struct mvPhongMaterialAsset;
struct mvPBRMaterialAsset;

struct mvAssetManager
{

	// textures
	u32                   maxTextureCount = 100u;
	u32                   textureCount = 0u;
	mvTextureAsset*       textures = nullptr;
						  
	// cube textures	  
	u32                   maxCubeTextureCount = 100u;
	u32                   cubeTextureCount = 0u;
	mvCubeTextureAsset*   cubeTextures = nullptr;
						  
	// samplers			  
	u32                   maxSamplerCount = 100u;
	u32                   samplerCount = 0u;
	mvSamplerAsset*       samplers = nullptr;
				       
	// phong materials       
	u32                   maxPhongMaterialCount = 100u;
	u32                   phongMaterialCount = 0u;
	mvPhongMaterialAsset* phongMaterials = nullptr;

	// pbr materials       
	u32                   maxPbrMaterialCount = 100u;
	u32                   pbrMaterialCount = 0u;
	mvPBRMaterialAsset*   pbrMaterials = nullptr;
				       
	// buffers	       
	u32                   maxBufferCount = 100u;
	u32                   bufferCount = 0u;
	mvBufferAsset*        buffers = nullptr;
				       	  
	// meshes	       	  
	u32                   maxMeshCount = 100u;
	u32                   meshCount = 0u;
	mvMeshAsset*          meshes = nullptr;
};

void mvInitializeAssetManager(mvAssetManager* manager);
void mvCleanupAssetManager   (mvAssetManager* manager);

s32 mvGetPhongMaterialAsset(mvAssetManager* manager, const std::string& vs, const std::string& ps, b8 cull, b8 useDiffusemap, b8 useNormalmap, b8 useSpecularMap);
s32 mvGetPBRMaterialAsset  (mvAssetManager* manager, const std::string& vs, const std::string& ps, b8 cull, b8 useAlbedomap, b8 useNormalmap, b8 useRoughnessMap, b8 useMetalMap);
s32 mvGetTextureAsset      (mvAssetManager* manager, const std::string& path);
s32 mvGetCubeTextureAsset  (mvAssetManager* manager, const std::string& path);
s32 mvGetBufferAsset       (mvAssetManager* manager, void* data, u32 size, D3D11_BIND_FLAG flags, const std::string& tag);
s32 mvGetSamplerAsset      (mvAssetManager* manager, D3D11_FILTER mode, D3D11_TEXTURE_ADDRESS_MODE addressing, b8 hwPcf);

s32 mvRegistryMeshAsset(mvAssetManager* manager, mvMesh mesh);

struct mvMeshAsset
{
	mvMesh mesh;
};

struct mvBufferAsset
{
	std::string hash;
	mvBuffer    buffer;
};

struct mvTextureAsset
{
	std::string hash;
	mvTexture   texture;
};

struct mvCubeTextureAsset
{
	std::string   hash;
	mvCubeTexture texture;
};

struct mvSamplerAsset
{
	std::string hash;
	mvSampler   sampler;
};

struct mvPhongMaterialAsset
{
	std::string     hash;
	mvPhongMaterial material;
};

struct mvPBRMaterialAsset
{
	std::string   hash;
	mvPBRMaterial material;
};