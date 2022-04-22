/*
 mvImporter, v0.0.1 (WIP)
   * no dependencies
   * assumes GLTF file is correct
   * contains a hacky but minimal json parser

   Do this:
	  #define MV_IMPORTER_IMPLEMENTATION
   before you include this file in *one* C++ file to create the implementation.
   // i.e. it should look like this:
   #include ...
   #include ...
   #include ...
   #define MV_IMPORTER_IMPLEMENTATION
   #include "mvImporter.h"
*/

#ifndef MV_IMPORTER_H
#define MV_IMPORTER_H

#ifndef MV_IMPORTER_API
#define MV_IMPORTER_API
#endif

#ifndef MV_IMPORTER_MAX_STRING_LENGTH
#define MV_IMPORTER_MAX_STRING_LENGTH 256
#endif

#ifndef MV_IMPORTER_MAX_NAME_LENGTH
#define MV_IMPORTER_MAX_NAME_LENGTH 256
#endif

#include <string> // temporary
#include <assert.h>

#ifdef MV_IMPORTER_IMPLEMENTATION
#include <stdio.h>
#include <stdlib.h>
#endif // MV_IMPORTER_IMPLEMENTATION

#include "sJsonParser.h"

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
struct mvGLTFAnimationChannelTarget; // GLTF -> "animations.channel.target"
struct mvGLTFAnimationChannel;       // GLTF -> "animations.channels"
struct mvGLTFAnimationSampler;       // GLTF -> "animations.samplers"
struct mvGLTFAnimation;              // GLTF -> "animations"
struct mvGLTFCamera;                 // GLTF -> "cameras"
struct mvGLTFPerspective;            // GLTF -> "perspective"
struct mvGLTFOrthographic;           // GLTF -> "orthographic"
struct mvGLTFNode;                   // GLTF -> "nodes"
struct mvGLTFSkin;                   // GLTF -> "skins"
struct mvGLTFScene;                  // GLTF -> "scenes"
struct mvGLTFMorphTarget;            // GLTF -> "meshPrimitives.targets"
struct mvGLTFMeshPrimitive;          // GLTF -> "meshPrimitives"
struct mvGLTFMesh;                   // GLTF -> "meshes"
struct mvGLTFMaterial;               // GLTF -> "materials"
struct mvGLTFTexture;                // GLTF -> "textures"
struct mvGLTFSampler;                // GLTF -> "samplers"
struct mvGLTFImage;                  // GLTF -> "images"
struct mvGLTFBuffer;                 // GLTF -> "buffers"
struct mvGLTFBufferView;             // GLTF -> "bufferViews"
struct mvGLTFAccessor;               // GLTF -> "accessors
struct mvGLTFAttribute;              // GLTF -> "meshes" > "primitives" > "attributes"
struct mvGLTFModel;                  // contains arrays of the above and counts

//-----------------------------------------------------------------------------
// mvImporter End-User API
//-----------------------------------------------------------------------------

MV_IMPORTER_API mvGLTFModel mvLoadGLTF      (const char* root, const char* file);
MV_IMPORTER_API void        mvCleanupGLTF   (mvGLTFModel& model);

//-----------------------------------------------------------------------------
// Flags, Enumerations, & Struct Definitions
//-----------------------------------------------------------------------------

enum mvGLTFAlphaMode
{
	MV_ALPHA_MODE_OPAQUE = 0,
	MV_ALPHA_MODE_MASK   = 1,
	MV_ALPHA_MODE_BLEND  = 2
};

enum mvGLTFPrimMode
{
	MV_IMP_POINTS    = 0,
	MV_IMP_LINES     = 1,
	MV_IMP_TRIANGLES = 4
};

