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
#define SEMPER_JSON_IMPLEMENTATION
/*
   sJson, v0.1 (WIP)
   * no dependencies
   * simple
   Do this:
	  #define SEMPER_JSON_IMPLEMENTATION
   before you include this file in *one* C or C++ file to create the implementation.
   // i.e. it should look like this:
   #include ...
   #include ...
   #include ...
   #define SEMPER_JSON_IMPLEMENTATION
   #include "sJson.h"
*/

#ifndef SEMPER_JSON_H
#define SEMPER_JSON_H

#ifndef S_JSON_MAX_NAME_LENGTH
#define S_JSON_MAX_NAME_LENGTH 256
#endif

#ifndef S_ASSERT
#include <assert.h>
#define S_ASSERT(x) assert(x)
#endif

#include <stdlib.h>
#include <string.h> // memcpy, strcmp

#ifndef S_ALLOC
#define S_ALLOC(x) malloc(x)
#endif

#ifndef S_FREE
#define S_FREE(x) free(x)
#endif

//-----------------------------------------------------------------------------
// [SECTION] Forward declarations and basic types
//-----------------------------------------------------------------------------
struct sJsonObject;
typedef int sJsonType; // enum -> sJsonType_

//-----------------------------------------------------------------------------
// [SECTION] Semper end-user API functions
//-----------------------------------------------------------------------------
namespace Semper
{
	sJsonObject* load_json(char* rawData, int size);
	void         free_json(sJsonObject** rootObject);
}

//-----------------------------------------------------------------------------
// [SECTION] Enums
//-----------------------------------------------------------------------------

enum sJsonType_
{
	S_JSON_TYPE_NONE,
	S_JSON_TYPE_STRING,
	S_JSON_TYPE_ARRAY,
	S_JSON_TYPE_NUMBER,
	S_JSON_TYPE_BOOL,
	S_JSON_TYPE_OBJECT,
	S_JSON_TYPE_NULL,
};

//-----------------------------------------------------------------------------
// [SECTION] Structs
//-----------------------------------------------------------------------------

struct sJsonObject
{
	sJsonType    type;
	sJsonObject* children;
	int          childCount;	
	char         name[S_JSON_MAX_NAME_LENGTH];
	char*        value;
	void*        _internal;

	// retrieve members
	inline sJsonObject* getMember      (const char* member){ for (int i = 0; i < childCount; i++) if (strcmp(member, children[i].name) == 0) return &children[i]; return nullptr;}
	inline bool         doesMemberExist(const char* member){ return getMember(member) != nullptr;}

	// cast values
	inline int      asInt()    { S_ASSERT(type == S_JSON_TYPE_NUMBER); return (int)strtod(value, nullptr);}
	inline unsigned asUInt()   { S_ASSERT(type == S_JSON_TYPE_NUMBER); return (unsigned)strtod(value, nullptr);}
	inline float    asFloat()  { S_ASSERT(type == S_JSON_TYPE_NUMBER); return (float)asDouble();}
	inline double   asDouble() { S_ASSERT(type == S_JSON_TYPE_NUMBER); return strtod(value, nullptr);}
	inline char*    asString() { S_ASSERT(type == S_JSON_TYPE_STRING); return value;}
	inline bool    	asBool()   { S_ASSERT(type == S_JSON_TYPE_BOOL);   return value[0] == 't';}
	
	// cast array values
	inline void asIntArray   (int*      out, int size) { S_ASSERT(type == S_JSON_TYPE_ARRAY); S_ASSERT(out); if(size > childCount) size = childCount; for(int i = 0; i < size; i++) out[i] = children[i].asInt();}
	inline void asUIntArray  (unsigned* out, int size) { S_ASSERT(type == S_JSON_TYPE_ARRAY); S_ASSERT(out); if(size > childCount) size = childCount; for(int i = 0; i < size; i++) out[i] = children[i].asUInt();}
	inline void asFloatArray (float*    out, int size) { S_ASSERT(type == S_JSON_TYPE_ARRAY); S_ASSERT(out); if(size > childCount) size = childCount; for(int i = 0; i < size; i++) out[i] = children[i].asFloat();}
	inline void asDoubleArray(double*   out, int size) { S_ASSERT(type == S_JSON_TYPE_ARRAY); S_ASSERT(out); if(size > childCount) size = childCount; for(int i = 0; i < size; i++) out[i] = children[i].asDouble();}
	inline void asBoolArray  (bool*     out, int size) { S_ASSERT(type == S_JSON_TYPE_ARRAY); S_ASSERT(out); if(size > childCount) size = childCount; for(int i = 0; i < size; i++) out[i] = children[i].asBool();}
	inline void asStringArray(char**    out, int size) { S_ASSERT(type == S_JSON_TYPE_ARRAY); S_ASSERT(out); if(size > childCount) size = childCount; for(int i = 0; i < size; i++) out[i] = children[i].asString();}

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

	inline sJsonObject& operator[](const char* member) { auto m = getMember(member); S_ASSERT(m!=nullptr); return m==nullptr ? *this : *m;}
	inline sJsonObject& operator[](int i)              { S_ASSERT(children != nullptr); S_ASSERT(i < childCount); return children[i]; };
};

#endif

#ifdef SEMPER_JSON_IMPLEMENTATION

