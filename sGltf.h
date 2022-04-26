/*
 sGltf, v0.0.1 (WIP)
   * no dependencies
   * assumes GLTF file is correct

   Do this:
	  #define S_GLTF_IMPLEMENTATION
   before you include this file in *one* C++ file to create the implementation.
   // i.e. it should look like this:
   #include ...
   #include ...
   #include ...
   #define S_GLTF_IMPLEMENTATION
   #include "sGltf.h"
*/

#ifndef S_GLTF_H
#define S_GLTF_H

#ifndef S_GLTF_MAX_STRING_LENGTH
#define S_GLTF_MAX_STRING_LENGTH 256
#endif

#ifndef S_GLTF_MAX_NAME_LENGTH
#define S_GLTF_MAX_NAME_LENGTH 256
#endif

#include <string> // temporary

#ifndef S_GLTF_ALLOC
#define S_GLTF_ALLOC(x) malloc(x)
#endif

#ifndef S_GLTF_FREE
#define S_GLTF_FREE(x) free(x)
#endif

#ifndef S_GLTF_ASSERT
#include <assert.h>
#define S_GLTF_ASSERT(x) assert(x)
#endif

#ifdef S_GLTF_IMPLEMENTATION
#include <stdio.h>
#include <stdlib.h>
#endif // S_GLTF_IMPLEMENTATION

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
struct sGLTFAnimationChannelTarget; // GLTF -> "animations.channel.target"
struct sGLTFAnimationChannel;       // GLTF -> "animations.channels"
struct sGLTFAnimationSampler;       // GLTF -> "animations.samplers"
struct sGLTFAnimation;              // GLTF -> "animations"
struct sGLTFCamera;                 // GLTF -> "cameras"
struct sGLTFPerspective;            // GLTF -> "perspective"
struct sGLTFOrthographic;           // GLTF -> "orthographic"
struct sGLTFNode;                   // GLTF -> "nodes"
struct sGLTFSkin;                   // GLTF -> "skins"
struct sGLTFScene;                  // GLTF -> "scenes"
struct sGLTFMorphTarget;            // GLTF -> "meshPrimitives.targets"
struct sGLTFMeshPrimitive;          // GLTF -> "meshPrimitives"
struct sGLTFMesh;                   // GLTF -> "meshes"
struct sGLTFMaterial;               // GLTF -> "materials"
struct sGLTFTexture;                // GLTF -> "textures"
struct sGLTFSampler;                // GLTF -> "samplers"
struct sGLTFImage;                  // GLTF -> "images"
struct sGLTFBuffer;                 // GLTF -> "buffers"
struct sGLTFBufferView;             // GLTF -> "bufferViews"
struct sGLTFAccessor;               // GLTF -> "accessors
struct sGLTFAttribute;              // GLTF -> "meshes" > "primitives" > "attributes"
struct sGLTFModel;                  // contains arrays of the above and counts

// enums
typedef int sGLTFAlphaMode;
typedef int sGLTFPrimMode;
typedef int sGLTFAccessorType;
typedef int sGLTFComponentType;
typedef int sGLTFFilter;
typedef int sGLTFWrapMode;
typedef int sGLTFCameraType;

//-----------------------------------------------------------------------------
// sImporter End-User API
//-----------------------------------------------------------------------------
namespace Semper
{
	sGLTFModel load_gltf(const char* root, const char* file);
	void       free_gltf(sGLTFModel& model);
}

//-----------------------------------------------------------------------------
// Flags, Enumerations, & Struct Definitions
//-----------------------------------------------------------------------------

enum sGLTFAlphaMode_
{
	S_ALPHA_MODE_OPAQUE = 0,
	S_ALPHA_MODE_MASK   = 1,
	S_ALPHA_MODE_BLEND  = 2
};

enum sGLTFPrimMode_
{
	S_GLTF_POINTS    = 0,
	S_GLTF_LINES     = 1,
	S_GLTF_TRIANGLES = 4
};

enum sGLTFAccessorType_
{
	S_GLTF_SCALAR,
	S_GLTF_VEC2,
	S_GLTF_VEC3,
	S_GLTF_VEC4,
	S_GLTF_MAT2,
	S_GLTF_MAT3,
	S_GLTF_MAT4,
};

enum sGLTFComponentType_
{
	S_GLTF_BYTE           = 5120,
	S_GLTF_UNSIGNED_BYTE  = 5121,
	S_GLTF_SHORT          = 5122,
	S_GLTF_UNSIGNED_SHORT = 5123,
	S_GLTF_INT            = 5124,
	S_GLTF_UNSIGNED_INT   = 5125,
	S_GLTF_FLOAT          = 5126,
	S_GLTF_DOUBLE         = 5130
};

enum sGLTFFilter_
{
	S_GLTF_FILTER_NEAREST                = 9728,
	S_GLTF_FILTER_LINEAR                 = 9729,
	S_GLTF_FILTER_NEAREST_MIPMAP_NEAREST = 9984,
	S_GLTF_FILTER_LINEAR_MIPMAP_NEAREST  = 9985,
	S_GLTF_FILTER_NEAREST_MIPMAP_LINEAR  = 9986,
	S_GLTF_FILTER_LINEAR_MIPMAP_LINEAR   = 9987
};

enum sGLTFWrapMode_
{
	S_GLTF_WRAP_CLAMP_TO_EDGE   = 33071,
	S_GLTF_WRAP_MIRRORED_REPEAT = 33648,
	S_GLTF_WRAP_REPEAT          = 10497,
};

enum sGLTFCameraType_
{
	S_GLTF_PERSPECTIVE = 0,
	S_GLTF_ORTHOGRAPHIC = 1
};

struct sGLTFAttribute
{
	char  semantic[S_GLTF_MAX_NAME_LENGTH];
	int   index; // accessor index, default -1
};

struct sGLTFAccessor
{
	char               name[S_GLTF_MAX_NAME_LENGTH]; // default ""
	sGLTFAccessorType  type;                         // default S_GLTF_SCALAR
	int                buffer_view_index;            // default -1
	sGLTFComponentType component_type;               // default S_GLTF_FLOAT
	int                byteOffset;                   // default 0
	int                count;                        // default -1
	float              maxes[16];
	float              mins[16];
};

struct sGLTFTexture
{
	char name[S_GLTF_MAX_NAME_LENGTH]; // default ""
	int  image_index;                  // default -1
	int  sampler_index;                // default -1
};

struct sGLTFSampler
{
	char name[S_GLTF_MAX_NAME_LENGTH]; // default ""
	int  mag_filter;                   // default -1
	int  min_filter;                   // default -1
	int  wrap_s;                       // default S_GLTF_WRAP_REPEAT
	int  wrap_t;                       // default S_GLTF_WRAP_REPEAT
};

struct sGLTFImage
{
	std::string    mimeType;          // default ""
	std::string    uri;               // default ""
	unsigned char* data;              // default nullptr
	size_t         dataCount;         // default 0u
	bool           embedded;          // default
	int            buffer_view_index; // default -1
};

struct sGLTFBuffer
{
	unsigned       byte_length; // default 0u
	std::string    uri;         // default ""
	unsigned char* data;        // default nullptr
	size_t         dataCount;   // default 0u
};

struct sGLTFBufferView
{
	char name[S_GLTF_MAX_NAME_LENGTH]; // default ""
	int  buffer_index;                 // default -1
	int  byte_offset;                  // default  0
	int  byte_length;                  // default -1
	int  byte_stride;                  // default -1
};

struct sGLTFMorphTarget
{
	sGLTFAttribute* attributes;       // default nullptr
	unsigned        attribute_count; // default 0u
};

struct sGLTFMeshPrimitive
{
	int               indices_index;   // accessor index, default -1
	int               material_index;  // default -1
	sGLTFPrimMode     mode;             // default S_GLTF_TRIANGLES
	sGLTFAttribute*   attributes;       // default nullptr
	unsigned          attribute_count; // default 0u
	sGLTFMorphTarget* targets;          // default nullptr
	unsigned          target_count;    // default 0u
};

struct sGLTFMesh
{
	char                name[S_GLTF_MAX_NAME_LENGTH]; // default ""
	sGLTFMeshPrimitive* primitives;                   // default nullptr
	unsigned            primitives_count;             // default 0u
	float*              weights;                      // default nullptr
	unsigned            weights_count;                // default 0u
};

struct sGLTFMaterial
{
	char           name[S_GLTF_MAX_NAME_LENGTH];   // default ""
	int            base_color_texture;             // default -1
	int            metallic_roughness_texture;     // default -1
	int            normal_texture;                 // default -1
	int            occlusion_texture;              // default -1
	int            emissive_texture;               // default -1
	int            clearcoat_texture;              // default -1
	int            clearcoat_roughness_texture;    // default -1
	int            clearcoat_normal_texture;       // default -1
	float          normal_texture_scale;           // default 1.0f
	float          clearcoat_normal_texture_scale; // default 1.0f
	float          occlusion_texture_strength;     // default 1.0f
	float          metallic_factor;                // default 1.0f
	float          roughness_factor;               // default 1.0f
	float          base_color_factor[4];           // default { 1.0f, 1.0f, 1.0f, 1.0f }
	float          emissive_factor[3];             // default { 0.0f, 0.0f, 0.0f }
	float          alphaCutoff;                    // default 0.5f
	bool           double_sided;                   // default false
	sGLTFAlphaMode alphaMode;                      // default S_ALPHA_MODE_OPAQUE
	float          clearcoat_factor;               // default 0.0f
	float          clearcoat_roughness_factor;     // default 0.0f

