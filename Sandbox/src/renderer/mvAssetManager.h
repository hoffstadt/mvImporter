#pragma once

#include <string>
#include "mvTypes.h"
#include "mvTextures.h"
#include "mvMaterials.h"
#include "mvBuffers.h"
#include "mvMesh.h"
#include "mvScene.h"
#include "mvCamera.h"

struct mvTargetViewAsset;
struct mvDepthViewAsset;
struct mvMeshAsset;
struct mvBufferAsset;
struct mvCBufferAsset;
struct mvTextureAsset;
struct mvCubeTextureAsset;
struct mvSamplerAsset;
struct mvMaterialAsset;
struct mvSceneAsset;
struct mvNodeAsset;
struct mvCameraAsset;
struct mvPipelineAsset;
struct mvAssetManager;

void mvInitializeAssetManager(mvAssetManager* manager);
void mvCleanupAssetManager   (mvAssetManager* manager);

// registering
mvAssetID register_asset(mvAssetManager* manager, const std::string& tag, mvScene asset);
mvAssetID register_asset(mvAssetManager* manager, const std::string& tag, mvMesh asset);
mvAssetID register_asset(mvAssetManager* manager, const std::string& tag, mvBuffer asset);
mvAssetID register_asset(mvAssetManager* manager, const std::string& tag, mvConstBuffer asset);
mvAssetID register_asset(mvAssetManager* manager, const std::string& tag, mvMaterial asset);
mvAssetID register_asset(mvAssetManager* manager, const std::string& tag, mvNode asset);
mvAssetID register_asset(mvAssetManager* manager, const std::string& tag, mvTexture asset);
mvAssetID register_asset(mvAssetManager* manager, const std::string& tag, mvCubeTexture asset);
mvAssetID register_asset(mvAssetManager* manager, const std::string& tag, ID3D11SamplerState* asset);
mvAssetID register_asset(mvAssetManager* manager, const std::string& tag, mvCamera asset);
mvAssetID register_asset(mvAssetManager* manager, const std::string& tag, mvPipeline asset);
mvAssetID register_asset(mvAssetManager* manager, const std::string& tag, ID3D11RenderTargetView* asset);
mvAssetID register_asset(mvAssetManager* manager, const std::string& tag, ID3D11DepthStencilView* asset);

// unregistering
b8 unregister_texture_asset (mvAssetManager* manager, mvAssetID asset);
b8 unregister_buffer_asset  (mvAssetManager* manager, mvAssetID asset);
b8 unregister_mesh_asset    (mvAssetManager* manager, mvAssetID asset);
b8 unregister_camera_asset  (mvAssetManager* manager, mvAssetID asset);
b8 unregister_node_asset    (mvAssetManager* manager, mvAssetID asset);
b8 unregister_material_asset(mvAssetManager* manager, mvAssetID asset);
b8 unregister_scene_asset   (mvAssetManager* manager, mvAssetID asset);

// ID retrieval
mvAssetID mvGetSceneAssetID      (mvAssetManager* manager, const std::string& tag);
mvAssetID mvGetTextureAssetID    (mvAssetManager* manager, const std::string& path, const std::string& tag);
mvAssetID mvGetTextureAssetID    (mvAssetManager* manager, const std::string& tag, std::vector<unsigned char> data);
mvAssetID mvGetCubeTextureAssetID(mvAssetManager* manager, const std::string& path);
mvAssetID mvGetBufferAssetID     (mvAssetManager* manager, const std::string& tag, void* data, u32 size, D3D11_BIND_FLAG flags);
mvAssetID mvGetMaterialAssetID   (mvAssetManager* manager, const std::string& tag);
mvAssetID mvGetNodeAssetID       (mvAssetManager* manager, const std::string& tag);
mvAssetID mvGetSamplerAssetID    (mvAssetManager* manager, const std::string& tag);
mvAssetID mvGetCameraAssetID     (mvAssetManager* manager, const std::string& tag);
mvAssetID mvGetPipelineAssetID   (mvAssetManager* manager, const std::string& tag);
mvAssetID mvGetTargetViewAssetID (mvAssetManager* manager, const std::string& tag);
mvAssetID mvGetDepthViewAssetID  (mvAssetManager* manager, const std::string& tag);