enum mvGLTFAccessorType
{
	MV_IMP_SCALAR,
	MV_IMP_VEC2,
	MV_IMP_VEC3,
	MV_IMP_VEC4,
	MV_IMP_MAT2,
	MV_IMP_MAT3,
	MV_IMP_MAT4,
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

enum mvGLTFFilter
{
	MV_IMP_FILTER_NEAREST                = 9728,
	MV_IMP_FILTER_LINEAR                 = 9729,
	MV_IMP_FILTER_NEAREST_MIPMAP_NEAREST = 9984,
	MV_IMP_FILTER_LINEAR_MIPMAP_NEAREST  = 9985,
	MV_IMP_FILTER_NEAREST_MIPMAP_LINEAR  = 9986,
	MV_IMP_FILTER_LINEAR_MIPMAP_LINEAR   = 9987
};

enum mvGLTFWrapMode
{
	MV_IMP_WRAP_CLAMP_TO_EDGE   = 33071,
	MV_IMP_WRAP_MIRRORED_REPEAT = 33648,
	MV_IMP_WRAP_REPEAT          = 10497,
};

enum mvGLTFCameraType
{
	MV_IMP_PERSPECTIVE = 0,
	MV_IMP_ORTHOGRAPHIC = 1
};

struct mvGLTFAttribute
{
	char  semantic[MV_IMPORTER_MAX_NAME_LENGTH];
	int   index = -1; // accessor index
};

struct mvGLTFAccessor
{
	std::string         name;
	mvGLTFAccessorType  type = MV_IMP_SCALAR;
	int                 buffer_view_index = -1;
	mvGLTFComponentType component_type = MV_IMP_FLOAT;
	int                 byteOffset = 0;
	int                 count = -1;
	float               maxes[16];
	float               mins[16];
};

struct mvGLTFTexture
{
	std::string name;
	int         image_index   = -1;
	int         sampler_index = -1;
};

struct mvGLTFSampler
{
	std::string name;
	int         mag_filter = -1;
	int         min_filter = -1;
	int         wrap_s     = MV_IMP_WRAP_REPEAT;
	int         wrap_t     = MV_IMP_WRAP_REPEAT;
};

struct mvGLTFImage
{
	std::string    mimeType;
	std::string    uri;
	unsigned char* data;
	size_t         dataCount;
	bool           embedded;
	int            buffer_view_index = -1;
};

struct mvGLTFBuffer
{
	unsigned       byte_length = 0u;
	std::string    uri;
	unsigned char* data;
	size_t         dataCount;
};

struct mvGLTFBufferView
{
	std::string name;
	int         buffer_index = -1;
	int         byte_offset  =  0;
	int         byte_length  = -1;
	int         byte_stride  = -1;
};

struct mvGLTFMorphTarget
{
	mvGLTFAttribute* attributes = nullptr;
	unsigned         attribute_count = 0u;
};

struct mvGLTFMeshPrimitive
{
	int                indices_index = -1; // accessor index
	int                material_index = -1;
	mvGLTFPrimMode     mode = MV_IMP_TRIANGLES;
	mvGLTFAttribute*   attributes = nullptr;
	unsigned           attribute_count = 0u;
	mvGLTFMorphTarget* targets = nullptr;
	unsigned           target_count = 0u;
};

struct mvGLTFMesh
{
	std::string          name;
	mvGLTFMeshPrimitive* primitives = nullptr;
	unsigned             primitives_count = 0u;
	float*               weights = nullptr;
	unsigned             weights_count = 0u;
};

struct mvGLTFMaterial
{
	std::string     name;
	int             base_color_texture             = -1;
	int             metallic_roughness_texture     = -1;
	int             normal_texture                 = -1;
	int             occlusion_texture              = -1;
	int             emissive_texture               = -1;
	int             clearcoat_texture              = -1;
	int             clearcoat_roughness_texture    = -1;
	int             clearcoat_normal_texture       = -1;
	float           normal_texture_scale           = 1.0f;
	float           clearcoat_normal_texture_scale = 1.0f;
	float           occlusion_texture_strength     = 1.0f;
	float           metallic_factor                = 1.0f;
	float           roughness_factor               = 1.0f;
	float           base_color_factor[4]           = { 1.0f, 1.0f, 1.0f, 1.0f };
	float           emissive_factor[3]             = { 0.0f, 0.0f, 0.0f };
	float           alphaCutoff                    = 0.5;
	bool            double_sided                   = false;
	mvGLTFAlphaMode alphaMode                      = MV_ALPHA_MODE_OPAQUE;
	float           clearcoat_factor               = 0.0;
	float           clearcoat_roughness_factor     = 0.0;