	// extensions & models
	bool pbrMetallicRoughness; // default false
	bool clearcoat_extension;  // default false
};

struct sGLTFPerspective
{
	float aspectRatio; // default 0.0f;
	float yfov;        // default 0.0f;
	float zfar;        // default 0.0f;
	float znear;       // default 0.0f;
};

struct sGLTFOrthographic
{
	float xmag;  // default 0.0f;
	float ymag;  // default 0.0f;
	float zfar;  // default 0.0f;
	float znear; // default 0.0f;
};

struct sGLTFCamera
{
	char              name[S_GLTF_MAX_NAME_LENGTH]; // default ""
	sGLTFCameraType   type; // default S_GLTF_PERSPECTIVE
	sGLTFPerspective  perspective;
	sGLTFOrthographic orthographic;
};

struct sGLTFNode
{
	char        name[S_GLTF_MAX_NAME_LENGTH]; // default ""
	int         mesh_index;                   // default 0.0f;
	int         skin_index;                   // default 0.0f;
	int         camera_index;                 // default 0.0f;
	unsigned*   children;                     // default nullptr;
	unsigned    child_count;                  // default 0u;
	float       matrix[16];                   // default identity
	float       rotation[4];                  // default { 0.0f, 0.0f, 0.0f, 1.0f };
	float       scale[3];                     // default { 1.0f, 1.0f, 1.0f};
	float       translation[3];               // default { 0.0f, 0.0f, 0.0f};
	bool        hadMatrix;                    // default false;
};

struct sGLTFAnimationChannelTarget
{
	int  node; // default -1
	char path[S_GLTF_MAX_NAME_LENGTH];
};

struct sGLTFAnimationChannel
{
	int                         sampler; // default -1
	sGLTFAnimationChannelTarget target;
};

struct sGLTFAnimationSampler
{
	int  input;                                 // default -1
	int  output;                                // default -1
	char interpolation[S_GLTF_MAX_NAME_LENGTH]; // default "LINEAR"
};

struct sGLTFAnimation
{
	char                   name[S_GLTF_MAX_NAME_LENGTH]; // default ""
	sGLTFAnimationChannel* channels;                     // default nullptr
	unsigned               channel_count;                // default 0u
	sGLTFAnimationSampler* samplers;                     // default nullptr
	unsigned               sampler_count;                // default 0u
};

struct sGLTFScene
{
	unsigned* nodes;      // default nullptr
	unsigned  node_count; // default 0u
};

struct sGLTFSkin
{
	char        name[S_GLTF_MAX_NAME_LENGTH]; // default ""
	int         inverseBindMatrices;          // default -1
	int         skeleton;                     // default -1
	unsigned*   joints;                       // default nullptr
	unsigned    joints_count;                 // default 0u
};

struct sGLTFModel
{
	const char*      root;
	const char*      name;
	sGLTFScene*      scenes;      // default nullptr
	sGLTFNode*       nodes;       // default nullptr
	sGLTFMesh*       meshes;      // default nullptr
	sGLTFMaterial*   materials;   // default nullptr
	sGLTFTexture*    textures;    // default nullptr
	sGLTFSampler*    samplers;    // default nullptr
	sGLTFImage*      images;      // default nullptr
	sGLTFBuffer*     buffers;     // default nullptr
	sGLTFBufferView* bufferviews; // default nullptr
	sGLTFAccessor*   accessors;   // default nullptr
	sGLTFCamera*     cameras;     // default nullptr
	sGLTFAnimation*  animations;  // default nullptr
	sGLTFSkin*       skins;       // default nullptr
	char**           extensions;  // default nullptr

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
#endif

//-----------------------------------------------------------------------------
// sGLTF Implementation
//-----------------------------------------------------------------------------

//#define S_GLTF_IMPLEMENTATION
#ifdef S_GLTF_IMPLEMENTATION

//sJson, v0.1 (WIP)

#ifndef S_GLTF_JSON_MAX_NAME_LENGTH
#define S_GLTF_JSON_MAX_NAME_LENGTH 256
#endif

//-----------------------------------------------------------------------------
// [SECTION] Forward declarations and basic types
//-----------------------------------------------------------------------------
struct sGltfJsonObject;
typedef int sGltfJsonType; // enum -> sGltfJsonType_

//-----------------------------------------------------------------------------
// [SECTION] Semper end-user API functions
//-----------------------------------------------------------------------------
namespace Semper
{
	sGltfJsonObject* load_json(char* rawData, int size);
	void             free_json(sGltfJsonObject** rootObject);
}

//-----------------------------------------------------------------------------
// [SECTION] Enums
//-----------------------------------------------------------------------------

enum sGltfJsonType_
{
	S_GLTF_JSON_TYPE_NONE,
	S_GLTF_JSON_TYPE_STRING,
	S_GLTF_JSON_TYPE_ARRAY,
	S_GLTF_JSON_TYPE_NUMBER,
	S_GLTF_JSON_TYPE_BOOL,
	S_GLTF_JSON_TYPE_OBJECT,
	S_GLTF_JSON_TYPE_NULL,
};

//-----------------------------------------------------------------------------
// [SECTION] Structs
//-----------------------------------------------------------------------------

struct sGltfJsonObject
{
	sGltfJsonType    type;
	sGltfJsonObject* children;
	int              childCount;	
	char             name[S_GLTF_JSON_MAX_NAME_LENGTH];
	char*            value;
	void*            _internal;

	// retrieve members
	inline sGltfJsonObject* getMember      (const char* member){ for (int i = 0; i < childCount; i++) if (strcmp(member, children[i].name) == 0) return &children[i]; return nullptr;}
	inline bool             doesMemberExist(const char* member){ return getMember(member) != nullptr;}

	// cast values
	inline int      asInt()    { S_GLTF_ASSERT(type == S_GLTF_JSON_TYPE_NUMBER); return (int)strtod(value, nullptr);}
	inline unsigned asUInt()   { S_GLTF_ASSERT(type == S_GLTF_JSON_TYPE_NUMBER); return (unsigned)strtod(value, nullptr);}
	inline float    asFloat()  { S_GLTF_ASSERT(type == S_GLTF_JSON_TYPE_NUMBER); return (float)asDouble();}
	inline double   asDouble() { S_GLTF_ASSERT(type == S_GLTF_JSON_TYPE_NUMBER); return strtod(value, nullptr);}
	inline char*    asString() { S_GLTF_ASSERT(type == S_GLTF_JSON_TYPE_STRING); return value;}
	inline bool    	asBool()   { S_GLTF_ASSERT(type == S_GLTF_JSON_TYPE_BOOL);   return value[0] == 't';}
	
	// cast array values
	inline void asIntArray   (int*      out, int size) { S_GLTF_ASSERT(type == S_GLTF_JSON_TYPE_ARRAY); S_GLTF_ASSERT(out); if(size > childCount) size = childCount; for(int i = 0; i < size; i++) out[i] = children[i].asInt();}
	inline void asUIntArray  (unsigned* out, int size) { S_GLTF_ASSERT(type == S_GLTF_JSON_TYPE_ARRAY); S_GLTF_ASSERT(out); if(size > childCount) size = childCount; for(int i = 0; i < size; i++) out[i] = children[i].asUInt();}
	inline void asFloatArray (float*    out, int size) { S_GLTF_ASSERT(type == S_GLTF_JSON_TYPE_ARRAY); S_GLTF_ASSERT(out); if(size > childCount) size = childCount; for(int i = 0; i < size; i++) out[i] = children[i].asFloat();}
	inline void asDoubleArray(double*   out, int size) { S_GLTF_ASSERT(type == S_GLTF_JSON_TYPE_ARRAY); S_GLTF_ASSERT(out); if(size > childCount) size = childCount; for(int i = 0; i < size; i++) out[i] = children[i].asDouble();}
	inline void asBoolArray  (bool*     out, int size) { S_GLTF_ASSERT(type == S_GLTF_JSON_TYPE_ARRAY); S_GLTF_ASSERT(out); if(size > childCount) size = childCount; for(int i = 0; i < size; i++) out[i] = children[i].asBool();}
	inline void asStringArray(char**    out, int size) { S_GLTF_ASSERT(type == S_GLTF_JSON_TYPE_ARRAY); S_GLTF_ASSERT(out); if(size > childCount) size = childCount; for(int i = 0; i < size; i++) out[i] = children[i].asString();}

	// retrieve and cast values
	inline const char* getStringMember(const char* member, const char* defaultValue=0)     { auto m = getMember(member); return m==0       ? defaultValue : m->asString();}
	inline int         getIntMember   (const char* member, int         defaultValue=0)     { auto m = getMember(member); return m==nullptr ? defaultValue : m->asInt();}
	inline unsigned    getUIntMember  (const char* member, unsigned    defaultValue=0u)    { auto m = getMember(member); return m==nullptr ? defaultValue : m->asUInt();}
	inline float       getFloatMember (const char* member, float       defaultValue=0.0f)  { auto m = getMember(member); return m==nullptr ? defaultValue : m->asFloat();}
	inline double      getDoubleMember(const char* member, double      defaultValue=0.0)   { auto m = getMember(member); return m==nullptr ? defaultValue : m->asDouble();}
	inline bool        getBoolMember  (const char* member, bool        defaultValue=false) { auto m = getMember(member); return m==nullptr ? defaultValue : m->asBool();}