// asset retrieval
mvPipeline*             mvGetRawPipelineAsset(mvAssetManager* manager, const std::string& tag);
mvScene*                mvGetRawSceneAsset   (mvAssetManager* manager, const std::string& tag);
mvTexture*              mvGetRawTextureAsset (mvAssetManager* manager, const std::string& tag);
mvMesh*                 mvGetRawMeshAsset    (mvAssetManager* manager, const std::string& tag);
mvBuffer*               mvGetRawBufferAsset  (mvAssetManager* manager, const std::string& tag);
mvConstBuffer*          mvGetRawCBufferAsset  (mvAssetManager* manager, const std::string& tag);
mvMaterial*             mvGetRawMaterialAsset(mvAssetManager* manager, const std::string& tag);
mvNode*                 mvGetRawNodeAsset    (mvAssetManager* manager, const std::string& tag);
ID3D11SamplerState*     mvGetRawSamplerAsset (mvAssetManager* manager, const std::string& tag);
mvCamera*               mvGetRawCameraAsset  (mvAssetManager* manager, const std::string& tag);
ID3D11RenderTargetView* mvGetRawTargetViewAsset  (mvAssetManager* manager, const std::string& tag);
ID3D11DepthStencilView* mvGetRawDepthViewAsset  (mvAssetManager* manager, const std::string& tag);

struct mvAssetManager
{

	// render target views
	u32                 maxTargetViewCount = 10u;
	u32                 targetViewCount = 0u;
	mvTargetViewAsset*  targetViews = nullptr;

	// depth views
	u32                 maxDepthViewCount = 10u;
	u32                 depthViewCount = 0u;
	mvDepthViewAsset*   depthViews = nullptr;

	// textures
	u32                 maxTextureCount = 500u;
	u32                 textureCount = 0u;
	mvTextureAsset*     textures = nullptr;
	b8*                 freetextures = nullptr;
						 
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
	b8*                 freematerials = nullptr;
					    				        
	// buffers	        
	u32                 maxBufferCount = 500u;
	u32                 bufferCount = 0u;
	mvBufferAsset*      buffers = nullptr;
	b8*                 freebuffers = nullptr;

	// const buffers	        
	u32                 maxCBufferCount = 500u;
	u32                 cbufferCount = 0u;
	mvCBufferAsset*     cbuffers = nullptr;
				        
	// meshes	        
	u32                 maxMeshCount = 500u;
	u32                 meshCount = 0u;
	mvMeshAsset*        meshes = nullptr;
	b8*                 freemeshes = nullptr;
					    
	// nodes	        
	u32                 maxNodeCount = 1000u;
	u32                 nodeCount = 0u;
	mvNodeAsset*        nodes = nullptr;
	b8*                 freenodes = nullptr;
					    
	// scenes		    
	u32                 maxSceneCount = 500u;
	u32                 sceneCount = 0u;
	mvSceneAsset*       scenes = nullptr;
	b8*                 freescenes = nullptr;

	// cameras
	u32                 maxCameraCount = 5u;
	u32                 cameraCount = 0u;
	mvCameraAsset*      cameras = nullptr;
	b8*                 freecameras = nullptr;

	// pipelines
	u32                 maxPipelineCount = 1000u;
	u32                 pipelineCount = 0u;
	mvPipelineAsset*    pipelines = nullptr;
};

struct mvDepthViewAsset
{
	std::string             hash;
	ID3D11DepthStencilView* asset;
};

struct mvTargetViewAsset
{
	std::string             hash;
	ID3D11RenderTargetView* asset;
};

struct mvPipelineAsset
{
	std::string hash;
	mvPipeline asset;
};

struct mvMeshAsset
{
	std::string hash;
	mvMesh asset;
};

struct mvSceneAsset
{
	std::string hash;
	mvScene asset;
};

struct mvNodeAsset
{
	std::string hash;
	mvNode asset;
};

struct mvBufferAsset
{
	std::string hash;
	mvBuffer    asset;
};

struct mvCBufferAsset
{
	std::string   hash;
	mvConstBuffer asset;
};

struct mvTextureAsset
{
	std::string hash;
	mvTexture   asset;
};

struct mvCubeTextureAsset
{
	std::string   hash;
	mvCubeTexture asset;
};

struct mvSamplerAsset
{
	std::string         hash;
	ID3D11SamplerState* asset;
};

struct mvMaterialAsset
{
	std::string hash;
	mvMaterial asset;
};

struct mvCameraAsset
{
	std::string hash;
	mvCamera asset;
};