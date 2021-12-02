#pragma once

#include <string>
#include "mvTypes.h"
#include "mvTextures.h"
#include "mvMaterials.h"
#include "mvBuffers.h"
#include "mvMesh.h"
#include "mvScene.h"

struct mvMeshAsset;
struct mvBufferAsset;
struct mvTextureAsset;
struct mvCubeTextureAsset;
struct mvSamplerAsset;
struct mvMaterialAsset;
struct mvSceneAsset;
struct mvNodeAsset;

struct mvAssetManager
{

	// textures
	u32                 maxTextureCount = 500u;
	u32                 textureCount = 0u;
	mvTextureAsset*     textures = nullptr;
						 
	// cube textures	 
	u32                 maxCubeTextureCount = 500u;
	u32                 cubeTextureCount = 0u;
	mvCubeTextureAsset* cubeTextures = nullptr;
						 
	// samplers			 
	u32                 maxSamplerCount = 500u;
	u32                 samplerCount = 0u;
	mvSamplerAsset*     samplers = nullptr;
				        
	// materials     
	u32                 maxMaterialCount = 500u;
	u32                 materialCount = 0u;
	mvMaterialAsset*    materials = nullptr;
					    				        
	// buffers	        
	u32                 maxBufferCount = 500u;
	u32                 bufferCount = 0u;
	mvBufferAsset*      buffers = nullptr;
				        
	// meshes	        
	u32                 maxMeshCount = 500u;
	u32                 meshCount = 0u;
	mvMeshAsset*        meshes = nullptr;
					    
	// nodes	        
	u32                 maxNodeCount = 500u;
	u32                 nodeCount = 0u;
	mvNodeAsset*        nodes = nullptr;
					    
	// scenes		    
	u32                 maxSceneCount = 500u;
	u32                 sceneCount = 0u;
	mvSceneAsset*       scenes = nullptr;
};

void mvInitializeAssetManager(mvAssetManager* manager);
void mvCleanupAssetManager   (mvAssetManager* manager);

mvAssetID mvGetMaterialAsset     (mvAssetManager* manager, const std::string& vs, const std::string& ps, mvMaterialData& materialData);
mvAssetID mvGetTextureAsset      (mvAssetManager* manager, const std::string& path);
mvAssetID mvGetCubeTextureAsset  (mvAssetManager* manager, const std::string& path);
mvAssetID mvGetBufferAsset       (mvAssetManager* manager, void* data, u32 size, D3D11_BIND_FLAG flags, const std::string& tag);
mvAssetID mvGetSamplerAsset      (mvAssetManager* manager, D3D11_FILTER mode, D3D11_TEXTURE_ADDRESS_MODE addressing, b8 hwPcf);

mvAssetID mvRegistryMeshAsset(mvAssetManager* manager, mvMesh mesh);
mvAssetID mvRegistryNodeAsset(mvAssetManager* manager, mvNode node);
mvAssetID mvRegistrySceneAsset(mvAssetManager* manager, mvScene scene);

struct mvMeshAsset
{
	mvMesh mesh;
};

struct mvSceneAsset
{
	mvScene scene;
};

struct mvNodeAsset
{
	mvNode node;
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

struct mvMaterialAsset
{
	std::string hash;
	mvMaterial material;
};