	// retrieve and cast array values
	inline void getIntArrayMember   (const char* member, int*          out, int size){ auto m = getMember(member); if(m) m->asIntArray(out, size);}
	inline void getUIntArrayMember  (const char* member, unsigned int* out, int size){ auto m = getMember(member); if(m) m->asUIntArray(out, size);}
	inline void getFloatArrayMember (const char* member, float*        out, int size){ auto m = getMember(member); if(m) m->asFloatArray(out, size);}
	inline void getDoubleArrayMember(const char* member, double*       out, int size){ auto m = getMember(member); if(m) m->asDoubleArray(out, size);}
	inline void getBoolArrayMember  (const char* member, bool*         out, int size){ auto m = getMember(member); if(m) m->asBoolArray(out, size);}
	inline void getStringArrayMember(const char* member, char**        out, int size){ auto m = getMember(member); if(m) m->asStringArray(out, size);}

	inline sGltfJsonObject& operator[](const char* member) { auto m = getMember(member); S_GLTF_ASSERT(m!=nullptr); return m==nullptr ? *this : *m;}
	inline sGltfJsonObject& operator[](int i)              { S_GLTF_ASSERT(children != nullptr); S_GLTF_ASSERT(i < childCount); return children[i]; };
};

// borrowed from Dear ImGui
template<typename T>
struct sGltfVector_
{
	int size     = 0u;
	int capacity = 0u;
	T*  data     = nullptr;
	inline sGltfVector_() { size = capacity = 0; data = nullptr; }
	inline sGltfVector_<T>& operator=(const sGltfVector_<T>& src) { clear(); resize(src.size); memcpy(data, src.data, (size_t)size * sizeof(T)); return *this; }
	inline bool empty() const { return size == 0; }
	inline int  size_in_bytes() const   { return size * (int)sizeof(T); }
	inline T&   operator[](int i) { S_GLTF_ASSERT(i >= 0 && i < size); return data[i]; }
	inline void clear() { if (data) { size = capacity = 0; S_GLTF_FREE(data); data = nullptr; } }
	inline T*   begin() { return data; }
    inline T*   end() { return data + size; }
	inline T&   back() { S_GLTF_ASSERT(size > 0); return data[size - 1]; }
	inline void swap(sGltfVector_<T>& rhs) { int rhs_size = rhs.size; rhs.size = size; size = rhs_size; int rhs_cap = rhs.capacity; rhs.capacity = capacity; capacity = rhs_cap; T* rhs_data = rhs.data; rhs.data = data; data = rhs_data; }
	inline int  _grow_capacity(int sz) { int new_capacity = capacity ? (capacity + capacity / 2) : 8; return new_capacity > sz ? new_capacity : sz; }
	inline void resize(int new_size) { if (new_size > capacity) reserve(_grow_capacity(new_size)); size = new_size; }
	inline void reserve(int new_capacity) { if (new_capacity <= capacity) return; T* new_data = (T*)S_GLTF_ALLOC((size_t)new_capacity * sizeof(T)); if (data) { memcpy(new_data, data, (size_t)size * sizeof(T)); S_GLTF_FREE(data); } data = new_data; capacity = new_capacity; }
	inline void push_back(const T& v) { if (size == capacity) reserve(_grow_capacity(size*2)); memcpy(&data[size], &v, sizeof(v)); size++;}
	inline void pop_back() { S_GLTF_ASSERT(size > 0); size--; }
};

typedef int sGltfToken_Type;

enum sGltfToken_Type_
{
	S_GLTF_JSON_TOKEN_NONE,
	S_GLTF_JSON_TOKEN_LEFT_BRACE,
	S_GLTF_JSON_TOKEN_RIGHT_BRACE,
	S_GLTF_JSON_TOKEN_LEFT_BRACKET,
	S_GLTF_JSON_TOKEN_RIGHT_BRACKET,
	S_GLTF_JSON_TOKEN_COMMA,
	S_GLTF_JSON_TOKEN_COLON,
	S_GLTF_JSON_TOKEN_STRING,
	S_GLTF_JSON_TOKEN_PRIMITIVE,
	S_GLTF_JSON_TOKEN_MEMBER, // string but on the left of ":"
};

struct sGltfToken_
{
	sGltfToken_Type type = S_GLTF_JSON_TOKEN_NONE;
	sGltfVector_<char> value;
};

struct sGltfStack_
{
	inline void push(int id){ if (data.empty()) data.resize(2048); if (currentIndex == data.size) data.resize(data.size * 2); data[currentIndex++] = id;}
	inline void pop()   { data[currentIndex] = -1; currentIndex--;}
	inline int  top()   { return data[currentIndex - 1];}
	inline bool empty() { return currentIndex == 0;}

