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

// enums
typedef int mvGLTFAlphaMode;
typedef int mvGLTFPrimMode;
typedef int mvGLTFAccessorType;
typedef int mvGLTFComponentType;
typedef int mvGLTFFilter;
typedef int mvGLTFWrapMode;
typedef int mvGLTFCameraType;

//-----------------------------------------------------------------------------
// mvImporter End-User API
//-----------------------------------------------------------------------------

MV_IMPORTER_API mvGLTFModel mvLoadGLTF      (const char* root, const char* file);
MV_IMPORTER_API void        mvCleanupGLTF   (mvGLTFModel& model);

//-----------------------------------------------------------------------------
// Flags, Enumerations, & Struct Definitions
//-----------------------------------------------------------------------------

enum mvGLTFAlphaMode_
{
	MV_ALPHA_MODE_OPAQUE = 0,
	MV_ALPHA_MODE_MASK   = 1,
	MV_ALPHA_MODE_BLEND  = 2
};

enum mvGLTFPrimMode_
{
	MV_IMP_POINTS    = 0,
	MV_IMP_LINES     = 1,
	MV_IMP_TRIANGLES = 4
};

enum mvGLTFAccessorType_
{
	MV_IMP_SCALAR,
	MV_IMP_VEC2,
	MV_IMP_VEC3,
	MV_IMP_VEC4,
	MV_IMP_MAT2,
	MV_IMP_MAT3,
	MV_IMP_MAT4,
};

enum mvGLTFComponentType_
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

enum mvGLTFFilter_
{
	MV_IMP_FILTER_NEAREST                = 9728,
	MV_IMP_FILTER_LINEAR                 = 9729,
	MV_IMP_FILTER_NEAREST_MIPMAP_NEAREST = 9984,
	MV_IMP_FILTER_LINEAR_MIPMAP_NEAREST  = 9985,
	MV_IMP_FILTER_NEAREST_MIPMAP_LINEAR  = 9986,
	MV_IMP_FILTER_LINEAR_MIPMAP_LINEAR   = 9987
};

enum mvGLTFWrapMode_
{
	MV_IMP_WRAP_CLAMP_TO_EDGE   = 33071,
	MV_IMP_WRAP_MIRRORED_REPEAT = 33648,
	MV_IMP_WRAP_REPEAT          = 10497,
};

enum mvGLTFCameraType_
{
	MV_IMP_PERSPECTIVE = 0,
	MV_IMP_ORTHOGRAPHIC = 1
};

struct mvGLTFAttribute
{
	char  semantic[MV_IMPORTER_MAX_NAME_LENGTH];
	int   index;                                 // accessor index, default -1
};

struct mvGLTFAccessor
{
	std::string         name;              // default ""
	mvGLTFAccessorType  type;              // default MV_IMP_SCALAR
	int                 buffer_view_index; // default -1
	mvGLTFComponentType component_type;    // default MV_IMP_FLOAT
	int                 byteOffset;        // default 0
	int                 count;             // default -1
	float               maxes[16];
	float               mins[16];
};

struct mvGLTFTexture
{
	std::string name;          // default ""
	int         image_index;   // default -1
	int         sampler_index; // default -1
};

struct mvGLTFSampler
{
	std::string name;       // default ""
	int         mag_filter; // default -1
	int         min_filter; // default -1
	int         wrap_s;     // default MV_IMP_WRAP_REPEAT
	int         wrap_t;     // default MV_IMP_WRAP_REPEAT
};

struct mvGLTFImage
{
	std::string    mimeType;          // default ""
	std::string    uri;               // default ""
	unsigned char* data;              // default nullptr
	size_t         dataCount;         // default 0u
	bool           embedded;          // default
	int            buffer_view_index; // default -1
};

struct mvGLTFBuffer
{
	unsigned       byte_length; // default 0u
	std::string    uri;         // default ""
	unsigned char* data;        // default nullptr
	size_t         dataCount;   // default 0u
};

struct mvGLTFBufferView
{
	std::string name;
	int         buffer_index; // default -1
	int         byte_offset;  // default  0
	int         byte_length;  // default -1
	int         byte_stride;  // default -1
};

struct mvGLTFMorphTarget
{
	mvGLTFAttribute* attributes;      // default nullptr
	unsigned         attribute_count; // default 0u
};

struct mvGLTFMeshPrimitive
{
	int                indices_index;   // accessor index, default -1
	int                material_index;  // default -1
	mvGLTFPrimMode     mode;            // default MV_IMP_TRIANGLES
	mvGLTFAttribute*   attributes;      // default nullptr
	unsigned           attribute_count; // default 0u
	mvGLTFMorphTarget* targets;         // default nullptr
	unsigned           target_count;    // default 0u
};

struct mvGLTFMesh
{
	std::string          name;             // default ""
	mvGLTFMeshPrimitive* primitives;       // default nullptr
	unsigned             primitives_count; // default 0u
	float*               weights;          // default nullptr
	unsigned             weights_count;    // default 0u
};

