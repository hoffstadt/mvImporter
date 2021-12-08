#pragma once

#include <string>

#ifndef MV_IMPORTER_API
#define MV_IMPORTER_API
#endif

#include <assert.h>
#include <stdio.h>
#include <vector>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
struct mvGLTFNode;          // GLTF -> "nodes"
struct mvGLTFScene;         // GLTF -> "scenes"
struct mvGLTFMeshPrimitive; // GLTF -> "meshPrimitives"
struct mvGLTFMesh;          // GLTF -> "meshes"
struct mvGLTFMaterial;      // GLTF -> "materials"
struct mvGLTFTexture;       // GLTF -> "textures"
struct mvGLTFSampler;       // GLTF -> "samplers"
struct mvGLTFImage;         // GLTF -> "images"
struct mvGLTFBuffer;        // GLTF -> "buffers"
struct mvGLTFBufferView;    // GLTF -> "bufferViews"
struct mvGLTFAccessor;      // GLTF -> "accessors
struct mvGLTFAttribute;     // GLTF -> "meshes" > "primitives" > "attributes"
struct mvGLTFModel;         // contains arrays of the above and counts

//-----------------------------------------------------------------------------
// mvImporter End-User API
//-----------------------------------------------------------------------------

MV_IMPORTER_API mvGLTFModel mvLoadGLTF(const char* root, const char* file);
MV_IMPORTER_API mvGLTFModel mvLoadGLTF(const char* file);
MV_IMPORTER_API void        mvCleanupGLTF(mvGLTFModel& model);

//-----------------------------------------------------------------------------
// Basic Types
//-----------------------------------------------------------------------------

typedef float               mvF32;
typedef double              mvF64;
typedef signed char         mvS8;   // 8-bit signed integer
typedef unsigned char       mvU8;   // 8-bit unsigned integer
typedef signed short        mvS16;  // 16-bit signed integer
typedef unsigned short      mvU16;  // 16-bit unsigned integer
typedef signed int          mvS32;  // 32-bit signed integer == int
typedef unsigned int        mvU32;  // 32-bit unsigned integer (often used to store packed colors)
#if defined(_MSC_VER) && !defined(__clang__)
typedef signed   __int64    mvS64;  // 64-bit signed integer (pre and post C++11 with Visual Studio)
typedef unsigned __int64    mvU64;  // 64-bit unsigned integer (pre and post C++11 with Visual Studio)
#else
typedef signed   long long  mvS64;  // 64-bit signed integer (post C++11)
typedef unsigned long long  mvU64;  // 64-bit unsigned integer (post C++11)
#endif

//-----------------------------------------------------------------------------
// Flags, Enumerations, & Struct Definitions
//-----------------------------------------------------------------------------

enum mvGLTFPrimMode
{
	MV_IMP_POINTS    = 0,
	MV_IMP_LINES     = 1,
	MV_IMP_TRIANGLES = 4
};

enum mvGLTFPrimAttrType
{
	MV_IMP_POSITION  = 0,
	MV_IMP_TANGENT   = 1,
	MV_IMP_NORMAL    = 2,
	MV_IMP_TEXTCOORD = 3,
};

enum mvGLTFAccessorType
{
	MV_IMP_SCALAR,
	MV_IMP_VEC2,
	MV_IMP_VEC3,
	MV_IMP_VEC4,
};

enum mvGLTFComponentType
{
	MV_IMP_BYTE           = 5120,
	MV_IMP_UNSIGNED_BYTE  = 5121,
	MV_IMP_SHORT          = 5122,
	MV_IMP_UNSIGNED_SHORT = 5123,
	MV_IMP_INT            = 5124,
	MV_IMP_UNSIGNED_INT   = 5125,
	MV_IMP_FLOAT          = 5126,
	MV_IMP_DOUBLE         = 5130
};

enum mvGLTFWrapMode
{
	MV_IMP_WRAP_CLAMP_TO_EDGE   = 33071,
	MV_IMP_WRAP_MIRRORED_REPEAT = 33648,
	MV_IMP_WRAP_REPEAT          = 10497,
};

struct mvGLTFAttribute
{
	mvGLTFPrimAttrType type;
	mvS32              index = -1; // accessor index
};