// borrowed from Dear ImGui
template<typename T>
struct mvVector
{
	int size     = 0u;
	int capacity = 0u;
	T*  data     = nullptr;
	inline mvVector() { size = capacity = 0; data = nullptr; }
	inline mvVector<T>& operator=(const mvVector<T>& src) { clear(); resize(src.size); memcpy(data, src.data, (size_t)size * sizeof(T)); return *this; }
	inline bool empty() const { return size == 0; }
	inline int  size_in_bytes() const   { return size * (int)sizeof(T); }
	inline T&   operator[](int i) { S_ASSERT(i >= 0 && i < size); return data[i]; }
	inline void clear() { if (data) { size = capacity = 0; S_FREE(data); data = nullptr; } }
	inline T*   begin() { return data; }
    inline T*   end() { return data + size; }
	inline T&   back() { S_ASSERT(size > 0); return data[size - 1]; }
	inline void swap(mvVector<T>& rhs) { int rhs_size = rhs.size; rhs.size = size; size = rhs_size; int rhs_cap = rhs.capacity; rhs.capacity = capacity; capacity = rhs_cap; T* rhs_data = rhs.data; rhs.data = data; data = rhs_data; }
	inline int  _grow_capacity(int sz) { int new_capacity = capacity ? (capacity + capacity / 2) : 8; return new_capacity > sz ? new_capacity : sz; }
	inline void resize(int new_size) { if (new_size > capacity) reserve(_grow_capacity(new_size)); size = new_size; }
	inline void reserve(int new_capacity) { if (new_capacity <= capacity) return; T* new_data = (T*)S_ALLOC((size_t)new_capacity * sizeof(T)); if (data) { memcpy(new_data, data, (size_t)size * sizeof(T)); S_FREE(data); } data = new_data; capacity = new_capacity; }
	inline void push_back(const T& v) { if (size == capacity) reserve(_grow_capacity(size*2)); memcpy(&data[size], &v, sizeof(v)); size++;}
	inline void pop_back() { S_ASSERT(size > 0); size--; }
};

typedef int sToken_Type;

enum sToken_Type_
{
	S_JSON_TOKEN_NONE,
	S_JSON_TOKEN_LEFT_BRACE,
	S_JSON_TOKEN_RIGHT_BRACE,
	S_JSON_TOKEN_LEFT_BRACKET,
	S_JSON_TOKEN_RIGHT_BRACKET,
	S_JSON_TOKEN_COMMA,
	S_JSON_TOKEN_COLON,
	S_JSON_TOKEN_STRING,
	S_JSON_TOKEN_PRIMITIVE,
	S_JSON_TOKEN_MEMBER, // string but on the left of ":"
};

struct sToken_
{
	sToken_Type type = S_JSON_TOKEN_NONE;
	mvVector<char> value;
};

struct sStack
{
	inline void push(int id){ if (data.empty()) data.resize(2048); if (currentIndex == data.size) data.resize(data.size * 2); data[currentIndex++] = id;}
	inline void pop()   { data[currentIndex] = -1; currentIndex--;}
	inline int  top()   { return data[currentIndex - 1];}
	inline bool empty() { return currentIndex == 0;}

	int currentIndex = 0;
	mvVector<int> data;
};

static void
_parse_for_tokens(char* rawData, mvVector<sToken_>& tokens)
{
	int currentPos = 0u;
	char currentChar = rawData[currentPos];
	char basicTokens[] = { '{', '}', '[', ']', ':', ',' };

	bool inString = false;
	mvVector<char> buffer;

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
						sToken_ primitivetoken{};
						primitivetoken.type = S_JSON_TOKEN_PRIMITIVE;
						buffer.push_back('\0');
						for (int i = 0; i < buffer.size; i++)
						{
							primitivetoken.value.push_back(buffer[i]);
						}
						tokens.push_back({ primitivetoken.type });
						primitivetoken.value.swap(tokens.back().value);
						buffer.clear();
					}

					sToken_ token{};
					if      (currentChar == '{') token.type = S_JSON_TOKEN_LEFT_BRACE;
					else if (currentChar == '}') token.type = S_JSON_TOKEN_RIGHT_BRACE;
					else if (currentChar == '[') token.type = S_JSON_TOKEN_LEFT_BRACKET;
					else if (currentChar == ']') token.type = S_JSON_TOKEN_RIGHT_BRACKET;
					else if (currentChar == ',') token.type = S_JSON_TOKEN_COMMA;
					else if (currentChar == ':') token.type = S_JSON_TOKEN_COLON;

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
					sToken_ token{};
					buffer.push_back('\0');
					for (int i = 0; i < buffer.size; i++)
					{
						token.value.push_back(buffer[i]);
					}


					if (rawData[currentPos + 1] == ':')
						token.type = S_JSON_TOKEN_MEMBER;
					else
						token.type = S_JSON_TOKEN_STRING;
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
_update_children_pointers(sJsonObject* object, mvVector<sJsonObject*>* objects)
{
	S_ASSERT(object->_internal);

	if((*(mvVector<int>*)(object->_internal)).empty())
	{
		S_FREE(object->_internal);
		object->_internal = nullptr;
		return;
	}

	object->childCount = (*(mvVector<int>*)(object->_internal)).size;
	object->children = (sJsonObject*)S_ALLOC(sizeof(sJsonObject)*object->childCount);
	for(int i = 0; i < (*(mvVector<int>*)(object->_internal)).size; i++)
	{
		object->children[i] = *(*objects)[(*(mvVector<int>*)(object->_internal))[i]];
		_update_children_pointers(&object->children[i], objects);
	}

	S_FREE(object->_internal);
	object->_internal = nullptr;
}