	// extensions & models
	bool pbrMetallicRoughness  = false;
	bool clearcoat_extension   = false;
};

struct mvGLTFPerspective
{
	float aspectRatio = 0.0f;
	float yfov = 0.0f;
	float zfar = 0.0f;
	float znear = 0.0f;
};

struct mvGLTFOrthographic
{
	float xmag = 0.0f;
	float ymag = 0.0f;
	float zfar = 0.0f;
	float znear = 0.0f;
};

struct mvGLTFCamera
{
	std::string        name;
	mvGLTFCameraType   type = MV_IMP_PERSPECTIVE;
	mvGLTFPerspective  perspective;
	mvGLTFOrthographic orthographic;
};

struct mvGLTFNode
{
	std::string name;
	int         mesh_index   = -1;
	int         skin_index   = -1;
	int         camera_index = -1;
	unsigned*   children = nullptr;
	unsigned    child_count = 0u;
	float       matrix[16] = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f,
	};
	float       rotation[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float       scale[3]    = { 1.0f, 1.0f, 1.0f};
	float       translation[3]    = { 0.0f, 0.0f, 0.0f};
	bool        hadMatrix = false;
};

struct mvGLTFAnimationChannelTarget
{
	int         node = -1;
	std::string path;
};

struct mvGLTFAnimationChannel
{
	int                          sampler = -1;
	mvGLTFAnimationChannelTarget target;
};

struct mvGLTFAnimationSampler
{
	int         input = -1;
	int         output = -1;
	std::string interpolation = "LINEAR";
};

struct mvGLTFAnimation
{
	std::string             name;
	mvGLTFAnimationChannel* channels = nullptr;
	unsigned                channel_count = 0u;
	mvGLTFAnimationSampler* samplers = nullptr;
	unsigned                sampler_count = 0u;
};

struct mvGLTFScene
{
	unsigned* nodes = nullptr;
	unsigned  node_count = 0u;
};

struct mvGLTFSkin
{
	std::string name;
	int         inverseBindMatrices = -1;
	int         skeleton = -1;
	unsigned*   joints = nullptr;
	unsigned    joints_count = 0u;
};

struct mvGLTFModel
{
	std::string       root;
	std::string       name;
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
	mvGLTFCamera*     cameras     = nullptr;
	mvGLTFAnimation*  animations  = nullptr;
	mvGLTFSkin*       skins       = nullptr;
	std::string*      extensions  = nullptr;

	int      scene            = -1;
	unsigned scene_count      = 0u;
	unsigned node_count       = 0u;
	unsigned mesh_count       = 0u;
	unsigned material_count   = 0u;
	unsigned texture_count    = 0u;
	unsigned sampler_count    = 0u;
	unsigned image_count      = 0u;
	unsigned buffer_count     = 0u;
	unsigned bufferview_count = 0u;
	unsigned accessor_count   = 0u;
	unsigned camera_count     = 0u;
	unsigned animation_count  = 0u;
	unsigned skin_count       = 0u;
	unsigned extension_count  = 0u;
};

// end of header file
#endif // MV_IMPORTER_H

//-----------------------------------------------------------------------------
// mvImporter Implementation
//-----------------------------------------------------------------------------

#ifdef MV_IMPORTER_IMPLEMENTATION



#endif