	int currentIndex = 0;
	sGltfVector_<int> data;
};

static void
_parse_for_tokens(char* rawData, sGltfVector_<sGltfToken_>& tokens)
{
	int currentPos = 0u;
	char currentChar = rawData[currentPos];
	char basicTokens[] = { '{', '}', '[', ']', ':', ',' };

	bool inString = false;
	sGltfVector_<char> buffer;

	while (currentChar != 0)
	{
		bool tokenFound = false;
		if (!inString)
		{
			for (int i = 0; i < 6; i++)
			{
				if (currentChar == basicTokens[i])
				{
					if (!buffer.empty())
					{
						sGltfToken_ primitivetoken{};
						primitivetoken.type = S_GLTF_JSON_TOKEN_PRIMITIVE;
						buffer.push_back('\0');
						for (int i = 0; i < buffer.size; i++)
						{
							primitivetoken.value.push_back(buffer[i]);
						}
						tokens.push_back({ primitivetoken.type });
						primitivetoken.value.swap(tokens.back().value);
						buffer.clear();
					}

					sGltfToken_ token{};
					if      (currentChar == '{') token.type = S_GLTF_JSON_TOKEN_LEFT_BRACE;
					else if (currentChar == '}') token.type = S_GLTF_JSON_TOKEN_RIGHT_BRACE;
					else if (currentChar == '[') token.type = S_GLTF_JSON_TOKEN_LEFT_BRACKET;
					else if (currentChar == ']') token.type = S_GLTF_JSON_TOKEN_RIGHT_BRACKET;
					else if (currentChar == ',') token.type = S_GLTF_JSON_TOKEN_COMMA;
					else if (currentChar == ':') token.type = S_GLTF_JSON_TOKEN_COLON;

					token.value.push_back(currentChar);
					token.value.push_back(0);
					tokens.push_back({ token.type });
					token.value.swap(tokens.back().value);
					tokenFound = true;
					break;
				}
			}
		}

		// strings
		if (!tokenFound)
		{
			if (currentChar == '"')
			{
				if (inString)
				{
					sGltfToken_ token{};
					buffer.push_back('\0');
					for (int i = 0; i < buffer.size; i++)
					{
						token.value.push_back(buffer[i]);
					}


					if (rawData[currentPos + 1] == ':')
						token.type = S_GLTF_JSON_TOKEN_MEMBER;
					else
						token.type = S_GLTF_JSON_TOKEN_STRING;
					tokens.push_back({token.type});
					token.value.swap(tokens.back().value);
					tokenFound = true;
					inString = false;
					buffer.clear();
				}
				else
				{
					inString = true;
					buffer.clear();
					tokenFound = true;
				}
			}
			else if (inString)
			{
				tokenFound = true;
				buffer.push_back(currentChar);
			}
		}

		// primitives
		if (!tokenFound)
		{
			buffer.push_back(currentChar);
		}

		currentPos++;
		currentChar = rawData[currentPos];
	}

}

static void
_remove_whitespace(char* rawData, char* spacesRemoved, size_t size)
{
	size_t currentPos = 0;
	size_t newCursor = 0;
	bool insideString = false;
	char currentChar = rawData[currentPos];
	while (currentChar != 0)
	{
		if (currentChar == '"' && insideString)
			insideString = false;
		else if (currentChar == '"')
			insideString = true;

		if (currentChar == ' ' || currentChar == '\n'
			|| currentChar == '\r' || currentChar == '\t')
		{
			if (insideString)
			{
				spacesRemoved[newCursor] = rawData[currentPos];
				newCursor++;
			}
			currentPos++;
		}
		else
		{
			spacesRemoved[newCursor] = rawData[currentPos];
			currentPos++;
			newCursor++;
		}

		if (currentPos >= size || newCursor >= size)
		{
			spacesRemoved[newCursor] = 0;
			break;
		}
		currentChar = rawData[currentPos];
	}
}

static void
_update_children_pointers(sGltfJsonObject* object, sGltfVector_<sGltfJsonObject*>* objects)
{
	S_GLTF_ASSERT(object->_internal);

	if((*(sGltfVector_<int>*)(object->_internal)).empty())
	{
		S_GLTF_FREE(object->_internal);
		object->_internal = nullptr;
		return;
	}

	object->childCount = (*(sGltfVector_<int>*)(object->_internal)).size;
	object->children = (sGltfJsonObject*)S_GLTF_ALLOC(sizeof(sGltfJsonObject)*object->childCount);
	for(int i = 0; i < (*(sGltfVector_<int>*)(object->_internal)).size; i++)
	{
		object->children[i] = *(*objects)[(*(sGltfVector_<int>*)(object->_internal))[i]];
		_update_children_pointers(&object->children[i], objects);
	}

	S_GLTF_FREE(object->_internal);
	object->_internal = nullptr;
}

sGltfJsonObject*
Semper::load_json(char* rawData, int size)
{
	sGltfStack_ parentIDStack;
	sGltfVector_<sGltfJsonObject*> objectArray;

	char* spacesRemoved = (char*)S_GLTF_ALLOC(sizeof(char)*size);
	_remove_whitespace(rawData, spacesRemoved, size);

	sGltfVector_<sGltfToken_>* tokens =  (sGltfVector_<sGltfToken_>*)S_GLTF_ALLOC(sizeof(sGltfVector_<sGltfToken_>));
	new (tokens) sGltfVector_<sGltfToken_>();
	_parse_for_tokens(spacesRemoved, *tokens);

	sGltfJsonObject *rootObject = (sGltfJsonObject *)S_GLTF_ALLOC(sizeof(sGltfJsonObject));
	rootObject->type = S_GLTF_JSON_TYPE_OBJECT;
	rootObject->_internal = (sGltfVector_<int>*)S_GLTF_ALLOC(sizeof(sGltfVector_<int>));
	new (rootObject->_internal) sGltfVector_<int>();
	
	objectArray.push_back(rootObject);
	parentIDStack.push(0);

	int i = 0;
	bool waitingOnValue = true;
	while (true)
	{

		if (i >= tokens->size)
			break;

		sGltfJsonObject* parent = objectArray[parentIDStack.top()];

		switch ((*tokens)[i].type)
		{

		case S_GLTF_JSON_TOKEN_LEFT_BRACE:
		{

			if(waitingOnValue)
			{
				waitingOnValue = false; // object was created in S_GLTF_JSON_TOKEN_MEMBER case below.
			}
			else
			{
				sGltfJsonObject *newObject = (sGltfJsonObject*)S_GLTF_ALLOC(sizeof(sGltfJsonObject));
				newObject->type = S_GLTF_JSON_TYPE_OBJECT;
				newObject->_internal = (sGltfVector_<int>*)S_GLTF_ALLOC(sizeof(sGltfVector_<int>));
				new (newObject->_internal) sGltfVector_<int>();
				objectArray.push_back(newObject);
				parentIDStack.push(objectArray.size-1);
				(*(sGltfVector_<int>*)(parent->_internal)).push_back(objectArray.size-1);
			}
			i++;
			break;
		}

		case S_GLTF_JSON_TOKEN_LEFT_BRACKET:
		{

			if(waitingOnValue)
			{
				waitingOnValue = false; // object was created in S_GLTF_JSON_TOKEN_MEMBER case below.
			}
			else
			{
				sGltfJsonObject *newObject = (sGltfJsonObject*)S_GLTF_ALLOC(sizeof(sGltfJsonObject));
				newObject->type = S_GLTF_JSON_TYPE_ARRAY;
				newObject->_internal = (sGltfVector_<int>*)S_GLTF_ALLOC(sizeof(sGltfVector_<int>));
				new (newObject->_internal) sGltfVector_<int>();
				objectArray.push_back(newObject);
				parentIDStack.push(objectArray.size-1);
				(*(sGltfVector_<int>*)(parent->_internal)).push_back(objectArray.size-1);	
			}
			i++;
			break;
		}

		case S_GLTF_JSON_TOKEN_RIGHT_BRACE:
		{
			parentIDStack.pop();
			i++;
			break;
		}

		case S_GLTF_JSON_TOKEN_RIGHT_BRACKET:
		{
			parentIDStack.pop();
			i++;
			break;
		}

		case S_GLTF_JSON_TOKEN_MEMBER:
		{

			sGltfJsonObject* newObject = (sGltfJsonObject*)S_GLTF_ALLOC(sizeof(sGltfJsonObject));
			objectArray.push_back(newObject);
			newObject->_internal = (sGltfVector_<int>*)S_GLTF_ALLOC(sizeof(sGltfVector_<int>));
			new (newObject->_internal) sGltfVector_<int>();
			parentIDStack.push(objectArray.size-1);
			(*(sGltfVector_<int>*)(parent->_internal)).push_back(objectArray.size-1);
			memcpy(newObject->name, (*tokens)[i].value.data, (*tokens)[i].value.size_in_bytes());

			// look ahead to 2 tokens to look at type (skipping over ':' )
			sGltfToken_ valueToken = (*tokens)[i + 2];
			sGltfToken_Type valueType = valueToken.type;
			if      (valueType == S_GLTF_JSON_TOKEN_LEFT_BRACKET) newObject->type = S_GLTF_JSON_TYPE_ARRAY;
			else if (valueType == S_GLTF_JSON_TOKEN_LEFT_BRACE)   newObject->type = S_GLTF_JSON_TYPE_OBJECT;
			else if (valueType == S_GLTF_JSON_TOKEN_STRING)       newObject->type = S_GLTF_JSON_TYPE_STRING;
			else if (valueType == S_GLTF_JSON_TOKEN_PRIMITIVE)
			{
				if(valueToken.value[0] == 't')      newObject->type = S_GLTF_JSON_TYPE_BOOL;
				else if(valueToken.value[0] == 'f') newObject->type = S_GLTF_JSON_TYPE_BOOL;
				else if(valueToken.value[0] == 'n') newObject->type = S_GLTF_JSON_TYPE_NULL;
				else                                newObject->type = S_GLTF_JSON_TYPE_NUMBER;
			}
			i++;
			waitingOnValue = true;
			break;
		}

		case S_GLTF_JSON_TOKEN_STRING:
		{
			if(waitingOnValue)
			{
				parent->value = (*tokens)[i].value.data;
				waitingOnValue=false;
				parentIDStack.pop();
			}
			else // in array
			{
				sGltfJsonObject *newObject = (sGltfJsonObject*)S_GLTF_ALLOC(sizeof(sGltfJsonObject));
				newObject->type = S_GLTF_JSON_TYPE_STRING;
				newObject->_internal = (sGltfVector_<int>*)S_GLTF_ALLOC(sizeof(sGltfVector_<int>));
				new (newObject->_internal) sGltfVector_<int>();
				objectArray.push_back(newObject);
				newObject->value = (*tokens)[i].value.data;
				(*(sGltfVector_<int>*)(parent->_internal)).push_back(objectArray.size-1);	
			}
			i++;
			break;
		}

		case S_GLTF_JSON_TOKEN_PRIMITIVE:
		{
			if(waitingOnValue)
			{
				parent->value = (*tokens)[i].value.data;
				waitingOnValue=false;
				parentIDStack.pop();
			}
			else // in array
			{
				sGltfJsonObject *newObject = (sGltfJsonObject*)S_GLTF_ALLOC(sizeof(sGltfJsonObject));
				if((*tokens)[i].value.data[0] == 't')      newObject->type = S_GLTF_JSON_TYPE_BOOL;
				else if((*tokens)[i].value.data[0] == 'f') newObject->type = S_GLTF_JSON_TYPE_BOOL;
				else if((*tokens)[i].value.data[0] == 'n') newObject->type = S_GLTF_JSON_TYPE_NULL;
				else                                       newObject->type = S_GLTF_JSON_TYPE_NUMBER;
				newObject->_internal = (sGltfVector_<int>*)S_GLTF_ALLOC(sizeof(sGltfVector_<int>));
				new (newObject->_internal) sGltfVector_<int>();
				objectArray.push_back(newObject);
				newObject->value = (*tokens)[i].value.data;
				(*(sGltfVector_<int>*)(parent->_internal)).push_back(objectArray.size-1);	
			}
			i++;
			break;
		}

		default:
			i++;
			break;
		}

	}
	parentIDStack.pop();
	_update_children_pointers(rootObject, &objectArray);
	objectArray.clear();
	parentIDStack.data.clear();
	return rootObject;
}

static void
_free_json(sGltfJsonObject* object)
{
	for(int i = 0; i < object->childCount; i++)
	{
		_free_json(&object->children[i]);
	}
	if(object->childCount > 0)
		S_GLTF_FREE(object->children);
}

void
Semper::free_json(sGltfJsonObject** rootObjectPtr)
{
	sGltfJsonObject* rootObject = *rootObjectPtr;
	_free_json(rootObject);
	S_GLTF_FREE(rootObject);
	rootObjectPtr = nullptr;
}

static bool
_is_data_uri(const std::string& in)
{
	std::string header = "data:application/octet-stream;base64,";
	if (in.find(header) == 0)
		return true;

	header = "data:image/jpeg;base64,";
	if (in.find(header) == 0)
		return true;

	header = "data:image/png;base64,";
	if (in.find(header) == 0)
		return true;

	header = "data:image/bmp;base64,";
	if (in.find(header) == 0)
		return true;

	header = "data:image/gif;base64,";
	if (in.find(header) == 0)
		return true;

	header = "data:text/plain;base64,";
	if (in.find(header) == 0)
		return true;

	header = "data:application/gltf-buffer;base64,";
	if (in.find(header) == 0)
		return true;

	return false;
}

static inline bool
_is_base64(unsigned char c)
{
	return (isalnum(c) || (c == '+') || (c == '/'));
}

static std::string
_base64_decode(std::string const& encoded_string)
{
	int in_len = static_cast<int>(encoded_string.size());
	int i = 0;
	int j = 0;
	int in_ = 0;
	unsigned char char_array_4[4], char_array_3[3];
	std::string ret;

	const std::string base64_chars =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz"
		"0123456789+/";

	while (in_len-- && (encoded_string[in_] != '=') &&
		_is_base64(encoded_string[in_])) {
		char_array_4[i++] = encoded_string[in_];
		in_++;
		if (i == 4) {
			for (i = 0; i < 4; i++)
				char_array_4[i] =
				static_cast<unsigned char>(base64_chars.find(char_array_4[i]));

			char_array_3[0] =
				(char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
			char_array_3[1] =
				((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
			char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

			for (i = 0; (i < 3); i++) ret += char_array_3[i];
			i = 0;
		}
	}

	if (i)
	{
		for (j = i; j < 4; j++) char_array_4[j] = 0;

		for (j = 0; j < 4; j++)
			char_array_4[j] =
			static_cast<unsigned char>(base64_chars.find(char_array_4[j]));

		char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
		char_array_3[1] =
			((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
		char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

		for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
	}

	return ret;
}

static bool
_decode_data_uri(unsigned char** out, std::string& mime_type, const std::string& in, size_t reqBytes, bool checkSize, size_t* finalSize)
{
	std::string header = "data:application/octet-stream;base64,";
	std::string data;
	if (in.find(header) == 0) {
		data = _base64_decode(in.substr(header.size()));  // cut mime string.
	}

	if (data.empty()) {
		header = "data:image/jpeg;base64,";
		if (in.find(header) == 0) {
			mime_type = "image/jpeg";
			data = _base64_decode(in.substr(header.size()));  // cut mime string.
		}
	}

	if (data.empty()) {
		header = "data:image/png;base64,";
		if (in.find(header) == 0) {
			mime_type = "image/png";
			data = _base64_decode(in.substr(header.size()));  // cut mime string.
		}
	}

	if (data.empty()) {
		header = "data:image/bmp;base64,";
		if (in.find(header) == 0) {
			mime_type = "image/bmp";
			data = _base64_decode(in.substr(header.size()));  // cut mime string.
		}
	}

	if (data.empty()) {
		header = "data:image/gif;base64,";
		if (in.find(header) == 0) {
			mime_type = "image/gif";
			data = _base64_decode(in.substr(header.size()));  // cut mime string.
		}
	}

	if (data.empty()) {
		header = "data:text/plain;base64,";
		if (in.find(header) == 0) {
			mime_type = "text/plain";
			data = _base64_decode(in.substr(header.size()));
		}
	}

	if (data.empty()) {
		header = "data:application/gltf-buffer;base64,";
		if (in.find(header) == 0) {
			data = _base64_decode(in.substr(header.size()));
		}
	}

	// TODO(syoyo): Allow empty buffer? #229
	if (data.empty()) 
	{
		return false;
	}

	if(checkSize)
	{
		if(data.size() != reqBytes)
			return false;
		*out = new unsigned char[reqBytes];
		*finalSize = reqBytes;
	}
	else
	{
		*out = new unsigned char[data.size()];
		*finalSize = data.size();
	}

	memcpy(*out, data.data(), data.size());	
	return true;
}

static char**
_LoadExtensions(sGltfJsonObject& j, unsigned& size)
{
	if (!j.doesMemberExist("extensionsUsed"))
		return nullptr;

	unsigned extensionCount = j["extensionsUsed"].childCount;

	char** extensions = new char*[extensionCount];

	for (int i = 0; i < extensionCount; i++)
	{
		extensions[i] = new char[S_GLTF_MAX_NAME_LENGTH];
		sGltfJsonObject& jExtension = j["extensionsUsed"][i];
		strncpy(extensions[i], jExtension.value, S_GLTF_MAX_NAME_LENGTH);
		size++;
	}

	return extensions;
}

static sGLTFAnimation*
_LoadAnimations(sGltfJsonObject& j, unsigned& size)
{
	sGltfJsonObject* janimations = j.getMember("animations");
	if(janimations == nullptr)
		return nullptr;
	size = janimations->childCount;

	sGLTFAnimation* animations = new sGLTFAnimation[size];

	for (int i = 0; i < size; i++)
	{
		sGltfJsonObject& janimation = (*janimations)[i];
		sGLTFAnimation& animation = animations[i];
		animation.channel_count = 0u;
		animation.channels = nullptr;
		animation.sampler_count = 0u;
		animation.samplers = nullptr;
		strncpy(animation.name, janimation.getStringMember("name", ""), S_GLTF_MAX_NAME_LENGTH);

		if (sGltfJsonObject* jsamplers = janimation.getMember("samplers"))
		{
			animation.sampler_count = jsamplers->childCount;
			animation.samplers = new sGLTFAnimationSampler[animation.sampler_count];

			for (int s = 0; s < animation.sampler_count; s++)
			{
				sGltfJsonObject& jsampler = (*jsamplers)[s];
				sGLTFAnimationSampler& sampler = animation.samplers[s];
				sampler.input = jsampler.getIntMember("input", -1);
				sampler.output = jsampler.getIntMember("output", -1);
				strncpy(sampler.interpolation, jsampler.getStringMember("interpolation", "LINEAR"), S_GLTF_MAX_NAME_LENGTH);
			}
		}

		if (sGltfJsonObject* jchannels = janimation.getMember("channels"))
		{
			animation.channel_count = jchannels->childCount;
			animation.channels = new sGLTFAnimationChannel[animation.channel_count];

			for (int i = 0; i < animation.channel_count; i++)
			{
				sGltfJsonObject& jchannel = (*jchannels)[i];
				sGLTFAnimationChannel& channel = animation.channels[i];

				channel.sampler = jchannel.getIntMember("sampler", -1);

				if (sGltfJsonObject* jtarget = jchannel.getMember("target"))
				{
					channel.target.node = jtarget->getIntMember("node", -1);
					strncpy(channel.target.path, jtarget->getStringMember("path", ""), S_GLTF_MAX_NAME_LENGTH);
				}
			}
		}
	}
	return animations;
}

static sGLTFCamera*
_LoadCameras(sGltfJsonObject& j, unsigned& size)
{
	sGltfJsonObject* jcameras = j.getMember("cameras");
	if(jcameras == nullptr)
		return nullptr;
	size = jcameras->childCount;

	sGLTFCamera* cameras = new sGLTFCamera[size];

	for (int i = 0; i < size; i++)
	{
		sGltfJsonObject& jcamera = (*jcameras)[i];
		sGLTFCamera& camera = cameras[i];
		strncpy(camera.name, jcamera.getStringMember("name", ""), S_GLTF_MAX_NAME_LENGTH);
		std::string type = jcamera.getStringMember("type", "perspective");

		if (type == "perspective") camera.type = S_GLTF_PERSPECTIVE;
		else                       camera.type = S_GLTF_ORTHOGRAPHIC;

		if (sGltfJsonObject* jperspective = jcamera.getMember("perspective"))
		{
			camera.perspective.aspectRatio = jperspective->getFloatMember("aspectRatio", 0.0f);
			camera.perspective.yfov = jperspective->getFloatMember("yfov", 0.0f);
			camera.perspective.zfar = jperspective->getFloatMember("zfar", 0.0f);
			camera.perspective.znear = jperspective->getFloatMember("znear", 0.0f);
		}

		if (sGltfJsonObject* jorthographic = jcamera.getMember("orthographic"))
		{
			camera.orthographic.xmag = jorthographic->getFloatMember("xmag", 0.0f);
			camera.orthographic.ymag = jorthographic->getFloatMember("ymag", 0.0f);
			camera.orthographic.zfar = jorthographic->getFloatMember("zfar", 0.0f);
			camera.orthographic.znear = jorthographic->getFloatMember("znear", 0.0f);
		}
	}
	return cameras;
}

static sGLTFScene*
_LoadScenes(sGltfJsonObject& j, unsigned& size)
{
	sGltfJsonObject* jscenes = j.getMember("scenes");
	if(jscenes == nullptr)
		return nullptr;
	size = jscenes->childCount;
	sGLTFScene* scenes = new sGLTFScene[size];
	for (int i = 0; i < size; i++)
	{
		sGltfJsonObject& jscene = (*jscenes)[0];
		sGLTFScene& scene = scenes[i];
		scene.node_count = 0u;
		scene.nodes = nullptr;
		if (sGltfJsonObject* jnodes = jscene.getMember("nodes"))
		{
			scene.node_count = jnodes->childCount;
			scene.nodes = new unsigned[scene.node_count];
			for (int j = 0; j < scene.node_count; j++)
			{
				int node = (*jnodes)[j].asInt();
				scene.nodes[j] = node;
			}
		}
	}
	return scenes;
}

static sGLTFNode*
_LoadNodes(sGltfJsonObject& j, unsigned& size)
{
	sGltfJsonObject* jnodes = j.getMember("nodes");
	if(jnodes == nullptr)
		return nullptr;
	size = jnodes->childCount;
	sGLTFNode* nodes = new sGLTFNode[size];

	for (int i = 0; i < size; i++)
	{
		sGltfJsonObject& jnode = (*jnodes)[i];
		sGLTFNode& node = nodes[i];
		node.child_count = 0u;
		node.children = nullptr;
		for(int i = 0; i < 16; i++)
			node.matrix[i] = 0.0f;
		node.matrix[0] = 1.0f;
		node.matrix[5] = 1.0f;
		node.matrix[10] = 1.0f;
		node.matrix[15] = 1.0f;
		node.rotation[0] = 0.0f;
		node.rotation[1] = 0.0f;
		node.rotation[2] = 0.0f;
		node.rotation[3] = 1.0f;
		node.scale[0] = 1.0f;
		node.scale[1] = 1.0f;
		node.scale[2] = 1.0f;
		node.translation[0] = 0.0f;
		node.translation[1] = 0.0f;
		node.translation[2] = 0.0f;
		strncpy(node.name, jnode.getStringMember("name", ""), S_GLTF_MAX_NAME_LENGTH);
		node.mesh_index = jnode.getIntMember("mesh", -1);
		node.camera_index = jnode.getIntMember("camera", -1);
		node.skin_index = jnode.getIntMember("skin", -1);
		jnode.getFloatArrayMember("translation", node.translation, 3);
		jnode.getFloatArrayMember("scale", node.scale, 3);
		jnode.getFloatArrayMember("rotation", node.rotation, 4);
		jnode.getFloatArrayMember("matrix", node.matrix, 16);
		node.hadMatrix = jnode.doesMemberExist("matrix");

		if (sGltfJsonObject* jchildren = jnode.getMember("children"))
		{
			node.child_count = jchildren->childCount;
			node.children = new unsigned[node.child_count];
			for (int j = 0; j < node.child_count; j++)
			{
				unsigned child = (*jchildren)[j].asFloat();
				node.children[j] = child;
			}
		}
	}
	return nodes;
}

static sGLTFMesh*
_LoadMeshes(sGltfJsonObject& j, unsigned& size)
{
	sGltfJsonObject* jmeshes = j.getMember("meshes");
	if(jmeshes == nullptr)
		return nullptr;
	size = jmeshes->childCount;
	sGLTFMesh* meshes = new sGLTFMesh[size];

	for (int i = 0; i < size; i++)
	{
		sGltfJsonObject& jmesh = (*jmeshes)[i];
		sGLTFMesh& mesh = meshes[i];
		mesh.primitives = nullptr;
		mesh.primitives_count = 0u;
		mesh.weights = nullptr;
		mesh.weights_count = 0u;
		strncpy(mesh.name, jmesh.getStringMember("name", ""), S_GLTF_MAX_NAME_LENGTH);

		if (sGltfJsonObject* jweights = jmesh.getMember("weights"))
		{
			mesh.weights_count = (*jweights).childCount;
			mesh.weights = new float[mesh.weights_count];
			for (int j = 0; j < mesh.weights_count; j++)
			{
				sGltfJsonObject m = (*jweights)[j];
				mesh.weights[j] = m.asFloat();
			}
		}

		if (sGltfJsonObject* jprimitives = jmesh.getMember("primitives"))
		{
			mesh.primitives_count = jprimitives->childCount;
			mesh.primitives = new sGLTFMeshPrimitive[mesh.primitives_count];

			for (int j = 0; j < mesh.primitives_count; j++)
			{
				sGLTFMeshPrimitive& primitive = mesh.primitives[j];
				sGltfJsonObject jprimitive = (*jprimitives)[j];
				primitive.mode = S_GLTF_TRIANGLES;
				primitive.attributes = nullptr;
				primitive.attribute_count = 0u;
				primitive.targets = nullptr;
				primitive.target_count = 0u;
				primitive.indices_index = jprimitive.getIntMember("indices", -1);
				primitive.material_index = jprimitive.getIntMember("material", -1);

				if (sGltfJsonObject* jattributes = jprimitive.getMember("attributes"))
				{
					primitive.attribute_count = jattributes->childCount;
					primitive.attributes = new sGLTFAttribute[primitive.attribute_count];
					
					for (int k = 0; k < primitive.attribute_count; k++)
					{
						sGltfJsonObject& m = (*jattributes)[k];
						memcpy(primitive.attributes[k].semantic, m.name, S_GLTF_MAX_NAME_LENGTH);
						primitive.attributes[k].index = m.asInt();
					}
				}

				if (sGltfJsonObject* jtargets = jprimitive.getMember("targets"))
				{
					primitive.target_count = jtargets->childCount;
					primitive.targets = new sGLTFMorphTarget[primitive.target_count];

					for (int k = 0; k < primitive.target_count; k++)
					{
						sGLTFMorphTarget& target = primitive.targets[k];
						sGltfJsonObject& jtarget = (*jtargets)[k];
						
						target.attribute_count = jtarget.childCount;
						target.attributes = new sGLTFAttribute[target.attribute_count];
						for (int x = 0; x < target.attribute_count; x++)
						{
							sGltfJsonObject& jattribute = jtarget[x];
							memcpy(target.attributes[x].semantic, jattribute.name, S_GLTF_MAX_NAME_LENGTH);
							target.attributes[x].index = jattribute.asInt();
						}
					}
				}
			}
		}
	}
	return meshes;
}

static sGLTFMaterial*
_LoadMaterials(sGltfJsonObject& j, unsigned& size)
{
	sGltfJsonObject* jmaterials = j.getMember("materials");
	if(jmaterials == nullptr)
		return nullptr;
	size = jmaterials->childCount;
	sGLTFMaterial* materials = new sGLTFMaterial[size];

	for (int i = 0; i < size; i++)
	{
		sGltfJsonObject& jmaterial = (*jmaterials)[i];
		sGLTFMaterial& material = materials[i];
		strncpy(material.name, jmaterial.getStringMember("name", ""), S_GLTF_MAX_NAME_LENGTH);
		material.double_sided = jmaterial.getBoolMember("doubleSided", false);
		material.alphaCutoff = jmaterial.getFloatMember("alphaCutoff", 0.5f);
		material.pbrMetallicRoughness = false;
		material.clearcoat_extension = false;
		material.metallic_factor = jmaterial["pbrMetallicRoughness"].getFloatMember("metallicFactor", 1.0f);
		material.roughness_factor = jmaterial["pbrMetallicRoughness"].getFloatMember("roughnessFactor", 1.0f);
		material.normal_texture_scale = 1.0f;
		material.clearcoat_normal_texture_scale = 1.0f;
		material.occlusion_texture_strength = 1.0f;
		material.metallic_factor = 1.0f;
		material.roughness_factor = 1.0f;
		material.base_color_factor[0] = 1.0f;
		material.base_color_factor[1] = 1.0f;
		material.base_color_factor[2] = 1.0f;
		material.base_color_factor[3] = 1.0f;
		material.emissive_factor[0] = 0.0f;
		material.emissive_factor[1] = 0.0f;
		material.emissive_factor[2] = 0.0f;
		material.alphaCutoff = 0.5f;
		material.double_sided = false;
		material.clearcoat_factor = 0.0f;
		material.clearcoat_roughness_factor = 0.0f;
		material.clearcoat_texture = -1;
		material.clearcoat_roughness_texture = -1;
		material.clearcoat_normal_texture = -1;
		material.base_color_texture = -1;
		material.metallic_roughness_texture = -1;
		material.normal_texture = -1;
		material.occlusion_texture = -1;
		material.emissive_texture = -1;

		jmaterial.getFloatArrayMember("emissiveFactor", material.emissive_factor, 3);	

		std::string alphaMode = jmaterial.getStringMember("alphaMode", "OPAQUE");
		if     (alphaMode == "OPAQUE") material.alphaMode = S_ALPHA_MODE_OPAQUE;
		else if(alphaMode == "MASK")   material.alphaMode = S_ALPHA_MODE_MASK;
		else                           material.alphaMode = S_ALPHA_MODE_BLEND;

		if (sGltfJsonObject* jrt = jmaterial.getMember("normalTexture"))
		{
			material.normal_texture = jrt->getIntMember("index", -1);
			material.normal_texture_scale = jrt->getFloatMember("scale", 1.0f);
		}

		if (sGltfJsonObject* jot = jmaterial.getMember("occlusionTexture"))
		{
			material.occlusion_texture = jot->getIntMember("index", -1);
			material.occlusion_texture_strength = jot->getFloatMember("strength", 1.0f);
		}

		if (sGltfJsonObject* jet = jmaterial.getMember("emissiveTexture"))
		{
			material.emissive_texture = jet->getIntMember("index", -1);
		}

		if (sGltfJsonObject* jpbrmr = jmaterial.getMember("pbrMetallicRoughness"))
		{
			material.pbrMetallicRoughness = true;
			material.metallic_factor = jpbrmr->getFloatMember("metallicFactor", 1.0f);
			material.roughness_factor = jpbrmr->getFloatMember("roughnessFactor", 1.0f);
			jpbrmr->getFloatArrayMember("baseColorFactor", material.base_color_factor, 4);

			if (sGltfJsonObject* jbaseColorTexture = jpbrmr->getMember("baseColorTexture"))
			{
				material.base_color_texture = jbaseColorTexture->getIntMember("index", -1);
			}
			
			if (sGltfJsonObject* jmrt = jpbrmr->getMember("metallicRoughnessTexture"))
			{
				material.metallic_roughness_texture = jmrt->getIntMember("index", -1);
			}

		}

		if (sGltfJsonObject* jextensions = jmaterial.getMember("extensions"))
		{
			if (sGltfJsonObject* jKHR_cc = jextensions->getMember("KHR_materials_clearcoat"))
			{
				material.clearcoat_extension = true;
				material.clearcoat_factor = jKHR_cc->getFloatMember("clearcoatFactor", 0.0f);
				material.clearcoat_roughness_factor = jKHR_cc->getIntMember("clearcoatRoughnessFactor", 0.0f);

				if (sGltfJsonObject* jcct = jKHR_cc->getMember("clearcoatTexture"))
				{
					material.clearcoat_texture = jcct->getIntMember("index", -1);
					material.clearcoat_normal_texture_scale = jcct->getFloatMember("scale", 1.0f);
				}
				if (sGltfJsonObject* jccrt = jKHR_cc->getMember("clearcoatRoughnessTexture"))
				{
					material.clearcoat_roughness_texture = jccrt->getIntMember("index", -1);
				}
				if (sGltfJsonObject* jccnt = jKHR_cc->getMember("clearcoatNormalTexture"))
				{
					material.clearcoat_normal_texture = jccnt->getIntMember("index", -1);
				}
			}
		}
	}
	return materials;
}

static sGLTFTexture*
_LoadTextures(sGltfJsonObject& j, unsigned& size)
{
	sGltfJsonObject* jtextures = j.getMember("textures");
	if(jtextures == nullptr)
		return nullptr;
	size = jtextures->childCount;
	sGLTFTexture* textures = new sGLTFTexture[size];

	for (int i = 0; i < size; i++)
	{
		sGltfJsonObject& jtexture = (*jtextures)[i];
		sGLTFTexture& texture = textures[i];
		texture.sampler_index = jtexture.getIntMember("sampler", -1);
		texture.image_index = jtexture.getIntMember("source", -1);
		strncpy(texture.name, jtexture.getStringMember("name", ""), S_GLTF_MAX_NAME_LENGTH);
	}
	return textures;
}

static sGLTFSampler*
_LoadSamplers(sGltfJsonObject& j, unsigned& size)
{
	sGltfJsonObject* jsamplers = j.getMember("samplers");
	if(jsamplers == nullptr)
		return nullptr;
	size = jsamplers->childCount;
	sGLTFSampler* samplers = new sGLTFSampler[size];

	for (int i = 0; i < size; i++)
	{
		sGltfJsonObject& jsampler = (*jsamplers)[i];
		sGLTFSampler& sampler = samplers[i];
		strncpy(sampler.name, jsampler.getStringMember("name", ""), S_GLTF_MAX_NAME_LENGTH);
		sampler.mag_filter = jsampler.getIntMember("magFilter", -1);
		sampler.min_filter = jsampler.getIntMember("minFilter", -1);
		sampler.wrap_s = jsampler.getIntMember("wrapS", S_GLTF_WRAP_REPEAT);
		sampler.wrap_t = jsampler.getIntMember("wrapT", S_GLTF_WRAP_REPEAT);
	}
	return samplers;
}

static sGLTFImage*
_LoadImages(sGltfJsonObject& j, unsigned& size)
{
	sGltfJsonObject* jimages = j.getMember("images");
	if(jimages == nullptr)
		return nullptr;
	size = jimages->childCount;
	sGLTFImage* images = new sGLTFImage[size];

	for (int i = 0; i < size; i++)
	{
		sGltfJsonObject& jimage = (*jimages)[i];
		sGLTFImage& image = images[i];
		image.uri = jimage.getStringMember("uri", "");
		image.mimeType = jimage.getStringMember("mimeType", "");
		image.buffer_view_index = jimage.getIntMember("bufferView", -1);
		image.data = nullptr;
		image.dataCount = 0u;
		image.embedded = false; // ?
	}
	return images;
}

static sGLTFBuffer*
_LoadBuffers(sGltfJsonObject& j, unsigned& size)
{
	sGltfJsonObject* jbuffers = j.getMember("buffers");
	if(jbuffers == nullptr)
		return nullptr;
	size = jbuffers->childCount;
	sGLTFBuffer* buffers = new sGLTFBuffer[size];
	for (int i = 0; i < size; i++)
	{
		sGltfJsonObject& jbuffer = (*jbuffers)[i];
		sGLTFBuffer& buffer = buffers[i];
		buffer.uri = jbuffer.getStringMember("uri", "");
		buffer.byte_length = jbuffer.getUIntMember("byteLength", 0u);	
		buffer.dataCount = 0u;
		buffer.data = nullptr;
	}
	return buffers;
}

static sGLTFBufferView*
_LoadBufferViews(sGltfJsonObject& j, unsigned& size)
{
	sGltfJsonObject* jbufferViews = j.getMember("bufferViews");
	if(jbufferViews == nullptr)
		return nullptr;
	size = jbufferViews->childCount;
	sGLTFBufferView* bufferviews = new sGLTFBufferView[size];

	for (int i = 0; i < size; i++)
	{
		sGltfJsonObject& jbufferview = (*jbufferViews)[i];
		sGLTFBufferView& bufferview = bufferviews[i];
		strncpy(bufferview.name, jbufferview.getStringMember("name", ""), S_GLTF_MAX_NAME_LENGTH);
		bufferview.buffer_index = jbufferview.getIntMember("buffer", -1);
		bufferview.byte_offset = jbufferview.getIntMember("byteOffset", 0);
		bufferview.byte_length = jbufferview.getIntMember("byteLength", -1);
		bufferview.byte_stride = jbufferview.getIntMember("byteStride", -1);
	}
	return bufferviews;
}

static sGLTFAccessor*
_LoadAccessors(sGltfJsonObject& j, unsigned& size)
{
	sGltfJsonObject* jaccessors = j.getMember("accessors");
	if(jaccessors == nullptr)
		return nullptr;
	size = jaccessors->childCount;
	sGLTFAccessor* accessors = new sGLTFAccessor[size];

	for (int i = 0; i < size; i++)
	{
		sGltfJsonObject& jaccessor = (*jaccessors)[i];
		sGLTFAccessor& accessor = accessors[i];
		strncpy(accessor.name, jaccessor.getStringMember("name", ""), S_GLTF_MAX_NAME_LENGTH);
		accessor.byteOffset = jaccessor.getIntMember("byteOffset", 0);
		accessor.count = jaccessor.getIntMember("count", -1);
		accessor.component_type = (sGLTFComponentType)jaccessor.getIntMember("componentType", S_GLTF_FLOAT);
		accessor.buffer_view_index = jaccessor.getIntMember("bufferView", -1);
		accessor.type = S_GLTF_SCALAR;
		jaccessor.getFloatArrayMember("max", accessor.maxes, 16);
		jaccessor.getFloatArrayMember("min", accessor.mins, 16);

		std::string accessorType = jaccessor.getStringMember("type", "SCALAR");
		if (accessorType == "SCALAR")    accessor.type = S_GLTF_SCALAR;
		else if (accessorType == "VEC2") accessor.type = S_GLTF_VEC2;
		else if (accessorType == "VEC3") accessor.type = S_GLTF_VEC3;
		else if (accessorType == "VEC4") accessor.type = S_GLTF_VEC4;
		else if (accessorType == "MAT2") accessor.type = S_GLTF_MAT2;
		else if (accessorType == "MAT3") accessor.type = S_GLTF_MAT3;
		else if (accessorType == "MAT4") accessor.type = S_GLTF_MAT4;
	}
	return accessors;
}

static sGLTFSkin*
_LoadSkins(sGltfJsonObject& j, unsigned& size)
{
	sGltfJsonObject* jskins = j.getMember("skins");
	if(jskins == nullptr)
		return nullptr;
	size = jskins->childCount;
	sGLTFSkin* skins = new sGLTFSkin[size];

	for (int i = 0; i < size; i++)
	{
		sGltfJsonObject& jnode = (*jskins)[i];
		sGLTFSkin& skin = skins[i];
		strncpy(skin.name, jnode.getStringMember("name", ""), S_GLTF_MAX_NAME_LENGTH);
		skin.inverseBindMatrices = jnode.getIntMember("inverseBindMatrices", -1);
		skin.skeleton = jnode.getIntMember("skeleton", -1);
		skin.joints = nullptr;
		skin.joints_count = 0u;

		if (sGltfJsonObject* jjoints = jnode.getMember("joints"))
		{
			skin.joints_count = jjoints->childCount;
			skin.joints = new unsigned[skin.joints_count];
			jjoints->asUIntArray(skin.joints, skin.joints_count);
		}
	}
	return skins;
}

static char*
_ReadFile(const char* file, unsigned& size, const char* mode)
{
	FILE* dataFile = fopen(file, mode);

	if (dataFile == nullptr)
	{
		assert(false && "File not found.");
		return nullptr;
	}

	else
	{
		// obtain file size:
		fseek(dataFile, 0, SEEK_END);
		size = ftell(dataFile);
		fseek(dataFile, 0, SEEK_SET);

		// allocate memory to contain the whole file:
		char* data = new char[size];

		// copy the file into the buffer:
		size_t result = fread(data, sizeof(char), size, dataFile);
		if (result != size)
		{
			if (feof(dataFile))
				printf("Error reading test.bin: unexpected end of file\n");
			else if (ferror(dataFile)) {
				perror("Error reading test.bin");
			}
			assert(false && "File not read.");
		}

		fclose(dataFile);

		return data;
	}
}

static sGLTFModel
sLoadBinaryGLTF(const char* root, const char* file)
{

	sGLTFModel model{};
	model.root = root;
	model.name = file;

	unsigned dataSize = 0u;
	char* data = (char*)_ReadFile(file, dataSize, "rb");

	unsigned magic = *(unsigned*)&data[0];
	unsigned version = *(unsigned*)&data[4];
	unsigned length = *(unsigned*)&data[8];

	unsigned chunkLength = *(unsigned*)&data[12];
	unsigned chunkType = *(unsigned*)&data[16];
	char* chunkData = &data[20];

	if (dataSize == 0u)
	{
		assert(false && "File not found.");
		return model;
	}

	sGltfJsonObject* rootObject = Semper::load_json(chunkData, chunkLength);
	model.scene = rootObject->getIntMember("scene", 0);
	model.scenes = _LoadScenes(*rootObject, model.scene_count);
	model.nodes = _LoadNodes(*rootObject, model.node_count);
	model.materials = _LoadMaterials(*rootObject, model.material_count);
	model.meshes = _LoadMeshes(*rootObject, model.mesh_count);
	model.textures = _LoadTextures(*rootObject, model.texture_count);
	model.samplers = _LoadSamplers(*rootObject, model.sampler_count);
	model.images = _LoadImages(*rootObject, model.image_count);
	model.buffers = _LoadBuffers(*rootObject, model.buffer_count);
	model.bufferviews = _LoadBufferViews(*rootObject, model.bufferview_count);
	model.accessors = _LoadAccessors(*rootObject, model.accessor_count);
	model.cameras = _LoadCameras(*rootObject, model.camera_count);
	model.animations = _LoadAnimations(*rootObject, model.animation_count);
	model.extensions = _LoadExtensions(*rootObject, model.extension_count);
	model.skins = _LoadSkins(*rootObject, model.skin_count);

	if (chunkLength + 20 != length)
	{
		unsigned datachunkLength = *(unsigned*)&data[20 + chunkLength];
		unsigned datachunkType = *(unsigned*)&data[24 + chunkLength];
		char* datachunkData = &data[28 + chunkLength];

		model.buffers[0].dataCount = model.buffers[0].byte_length;
		model.buffers[0].data = new unsigned char[model.buffers[0].dataCount];
		memcpy(model.buffers[0].data, datachunkData, model.buffers[0].byte_length);
		int a = 6;
	}

	for (unsigned i = 0; i < model.image_count; i++)
	{
		sGLTFImage& image = model.images[i];

		if (image.buffer_view_index > -1)
		{
			image.embedded = true;
			//image.data = model.buffers
			sGLTFBufferView bufferView = model.bufferviews[image.buffer_view_index];
			char* bufferRawData = (char*)model.buffers[bufferView.buffer_index].data;
			char* bufferRawSection = &bufferRawData[bufferView.byte_offset]; // start of buffer section
			image.dataCount = bufferView.byte_length;
			image.data = new unsigned char[image.dataCount];
			memcpy(image.data, bufferRawSection, bufferView.byte_length);
			continue;
		}

		if (_is_data_uri(image.uri))
		{
			image.embedded = true;
			std::string mime_type;
			if (!_decode_data_uri(&image.data, mime_type, image.uri, 0, false, &image.dataCount))
			{
				assert(false && "here");
			}
		}
		else
		{
			image.embedded = false;
		}

	}

	for (unsigned i = 0; i < model.buffer_count; i++)
	{
		sGLTFBuffer& buffer = model.buffers[i];

		if (buffer.data)
			continue;

		if (_is_data_uri(buffer.uri))
		{
			std::string mime_type;
			if (!_decode_data_uri(&buffer.data, mime_type, buffer.uri, buffer.byte_length, true, &buffer.dataCount))
			{
				assert(false && "here");
			}
		}
		else
		{
			std::string combinedFile = model.root;
			combinedFile.append(buffer.uri);
			unsigned dataSize = 0u;
			void* bufferdata = _ReadFile(combinedFile.c_str(), dataSize, "rb");
			buffer.dataCount = dataSize;
			buffer.data = new unsigned char[buffer.dataCount];
			memcpy(buffer.data, bufferdata, dataSize);
			delete[] bufferdata;
		}

	}

	Semper::free_json(&rootObject);
	delete[] data;
	return model;
}

sGLTFModel
Semper::load_gltf(const char* root, const char* file)
{

	size_t len = strlen(file);
	if (file[len - 1] == 'b')
		return sLoadBinaryGLTF(root, file);

	sGLTFModel model{};
	model.root = root;
	model.name = file;

	unsigned dataSize = 0u;
	char* data = _ReadFile(file, dataSize, "rb");

	if (dataSize == 0u)
	{
		assert(false && "File not found.");
		return model;
	}

	sGltfJsonObject* rootObject = load_json(data, dataSize);
	delete[] data;
	model.scene = rootObject->getIntMember("scene", 0);
	model.scenes = _LoadScenes(*rootObject, model.scene_count);
	model.nodes = _LoadNodes(*rootObject, model.node_count);
	model.materials = _LoadMaterials(*rootObject, model.material_count);
	model.meshes = _LoadMeshes(*rootObject, model.mesh_count);
	model.textures = _LoadTextures(*rootObject, model.texture_count);
	model.samplers = _LoadSamplers(*rootObject, model.sampler_count);
	model.images = _LoadImages(*rootObject, model.image_count);
	model.buffers = _LoadBuffers(*rootObject, model.buffer_count);
	model.bufferviews = _LoadBufferViews(*rootObject, model.bufferview_count);
	model.accessors = _LoadAccessors(*rootObject, model.accessor_count);
	model.cameras = _LoadCameras(*rootObject, model.camera_count);
	model.animations = _LoadAnimations(*rootObject, model.animation_count);
	model.extensions = _LoadExtensions(*rootObject, model.extension_count);
	model.skins = _LoadSkins(*rootObject, model.skin_count);

	for (unsigned i = 0; i < model.image_count; i++)
	{
		sGLTFImage& image = model.images[i];

		if (_is_data_uri(image.uri))
		{
			image.embedded = true;
			std::string mime_type;
			if (!_decode_data_uri(&image.data, mime_type, image.uri, 0, false, &image.dataCount))
			{
				assert(false && "here");
			}
		}
		else
		{
			image.embedded = false;
		}

	}

	for (unsigned i = 0; i < model.buffer_count; i++)
	{
		sGLTFBuffer& buffer = model.buffers[i];

		if (_is_data_uri(buffer.uri))
		{
			std::string mime_type;
			if (!_decode_data_uri(&buffer.data, mime_type, buffer.uri, buffer.byte_length, true, &buffer.dataCount))
			{
				assert(false && "here");
			}
		}
		else
		{
			std::string combinedFile = model.root;
			combinedFile.append(buffer.uri);
			unsigned dataSize = 0u;
			void* data = _ReadFile(combinedFile.c_str(), dataSize, "rb");
			buffer.dataCount = dataSize;
			buffer.data = new unsigned char[buffer.dataCount];
			memcpy(buffer.data, data, dataSize);
		}

	}

	Semper::free_json(&rootObject);
	return model;
}

void
Semper::free_gltf(sGLTFModel& model)
{

	for (unsigned i = 0; i < model.mesh_count; i++)
	{
		for (unsigned j = 0; j < model.meshes[i].primitives_count; j++)
		{
			
			for (unsigned k = 0; k < model.meshes[i].primitives[j].target_count; k++)
			{
				delete[] model.meshes[i].primitives[j].targets[k].attributes;
			}
			delete[] model.meshes[i].primitives[j].targets;
			delete[] model.meshes[i].primitives[j].attributes;
		}
		delete[] model.meshes[i].weights;
		delete[] model.meshes[i].primitives;
	}


	for (unsigned i = 0; i < model.node_count; i++)
	{
		if (model.nodes[i].children)
			delete[] model.nodes[i].children;
	}

	for (unsigned i = 0; i < model.scene_count; i++)
	{
		if (model.scenes[i].nodes)
			delete[] model.scenes[i].nodes;
	}

	for (unsigned i = 0; i < model.animation_count; i++)
	{
		if (model.animations[i].sampler_count > 0)
			delete[] model.animations[i].samplers;
		if (model.animations[i].channel_count > 0)
			delete[] model.animations[i].channels;
	}

	for (unsigned i = 0; i < model.skin_count; i++)
	{
		if (model.skins[i].joints_count > 0)
			delete[] model.skins[i].joints;
	}

	for (unsigned i = 0; i < model.extension_count; i++)
	{
		delete[] (model.extensions[i]);
	}

	delete[] model.scenes;
	delete[] model.nodes;
	delete[] model.meshes;
	delete[] model.materials;
	delete[] model.textures;
	delete[] model.samplers;
	delete[] model.images;
	delete[] model.buffers;
	delete[] model.bufferviews;
	delete[] model.accessors;
	delete[] model.cameras;
	delete[] model.animations;
	delete[] model.extensions;
	delete[] model.skins;
	delete[] model.extensions;

	model.scenes = nullptr;
	model.nodes = nullptr;
	model.meshes = nullptr;
	model.materials = nullptr;
	model.textures = nullptr;
	model.samplers = nullptr;
	model.images = nullptr;
	model.buffers = nullptr;
	model.bufferviews = nullptr;
	model.accessors = nullptr;
	model.cameras = nullptr;
	model.animations = nullptr;
	model.extensions = nullptr;
	model.skins = nullptr;
	model.extensions = nullptr;

	model.extension_count = 0u;
	model.scene_count = 0u;
	model.node_count = 0u;
	model.mesh_count = 0u;
	model.material_count = 0u;
	model.texture_count = 0u;
	model.sampler_count = 0u;
	model.image_count = 0u;
	model.buffer_count = 0u;
	model.bufferview_count = 0u;
	model.accessor_count = 0u;
	model.camera_count = 0u;
	model.animation_count = 0u;
	model.extension_count = 0u;
	model.skin_count = 0u;
}

#endif