struct mvGLTFAccessor
{
	std::string         name;
	mvGLTFAccessorType  type = MV_IMP_SCALAR;
	mvS32               buffer_view_index = -1;
	mvGLTFComponentType component_type = MV_IMP_FLOAT;
	mvS32               byteOffset = 0;
	mvS32               count = -1;
	mvF32               maxes[4];
	mvF32               mins[4];
};

struct mvGLTFTexture
{
	std::string name;
	mvS32       image_index   = -1;
	mvS32       sampler_index = -1;
};

struct mvGLTFSampler
{
	mvS32 mag_filter = -1;
	mvS32 min_filter = -1;
	mvS32 wrap_s     = -1;
	mvS32 wrap_t     = -1;
};

struct mvGLTFImage
{
	std::string                mimeType;
	std::string                uri;
	std::vector<unsigned char> data;
	bool                       embedded;
	mvS32                      buffer_view_index = -1;
};

struct mvGLTFBuffer
{
	mvU32                      byte_length = 0u;
	std::string                uri;
	std::vector<unsigned char> data;
};

struct mvGLTFBufferView
{
	std::string name;
	mvS32       buffer_index = -1;
	mvS32       byte_offset  =  0;
	mvS32       byte_length  = -1;
	mvS32       byte_stride  = -1;
};

struct mvGLTFMeshPrimitive
{
	mvS32            indices_index = -1; // accessor index
	mvS32            material_index = -1;
	mvGLTFPrimMode   mode = MV_IMP_TRIANGLES;
	mvGLTFAttribute* attributes = nullptr;
	mvU32            attribute_count = 0u;
};

struct mvGLTFMesh
{
	std::string          name;
	mvGLTFMeshPrimitive* primitives = nullptr;
	mvU32                primitives_count = 0u;
};

struct mvGLTFMaterial
{
	std::string name;
	mvS32       base_color_texture         = -1;
	mvS32       metallic_roughness_texture = -1;
	mvS32       normal_texture             = -1;
	mvS32       occlusion_texture          = -1;
	mvS32       emissive_texture           = -1;
	mvF32       normal_texture_scale       = 0.8f;
	mvF32       occlusion_texture_strength = 0.9f;
	mvF32       metallic_factor            = 1.0f;
	mvF32       roughness_factor           = 0.0f;
	mvF32       base_color_factor[4]       = { 1.0f, 1.0f, 1.0f, 1.0f };
	mvF32       emissive_factor[3]         = { 0.0f, 0.0f, 0.0f };
	bool        double_sided               = false;
	bool        alpha_mode                 = false;
};

struct mvGLTFNode
{
	std::string name;
	mvS32       mesh_index = -1;
	mvS32       skin_index = -1;
	mvU32*      children = nullptr;
	mvU32       child_count = 0u;
	mvF32       matrix[16] = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f,
	};
	mvF32       rotation[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	mvF32       scale[3]    = { 1.0f, 1.0f, 1.0f};
	mvF32       translation[3]    = { 0.0f, 0.0f, 0.0f};
	bool        hadMatrix = false;
};

struct mvGLTFScene
{
	mvU32* nodes = nullptr;
	mvU32  node_count = 0u;
};

struct mvGLTFModel
{
	std::string       root;
	mvGLTFScene*      scenes      = nullptr;
	mvGLTFNode*       nodes       = nullptr;
	mvGLTFMesh*       meshes      = nullptr;
	mvGLTFMaterial*   materials   = nullptr;
	mvGLTFTexture*    textures    = nullptr;
	mvGLTFSampler*    samplers    = nullptr;
	mvGLTFImage*      images      = nullptr;
	mvGLTFBuffer*     buffers     = nullptr;
	mvGLTFBufferView* bufferviews = nullptr;
	mvGLTFAccessor*   accessors   = nullptr;

	mvU32 scene_count      = 0u;
	mvU32 node_count       = 0u;
	mvU32 mesh_count       = 0u;
	mvU32 material_count   = 0u;
	mvU32 texture_count    = 0u;
	mvU32 sampler_count    = 0u;
	mvU32 image_count      = 0u;
	mvU32 buffer_count     = 0u;
	mvU32 bufferview_count = 0u;
	mvU32 accessor_count   = 0u;
};