sJsonObject*
Semper::load_json(char* rawData, int size)
{
	sStack parentIDStack;
	mvVector<sJsonObject*> objectArray;

	char* spacesRemoved = (char*)S_ALLOC(sizeof(char)*size);
	_remove_whitespace(rawData, spacesRemoved, size);

	mvVector<sToken_>* tokens =  (mvVector<sToken_>*)S_ALLOC(sizeof(mvVector<sToken_>));
	new (tokens) mvVector<sToken_>();
	_parse_for_tokens(spacesRemoved, *tokens);

	sJsonObject *rootObject = (sJsonObject *)S_ALLOC(sizeof(sJsonObject));
	rootObject->type = S_JSON_TYPE_OBJECT;
	rootObject->_internal = (mvVector<int>*)S_ALLOC(sizeof(mvVector<int>));
	new (rootObject->_internal) mvVector<int>();
	
	objectArray.push_back(rootObject);
	parentIDStack.push(0);

	int i = 0;
	bool waitingOnValue = true;
	while (true)
	{

		if (i >= tokens->size)
			break;

		sJsonObject* parent = objectArray[parentIDStack.top()];

		switch ((*tokens)[i].type)
		{

		case S_JSON_TOKEN_LEFT_BRACE:
		{

			if(waitingOnValue)
			{
				waitingOnValue = false; // object was created in S_JSON_TOKEN_MEMBER case below.
			}
			else
			{
				sJsonObject *newObject = (sJsonObject*)S_ALLOC(sizeof(sJsonObject));
				newObject->type = S_JSON_TYPE_OBJECT;
				newObject->_internal = (mvVector<int>*)S_ALLOC(sizeof(mvVector<int>));
				new (newObject->_internal) mvVector<int>();
				objectArray.push_back(newObject);
				parentIDStack.push(objectArray.size-1);
				(*(mvVector<int>*)(parent->_internal)).push_back(objectArray.size-1);
			}
			i++;
			break;
		}

		case S_JSON_TOKEN_LEFT_BRACKET:
		{

			if(waitingOnValue)
			{
				waitingOnValue = false; // object was created in S_JSON_TOKEN_MEMBER case below.
			}
			else
			{
				sJsonObject *newObject = (sJsonObject*)S_ALLOC(sizeof(sJsonObject));
				newObject->type = S_JSON_TYPE_ARRAY;
				newObject->_internal = (mvVector<int>*)S_ALLOC(sizeof(mvVector<int>));
				new (newObject->_internal) mvVector<int>();
				objectArray.push_back(newObject);
				parentIDStack.push(objectArray.size-1);
				(*(mvVector<int>*)(parent->_internal)).push_back(objectArray.size-1);	
			}
			i++;
			break;
		}

		case S_JSON_TOKEN_RIGHT_BRACE:
		{
			parentIDStack.pop();
			i++;
			break;
		}

		case S_JSON_TOKEN_RIGHT_BRACKET:
		{
			parentIDStack.pop();
			i++;
			break;
		}

		case S_JSON_TOKEN_MEMBER:
		{

			sJsonObject* newObject = (sJsonObject*)S_ALLOC(sizeof(sJsonObject));
			objectArray.push_back(newObject);
			newObject->_internal = (mvVector<int>*)S_ALLOC(sizeof(mvVector<int>));
			new (newObject->_internal) mvVector<int>();
			parentIDStack.push(objectArray.size-1);
			(*(mvVector<int>*)(parent->_internal)).push_back(objectArray.size-1);
			memcpy(newObject->name, (*tokens)[i].value.data, (*tokens)[i].value.size_in_bytes());

			// look ahead to 2 tokens to look at type (skipping over ':' )
			sToken_ valueToken = (*tokens)[i + 2];
			sToken_Type valueType = valueToken.type;
			if      (valueType == S_JSON_TOKEN_LEFT_BRACKET) newObject->type = S_JSON_TYPE_ARRAY;
			else if (valueType == S_JSON_TOKEN_LEFT_BRACE)   newObject->type = S_JSON_TYPE_OBJECT;
			else if (valueType == S_JSON_TOKEN_STRING)       newObject->type = S_JSON_TYPE_STRING;
			else if (valueType == S_JSON_TOKEN_PRIMITIVE)
			{
				if(valueToken.value[0] == 't')      newObject->type = S_JSON_TYPE_BOOL;
				else if(valueToken.value[0] == 'f') newObject->type = S_JSON_TYPE_BOOL;
				else if(valueToken.value[0] == 'n') newObject->type = S_JSON_TYPE_NULL;
				else                                newObject->type = S_JSON_TYPE_NUMBER;
			}
			i++;
			waitingOnValue = true;
			break;
		}

		case S_JSON_TOKEN_STRING:
		{
			if(waitingOnValue)
			{
				parent->value = (*tokens)[i].value.data;
				waitingOnValue=false;
				parentIDStack.pop();
			}
			else // in array
			{
				sJsonObject *newObject = (sJsonObject*)S_ALLOC(sizeof(sJsonObject));
				newObject->type = S_JSON_TYPE_STRING;
				newObject->_internal = (mvVector<int>*)S_ALLOC(sizeof(mvVector<int>));
				new (newObject->_internal) mvVector<int>();
				objectArray.push_back(newObject);
				newObject->value = (*tokens)[i].value.data;
				(*(mvVector<int>*)(parent->_internal)).push_back(objectArray.size-1);	
			}
			i++;
			break;
		}

		case S_JSON_TOKEN_PRIMITIVE:
		{
			if(waitingOnValue)
			{
				parent->value = (*tokens)[i].value.data;
				waitingOnValue=false;
				parentIDStack.pop();
			}
			else // in array
			{
				sJsonObject *newObject = (sJsonObject*)S_ALLOC(sizeof(sJsonObject));
				if((*tokens)[i].value.data[0] == 't')      newObject->type = S_JSON_TYPE_BOOL;
				else if((*tokens)[i].value.data[0] == 'f') newObject->type = S_JSON_TYPE_BOOL;
				else if((*tokens)[i].value.data[0] == 'n') newObject->type = S_JSON_TYPE_NULL;
				else                                       newObject->type = S_JSON_TYPE_NUMBER;
				newObject->_internal = (mvVector<int>*)S_ALLOC(sizeof(mvVector<int>));
				new (newObject->_internal) mvVector<int>();
				objectArray.push_back(newObject);
				newObject->value = (*tokens)[i].value.data;
				(*(mvVector<int>*)(parent->_internal)).push_back(objectArray.size-1);	
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
_free_json(sJsonObject* object)
{
	for(int i = 0; i < object->childCount; i++)
	{
		_free_json(&object->children[i]);
	}
	if(object->childCount > 0)
		S_FREE(object->children);
}

void
Semper::free_json(sJsonObject** rootObjectPtr)
{
	sJsonObject* rootObject = *rootObjectPtr;
	_free_json(rootObject);
	S_FREE(rootObject);
	rootObjectPtr = nullptr;
}

#endif

static bool
isDataURI(const std::string& in)
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
is_base64(unsigned char c)
{
	return (isalnum(c) || (c == '+') || (c == '/'));
}

static std::string
base64_decode(std::string const& encoded_string)
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
		is_base64(encoded_string[in_])) {
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
DecodeDataURI(unsigned char** out, std::string& mime_type, const std::string& in, size_t reqBytes, bool checkSize)
{
	std::string header = "data:application/octet-stream;base64,";
	std::string data;
	if (in.find(header) == 0) {
		data = base64_decode(in.substr(header.size()));  // cut mime string.
	}

	if (data.empty()) {
		header = "data:image/jpeg;base64,";
		if (in.find(header) == 0) {
			mime_type = "image/jpeg";
			data = base64_decode(in.substr(header.size()));  // cut mime string.
		}
	}

	if (data.empty()) {
		header = "data:image/png;base64,";
		if (in.find(header) == 0) {
			mime_type = "image/png";
			data = base64_decode(in.substr(header.size()));  // cut mime string.
		}
	}

	if (data.empty()) {
		header = "data:image/bmp;base64,";
		if (in.find(header) == 0) {
			mime_type = "image/bmp";
			data = base64_decode(in.substr(header.size()));  // cut mime string.
		}
	}

	if (data.empty()) {
		header = "data:image/gif;base64,";
		if (in.find(header) == 0) {
			mime_type = "image/gif";
			data = base64_decode(in.substr(header.size()));  // cut mime string.
		}
	}

	if (data.empty()) {
		header = "data:text/plain;base64,";
		if (in.find(header) == 0) {
			mime_type = "text/plain";
			data = base64_decode(in.substr(header.size()));
		}
	}

	if (data.empty()) {
		header = "data:application/gltf-buffer;base64,";
		if (in.find(header) == 0) {
			data = base64_decode(in.substr(header.size()));
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
	}
	else
		*out = new unsigned char[data.size()];

	memcpy(*out, data.data(), reqBytes);
	return true;
}

namespace mvImp {

	static std::string*
	_LoadExtensions(sJsonObject& j, unsigned& size)
	{
		if (!j.doesMemberExist("extensionsUsed"))
			return nullptr;

		unsigned extensionCount = j["extensionsUsed"].childCount;

		std::string* extensions = new std::string[extensionCount];

		for (int i = 0; i < extensionCount; i++)
		{
			sJsonObject& jExtension = j["extensionsUsed"][i];
			extensions[i] = jExtension.value;
			size++;
		}

		return extensions;
	}

	static mvGLTFAnimation*
	_LoadAnimations(sJsonObject& j, unsigned& size)
	{
		sJsonObject* janimations = j.getMember("animations");
		if(janimations == nullptr)
			return nullptr;
		size = janimations->childCount;

		mvGLTFAnimation* animations = new mvGLTFAnimation[size];

		for (int i = 0; i < size; i++)
		{
			sJsonObject& janimation = (*janimations)[i];
			mvGLTFAnimation& animation = animations[i];
			animation.channel_count = 0u;
			animation.channels = nullptr;
			animation.sampler_count = 0u;
			animation.samplers = nullptr;
			animation.name = janimation.getStringMember("name", "");

			if (sJsonObject* jsamplers = janimation.getMember("samplers"))
			{
				animation.sampler_count = jsamplers->childCount;
				animation.samplers = new mvGLTFAnimationSampler[animation.sampler_count];

				for (int s = 0; s < animation.sampler_count; s++)
				{
					sJsonObject& jsampler = (*jsamplers)[s];
					mvGLTFAnimationSampler& sampler = animation.samplers[s];
					sampler.input = jsampler.getIntMember("input", -1);
					sampler.output = jsampler.getIntMember("output", -1);
					sampler.interpolation = jsampler.getStringMember("interpolation", "LINEAR");
				}
			}

			if (sJsonObject* jchannels = janimation.getMember("channels"))
			{
				animation.channel_count = jchannels->childCount;
				animation.channels = new mvGLTFAnimationChannel[animation.channel_count];

				for (int i = 0; i < animation.channel_count; i++)
				{
					sJsonObject& jchannel = (*jchannels)[i];
					mvGLTFAnimationChannel& channel = animation.channels[i];

					channel.sampler = jchannel.getIntMember("sampler", -1);

					if (sJsonObject* jtarget = jchannel.getMember("target"))
					{
						channel.target.node = jtarget->getIntMember("node", -1);
						channel.target.path = jtarget->getStringMember("path", "");
					}
				}
			}
		}
		return animations;
	}

	static mvGLTFCamera*
	_LoadCameras(sJsonObject& j, unsigned& size)
	{
		sJsonObject* jcameras = j.getMember("cameras");
		if(jcameras == nullptr)
			return nullptr;
		size = jcameras->childCount;

		mvGLTFCamera* cameras = new mvGLTFCamera[size];

		for (int i = 0; i < size; i++)
		{
			sJsonObject& jcamera = (*jcameras)[i];
			mvGLTFCamera& camera = cameras[i];
			camera.name = jcamera.getStringMember("name", "");
			std::string type = jcamera.getStringMember("type", "perspective");

			if (type == "perspective") camera.type = MV_IMP_PERSPECTIVE;
			else                       camera.type = MV_IMP_ORTHOGRAPHIC;

			if (sJsonObject* jperspective = jcamera.getMember("perspective"))
			{
				camera.perspective.aspectRatio = jperspective->getFloatMember("aspectRatio", 0.0f);
				camera.perspective.yfov = jperspective->getFloatMember("yfov", 0.0f);
				camera.perspective.zfar = jperspective->getFloatMember("zfar", 0.0f);
				camera.perspective.znear = jperspective->getFloatMember("znear", 0.0f);
			}

			if (sJsonObject* jorthographic = jcamera.getMember("orthographic"))
			{
				camera.orthographic.xmag = jorthographic->getFloatMember("xmag", 0.0f);
				camera.orthographic.ymag = jorthographic->getFloatMember("ymag", 0.0f);
				camera.orthographic.zfar = jorthographic->getFloatMember("zfar", 0.0f);
				camera.orthographic.znear = jorthographic->getFloatMember("znear", 0.0f);
			}
		}
		return cameras;
	}

	static mvGLTFScene*
	_LoadScenes(sJsonObject& j, unsigned& size)
	{
		sJsonObject* jscenes = j.getMember("scenes");
		if(jscenes == nullptr)
			return nullptr;
		size = jscenes->childCount;
		mvGLTFScene* scenes = new mvGLTFScene[size];
		for (int i = 0; i < size; i++)
		{
			sJsonObject& jscene = (*jscenes)[0];
			mvGLTFScene& scene = scenes[i];
			scene.node_count = 0u;
			scene.nodes = nullptr;
			if (sJsonObject* jnodes = jscene.getMember("nodes"))
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

	static mvGLTFNode*
	_LoadNodes(sJsonObject& j, unsigned& size)
	{
		sJsonObject* jnodes = j.getMember("nodes");
		if(jnodes == nullptr)
			return nullptr;
		size = jnodes->childCount;
		mvGLTFNode* nodes = new mvGLTFNode[size];

		for (int i = 0; i < size; i++)
		{
			sJsonObject& jnode = (*jnodes)[i];
			mvGLTFNode& node = nodes[i];
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
			node.name = jnode.getStringMember("name", "");
			node.mesh_index = jnode.getIntMember("mesh", -1);
			node.camera_index = jnode.getIntMember("camera", -1);
			node.skin_index = jnode.getIntMember("skin", -1);
			jnode.getFloatArrayMember("translation", node.translation, 3);
			jnode.getFloatArrayMember("scale", node.scale, 3);
			jnode.getFloatArrayMember("rotation", node.rotation, 4);
			jnode.getFloatArrayMember("matrix", node.matrix, 16);
			node.hadMatrix = jnode.doesMemberExist("matrix");

			if (sJsonObject* jchildren = jnode.getMember("children"))
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

	static mvGLTFMesh*
	_LoadMeshes(sJsonObject& j, unsigned& size)
	{
		sJsonObject* jmeshes = j.getMember("meshes");
		if(jmeshes == nullptr)
			return nullptr;
		size = jmeshes->childCount;
		mvGLTFMesh* meshes = new mvGLTFMesh[size];

		for (int i = 0; i < size; i++)
		{
			sJsonObject& jmesh = (*jmeshes)[i];
			mvGLTFMesh& mesh = meshes[i];
			mesh.primitives = nullptr;
			mesh.primitives_count = 0u;
			mesh.weights = nullptr;
			mesh.weights_count = 0u;
			mesh.name = jmesh.getStringMember("name", "");

			if (sJsonObject* jweights = jmesh.getMember("weights"))
			{
				mesh.weights_count = (*jweights).childCount;
				mesh.weights = new float[mesh.weights_count];
				for (int j = 0; j < mesh.weights_count; j++)
				{
					sJsonObject m = (*jweights)[j];
					mesh.weights[j] = m.asFloat();
				}
			}

			if (sJsonObject* jprimitives = jmesh.getMember("primitives"))
			{
				mesh.primitives_count = jprimitives->childCount;
				mesh.primitives = new mvGLTFMeshPrimitive[mesh.primitives_count];

				for (int j = 0; j < mesh.primitives_count; j++)
				{
					mvGLTFMeshPrimitive& primitive = mesh.primitives[j];
					sJsonObject jprimitive = (*jprimitives)[j];
					primitive.mode = MV_IMP_TRIANGLES;
					primitive.attributes = nullptr;
					primitive.attribute_count = 0u;
					primitive.targets = nullptr;
					primitive.target_count = 0u;
					primitive.indices_index = jprimitive.getIntMember("indices", -1);
					primitive.material_index = jprimitive.getIntMember("material", -1);

					if (sJsonObject* jattributes = jprimitive.getMember("attributes"))
					{
						primitive.attribute_count = jattributes->childCount;
						primitive.attributes = new mvGLTFAttribute[primitive.attribute_count];
						
						for (int k = 0; k < primitive.attribute_count; k++)
						{
							sJsonObject& m = (*jattributes)[k];
							memcpy(primitive.attributes[k].semantic, m.name, MV_IMPORTER_MAX_NAME_LENGTH);
							primitive.attributes[k].index = m.asInt();
						}
					}

					if (sJsonObject* jtargets = jprimitive.getMember("targets"))
					{
						primitive.target_count = jtargets->childCount;
						primitive.targets = new mvGLTFMorphTarget[primitive.target_count];

						for (int k = 0; k < primitive.target_count; k++)
						{
							mvGLTFMorphTarget& target = primitive.targets[k];
							sJsonObject& jtarget = (*jtargets)[k];
							
							target.attribute_count = jtarget.childCount;
							target.attributes = new mvGLTFAttribute[target.attribute_count];
							for (int x = 0; x < target.attribute_count; x++)
							{
								sJsonObject& jattribute = jtarget[x];
								memcpy(target.attributes[x].semantic, jattribute.name, MV_IMPORTER_MAX_NAME_LENGTH);
								target.attributes[x].index = jattribute.asInt();
							}
						}
					}
				}
			}
		}
		return meshes;
	}

	static mvGLTFMaterial*
	_LoadMaterials(sJsonObject& j, unsigned& size)
	{
		sJsonObject* jmaterials = j.getMember("materials");
		if(jmaterials == nullptr)
			return nullptr;
		size = jmaterials->childCount;
		mvGLTFMaterial* materials = new mvGLTFMaterial[size];

		for (int i = 0; i < size; i++)
		{
			sJsonObject& jmaterial = (*jmaterials)[i];
			mvGLTFMaterial& material = materials[i];
			material.name = jmaterial.getStringMember("name", "");
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
			if     (alphaMode == "OPAQUE") material.alphaMode = MV_ALPHA_MODE_OPAQUE;
			else if(alphaMode == "MASK")   material.alphaMode = MV_ALPHA_MODE_MASK;
			else                           material.alphaMode = MV_ALPHA_MODE_BLEND;

			if (sJsonObject* jrt = jmaterial.getMember("normalTexture"))
			{
				material.normal_texture = jrt->getIntMember("index", -1);
				material.normal_texture_scale = jrt->getFloatMember("scale", 1.0f);
			}

			if (sJsonObject* jot = jmaterial.getMember("occlusionTexture"))
			{
				material.occlusion_texture = jot->getIntMember("index", -1);
				material.occlusion_texture_strength = jot->getFloatMember("strength", 1.0f);
			}

			if (sJsonObject* jet = jmaterial.getMember("emissiveTexture"))
			{
				material.emissive_texture = jet->getIntMember("index", -1);
			}

			if (sJsonObject* jpbrmr = jmaterial.getMember("pbrMetallicRoughness"))
			{
				material.pbrMetallicRoughness = true;
				material.metallic_factor = jpbrmr->getFloatMember("metallicFactor", 1.0f);
				material.roughness_factor = jpbrmr->getFloatMember("roughnessFactor", 1.0f);
				jpbrmr->getFloatArrayMember("baseColorFactor", material.base_color_factor, 4);

				if (sJsonObject* jbaseColorTexture = jpbrmr->getMember("baseColorTexture"))
				{
					material.base_color_texture = jbaseColorTexture->getIntMember("index", -1);
				}
				
				if (sJsonObject* jmrt = jpbrmr->getMember("metallicRoughnessTexture"))
				{
					material.metallic_roughness_texture = jmrt->getIntMember("index", -1);
				}

			}

			if (sJsonObject* jextensions = jmaterial.getMember("extensions"))
			{
				if (sJsonObject* jKHR_cc = jextensions->getMember("KHR_materials_clearcoat"))
				{
					material.clearcoat_extension = true;
					material.clearcoat_factor = jKHR_cc->getFloatMember("clearcoatFactor", 0.0f);
					material.clearcoat_roughness_factor = jKHR_cc->getIntMember("clearcoatRoughnessFactor", 0.0f);

					if (sJsonObject* jcct = jKHR_cc->getMember("clearcoatTexture"))
					{
						material.clearcoat_texture = jcct->getIntMember("index", -1);
						material.clearcoat_normal_texture_scale = jcct->getFloatMember("scale", 1.0f);
					}
					if (sJsonObject* jccrt = jKHR_cc->getMember("clearcoatRoughnessTexture"))
					{
						material.clearcoat_roughness_texture = jccrt->getIntMember("index", -1);
					}
					if (sJsonObject* jccnt = jKHR_cc->getMember("clearcoatNormalTexture"))
					{
						material.clearcoat_normal_texture = jccnt->getIntMember("index", -1);
					}
				}
			}
		}
		return materials;
	}

	static mvGLTFTexture*
	_LoadTextures(sJsonObject& j, unsigned& size)
	{
		sJsonObject* jtextures = j.getMember("textures");
		if(jtextures == nullptr)
			return nullptr;
		size = jtextures->childCount;
		mvGLTFTexture* textures = new mvGLTFTexture[size];

		for (int i = 0; i < size; i++)
		{
			sJsonObject& jtexture = (*jtextures)[i];
			mvGLTFTexture& texture = textures[i];
			texture.sampler_index = jtexture.getIntMember("sampler", -1);
			texture.image_index = jtexture.getIntMember("source", -1);
			texture.name = jtexture.getStringMember("name", "");
		}
		return textures;
	}

	static mvGLTFSampler*
	_LoadSamplers(sJsonObject& j, unsigned& size)
	{
		sJsonObject* jsamplers = j.getMember("samplers");
		if(jsamplers == nullptr)
			return nullptr;
		size = jsamplers->childCount;
		mvGLTFSampler* samplers = new mvGLTFSampler[size];

		for (int i = 0; i < size; i++)
		{
			sJsonObject& jsampler = (*jsamplers)[i];
			mvGLTFSampler& sampler = samplers[i];
			sampler.name = jsampler.getStringMember("name", "");
			sampler.mag_filter = jsampler.getIntMember("magFilter", -1);
			sampler.min_filter = jsampler.getIntMember("minFilter", -1);
			sampler.wrap_s = jsampler.getIntMember("wrapS", MV_IMP_WRAP_REPEAT);
			sampler.wrap_t = jsampler.getIntMember("wrapT", MV_IMP_WRAP_REPEAT);
		}
		return samplers;
	}

	static mvGLTFImage*
	_LoadImages(sJsonObject& j, unsigned& size)
	{
		sJsonObject* jimages = j.getMember("images");
		if(jimages == nullptr)
			return nullptr;
		size = jimages->childCount;
		mvGLTFImage* images = new mvGLTFImage[size];

		for (int i = 0; i < size; i++)
		{
			sJsonObject& jimage = (*jimages)[i];
			mvGLTFImage& image = images[i];
			image.uri = jimage.getStringMember("uri", "");
			image.mimeType = jimage.getStringMember("mimeType", "");
			image.buffer_view_index = jimage.getIntMember("bufferView", -1);
			image.data = nullptr;
			image.dataCount = 0u;
			image.embedded = false; // ?
		}
		return images;
	}

	static mvGLTFBuffer*
	_LoadBuffers(sJsonObject& j, unsigned& size)
	{
		sJsonObject* jbuffers = j.getMember("buffers");
		if(jbuffers == nullptr)
			return nullptr;
		size = jbuffers->childCount;
		mvGLTFBuffer* buffers = new mvGLTFBuffer[size];
		for (int i = 0; i < size; i++)
		{
			sJsonObject& jbuffer = (*jbuffers)[i];
			mvGLTFBuffer& buffer = buffers[i];
			buffer.uri = jbuffer.getStringMember("uri", "");
			buffer.byte_length = jbuffer.getUIntMember("byteLength", 0u);	
			buffer.dataCount = 0u;
			buffer.data = nullptr;
		}
		return buffers;
	}

	static mvGLTFBufferView*
	_LoadBufferViews(sJsonObject& j, unsigned& size)
	{
		sJsonObject* jbufferViews = j.getMember("bufferViews");
		if(jbufferViews == nullptr)
			return nullptr;
		size = jbufferViews->childCount;
		mvGLTFBufferView* bufferviews = new mvGLTFBufferView[size];

		for (int i = 0; i < size; i++)
		{
			sJsonObject& jbufferview = (*jbufferViews)[i];
			mvGLTFBufferView& bufferview = bufferviews[i];
			bufferview.name = jbufferview.getStringMember("name", "");
			bufferview.buffer_index = jbufferview.getIntMember("buffer", -1);
			bufferview.byte_offset = jbufferview.getIntMember("byteOffset", 0);
			bufferview.byte_length = jbufferview.getIntMember("byteLength", -1);
			bufferview.byte_stride = jbufferview.getIntMember("byteStride", -1);
		}
		return bufferviews;
	}

	static mvGLTFAccessor*
	_LoadAccessors(sJsonObject& j, unsigned& size)
	{
		sJsonObject* jaccessors = j.getMember("accessors");
		if(jaccessors == nullptr)
			return nullptr;
		size = jaccessors->childCount;
		mvGLTFAccessor* accessors = new mvGLTFAccessor[size];

		for (int i = 0; i < size; i++)
		{
			sJsonObject& jaccessor = (*jaccessors)[i];
			mvGLTFAccessor& accessor = accessors[i];
			accessor.name = jaccessor.getStringMember("name", "");
			accessor.byteOffset = jaccessor.getIntMember("byteOffset", 0);
			accessor.count = jaccessor.getIntMember("count", -1);
			accessor.component_type = (mvGLTFComponentType)jaccessor.getIntMember("componentType", MV_IMP_FLOAT);
			accessor.buffer_view_index = jaccessor.getIntMember("bufferView", -1);
			accessor.type = MV_IMP_SCALAR;
			jaccessor.getFloatArrayMember("max", accessor.maxes, 16);
			jaccessor.getFloatArrayMember("min", accessor.mins, 16);

			std::string accessorType = jaccessor.getStringMember("type", "SCALAR");
			if (accessorType == "SCALAR")    accessor.type = MV_IMP_SCALAR;
			else if (accessorType == "VEC2") accessor.type = MV_IMP_VEC2;
			else if (accessorType == "VEC3") accessor.type = MV_IMP_VEC3;
			else if (accessorType == "VEC4") accessor.type = MV_IMP_VEC4;
			else if (accessorType == "MAT2") accessor.type = MV_IMP_MAT2;
			else if (accessorType == "MAT3") accessor.type = MV_IMP_MAT3;
			else if (accessorType == "MAT4") accessor.type = MV_IMP_MAT4;
		}
		return accessors;
	}

	static mvGLTFSkin*
	_LoadSkins(sJsonObject& j, unsigned& size)
	{
		sJsonObject* jskins = j.getMember("skins");
		if(jskins == nullptr)
			return nullptr;
		size = jskins->childCount;
		mvGLTFSkin* skins = new mvGLTFSkin[size];

		for (int i = 0; i < size; i++)
		{
			sJsonObject& jnode = (*jskins)[i];
			mvGLTFSkin& skin = skins[i];
			skin.name = jnode.getStringMember("name", "");
			skin.inverseBindMatrices = jnode.getIntMember("inverseBindMatrices", -1);
			skin.skeleton = jnode.getIntMember("skeleton", -1);
			skin.joints = nullptr;
			skin.joints_count = 0u;

			if (sJsonObject* jjoints = jnode.getMember("joints"))
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
}

static mvGLTFModel
mvLoadBinaryGLTF(const char* root, const char* file)
{

	mvGLTFModel model{};
	model.root = root;
	model.name = file;

	unsigned dataSize = 0u;
	char* data = (char*)mvImp::_ReadFile(file, dataSize, "rb");

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

	sJsonObject* rootObject = Semper::load_json(chunkData, chunkLength);
	model.scene = rootObject->getIntMember("scene", 0);
	model.scenes = mvImp::_LoadScenes(*rootObject, model.scene_count);
	model.nodes = mvImp::_LoadNodes(*rootObject, model.node_count);
	model.materials = mvImp::_LoadMaterials(*rootObject, model.material_count);
	model.meshes = mvImp::_LoadMeshes(*rootObject, model.mesh_count);
	model.textures = mvImp::_LoadTextures(*rootObject, model.texture_count);
	model.samplers = mvImp::_LoadSamplers(*rootObject, model.sampler_count);
	model.images = mvImp::_LoadImages(*rootObject, model.image_count);
	model.buffers = mvImp::_LoadBuffers(*rootObject, model.buffer_count);
	model.bufferviews = mvImp::_LoadBufferViews(*rootObject, model.bufferview_count);
	model.accessors = mvImp::_LoadAccessors(*rootObject, model.accessor_count);
	model.cameras = mvImp::_LoadCameras(*rootObject, model.camera_count);
	model.animations = mvImp::_LoadAnimations(*rootObject, model.animation_count);
	model.extensions = mvImp::_LoadExtensions(*rootObject, model.extension_count);
	model.skins = mvImp::_LoadSkins(*rootObject, model.skin_count);

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
		mvGLTFImage& image = model.images[i];

		if (image.buffer_view_index > -1)
		{
			image.embedded = true;
			//image.data = model.buffers
			mvGLTFBufferView bufferView = model.bufferviews[image.buffer_view_index];
			char* bufferRawData = (char*)model.buffers[bufferView.buffer_index].data;
			char* bufferRawSection = &bufferRawData[bufferView.byte_offset]; // start of buffer section
			image.dataCount = bufferView.byte_length;
			image.data = new unsigned char[image.dataCount];
			memcpy(image.data, bufferRawSection, bufferView.byte_length);
			continue;
		}

		if (isDataURI(image.uri))
		{
			image.embedded = true;
			std::string mime_type;
			if (!DecodeDataURI(&image.data, mime_type, image.uri, 0, false))
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
		mvGLTFBuffer& buffer = model.buffers[i];

		if (buffer.data)
			continue;

		if (isDataURI(buffer.uri))
		{
			std::string mime_type;
			if (!DecodeDataURI(&buffer.data, mime_type, buffer.uri, buffer.byte_length, true))
			{
				assert(false && "here");
			}
		}
		else
		{
			std::string combinedFile = model.root;
			combinedFile.append(buffer.uri);
			unsigned dataSize = 0u;
			void* bufferdata = mvImp::_ReadFile(combinedFile.c_str(), dataSize, "rb");
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

mvGLTFModel
mvLoadGLTF(const char* root, const char* file)
{

	size_t len = strlen(file);
	if (file[len - 1] == 'b')
		return mvLoadBinaryGLTF(root, file);

	mvGLTFModel model{};
	model.root = root;
	model.name = file;

	unsigned dataSize = 0u;
	char* data = mvImp::_ReadFile(file, dataSize, "rb");

	if (dataSize == 0u)
	{
		assert(false && "File not found.");
		return model;
	}

	sJsonObject* rootObject = Semper::load_json(data, dataSize);
	delete[] data;
	model.scene = rootObject->getIntMember("scene", 0);
	model.scenes = mvImp::_LoadScenes(*rootObject, model.scene_count);
	model.nodes = mvImp::_LoadNodes(*rootObject, model.node_count);
	model.materials = mvImp::_LoadMaterials(*rootObject, model.material_count);
	model.meshes = mvImp::_LoadMeshes(*rootObject, model.mesh_count);
	model.textures = mvImp::_LoadTextures(*rootObject, model.texture_count);
	model.samplers = mvImp::_LoadSamplers(*rootObject, model.sampler_count);
	model.images = mvImp::_LoadImages(*rootObject, model.image_count);
	model.buffers = mvImp::_LoadBuffers(*rootObject, model.buffer_count);
	model.bufferviews = mvImp::_LoadBufferViews(*rootObject, model.bufferview_count);
	model.accessors = mvImp::_LoadAccessors(*rootObject, model.accessor_count);
	model.cameras = mvImp::_LoadCameras(*rootObject, model.camera_count);
	model.animations = mvImp::_LoadAnimations(*rootObject, model.animation_count);
	model.extensions = mvImp::_LoadExtensions(*rootObject, model.extension_count);
	model.skins = mvImp::_LoadSkins(*rootObject, model.skin_count);

	for (unsigned i = 0; i < model.image_count; i++)
	{
		mvGLTFImage& image = model.images[i];

		if (isDataURI(image.uri))
		{
			image.embedded = true;
			std::string mime_type;
			if (!DecodeDataURI(&image.data, mime_type, image.uri, 0, false))
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
		mvGLTFBuffer& buffer = model.buffers[i];

		if (isDataURI(buffer.uri))
		{
			std::string mime_type;
			if (!DecodeDataURI(&buffer.data, mime_type, buffer.uri, buffer.byte_length, true))
			{
				assert(false && "here");
			}
		}
		else
		{
			std::string combinedFile = model.root;
			combinedFile.append(buffer.uri);
			unsigned dataSize = 0u;
			void* data = mvImp::_ReadFile(combinedFile.c_str(), dataSize, "rb");
			buffer.dataCount = dataSize;
			buffer.data = new unsigned char[buffer.dataCount];
			memcpy(buffer.data, data, dataSize);
		}

	}

	Semper::free_json(&rootObject);
	return model;
}

void
mvCleanupGLTF(mvGLTFModel& model)
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