struct mvGLTFMaterial
{
	std::string     name;                           // default ""
	int             base_color_texture;             // default -1
	int             metallic_roughness_texture;     // default -1
	int             normal_texture;                 // default -1
	int             occlusion_texture;              // default -1
	int             emissive_texture;               // default -1
	int             clearcoat_texture;              // default -1
	int             clearcoat_roughness_texture;    // default -1
	int             clearcoat_normal_texture;       // default -1
	float           normal_texture_scale;           // default 1.0f
	float           clearcoat_normal_texture_scale; // default 1.0f
	float           occlusion_texture_strength;     // default 1.0f
	float           metallic_factor;                // default 1.0f
	float           roughness_factor;               // default 1.0f
	float           base_color_factor[4];           // default { 1.0f, 1.0f, 1.0f, 1.0f }
	float           emissive_factor[3];             // default { 0.0f, 0.0f, 0.0f }
	float           alphaCutoff;                    // default 0.5f
	bool            double_sided;                   // default false
	mvGLTFAlphaMode alphaMode;                      // default MV_ALPHA_MODE_OPAQUE
	float           clearcoat_factor;               // default 0.0f
	float           clearcoat_roughness_factor;     // default 0.0f

	// extensions & models
	bool pbrMetallicRoughness; // default false
	bool clearcoat_extension;  // default false
};

struct mvGLTFPerspective
{
	float aspectRatio; // default 0.0f;
	float yfov;        // default 0.0f;
	float zfar;        // default 0.0f;
	float znear;       // default 0.0f;
};

struct mvGLTFOrthographic
{
	float xmag;  // default 0.0f;
	float ymag;  // default 0.0f;
	float zfar;  // default 0.0f;
	float znear; // default 0.0f;
};

struct mvGLTFCamera
{
	std::string        name; // default ""
	mvGLTFCameraType   type; // default MV_IMP_PERSPECTIVE
	mvGLTFPerspective  perspective;
	mvGLTFOrthographic orthographic;
};

struct mvGLTFNode
{
	std::string name;           // default ""
	int         mesh_index;     // default 0.0f;
	int         skin_index;     // default 0.0f;
	int         camera_index;   // default 0.0f;
	unsigned*   children;       // default nullptr;
	unsigned    child_count;    // default 0u;
	float       matrix[16];     // default identity
	float       rotation[4];    // default { 0.0f, 0.0f, 0.0f, 1.0f };
	float       scale[3];       // default { 1.0f, 1.0f, 1.0f};
	float       translation[3]; // default { 0.0f, 0.0f, 0.0f};
	bool        hadMatrix;      // default false;
};

struct mvGLTFAnimationChannelTarget
{
	int         node; // default -1
	std::string path;
};

struct mvGLTFAnimationChannel
{
	int                          sampler; // default -1
	mvGLTFAnimationChannelTarget target;
};

struct mvGLTFAnimationSampler
{
	int         input;         // default -1
	int         output;        // default -1
	std::string interpolation; // default "LINEAR"
};

struct mvGLTFAnimation
{
	std::string             name;          // default ""
	mvGLTFAnimationChannel* channels;      // default nullptr
	unsigned                channel_count; // default 0u
	mvGLTFAnimationSampler* samplers;      // default nullptr
	unsigned                sampler_count; // default 0u
};

struct mvGLTFScene
{
	unsigned* nodes;      // default nullptr
	unsigned  node_count; // default 0u
};

struct mvGLTFSkin
{
	std::string name;
	int         inverseBindMatrices; // default -1
	int         skeleton;            // default -1
	unsigned*   joints;              // default nullptr
	unsigned    joints_count;        // default 0u
};

struct mvGLTFModel
{
	std::string       root;        // default ""
	std::string       name;        // default ""
	mvGLTFScene*      scenes;      // default nullptr
	mvGLTFNode*       nodes;       // default nullptr
	mvGLTFMesh*       meshes;      // default nullptr
	mvGLTFMaterial*   materials;   // default nullptr
	mvGLTFTexture*    textures;    // default nullptr
	mvGLTFSampler*    samplers;    // default nullptr
	mvGLTFImage*      images;      // default nullptr
	mvGLTFBuffer*     buffers;     // default nullptr
	mvGLTFBufferView* bufferviews; // default nullptr
	mvGLTFAccessor*   accessors;   // default nullptr
	mvGLTFCamera*     cameras;     // default nullptr
	mvGLTFAnimation*  animations;  // default nullptr
	mvGLTFSkin*       skins;       // default nullptr
	std::string*      extensions;  // default nullptr

	int      scene;            // default -1
	unsigned scene_count;      // default 0u
	unsigned node_count;       // default 0u
	unsigned mesh_count;       // default 0u
	unsigned material_count;   // default 0u
	unsigned texture_count;    // default 0u
	unsigned sampler_count;    // default 0u
	unsigned image_count;      // default 0u
	unsigned buffer_count;     // default 0u
	unsigned bufferview_count; // default 0u
	unsigned accessor_count;   // default 0u
	unsigned camera_count;     // default 0u
	unsigned animation_count;  // default 0u
	unsigned skin_count;       // default 0u
	unsigned extension_count;  // default 0u
};

// end of header file
#endif // MV_IMPORTER_H

//-----------------------------------------------------------------------------
// mvImporter Implementation
//-----------------------------------------------------------------------------

#ifdef MV_IMPORTER_IMPLEMENTATION



#endif
