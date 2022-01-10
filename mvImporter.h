/*
 mvImporter, v0.0.1 (WIP)
   * no dependencies
   * assumes GLTF file is correct
   * ignores animation for now
   * contains a hackly put together json parser

   Do this:
	  #define MV_IMPORTER_IMPLEMENTATION
   before you include this file in *one* C or C++ file to create the implementation.
   // i.e. it should look like this:
   #include ...
   #include ...
   #include ...
   #define MV_IMPORTER_IMPLEMENTATION
   #include "mvImporter.h"
*/

#pragma once

#ifndef MV_IMPORTER_API
#define MV_IMPORTER_API
#endif

#include <string>
#include <vector>

//#define MV_IMPORTER_IMPLEMENTATION
#ifdef MV_IMPORTER_IMPLEMENTATION
#include <assert.h>
#include <stdio.h>
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

//-----------------------------------------------------------------------------
// mvImporter End-User API
//-----------------------------------------------------------------------------

MV_IMPORTER_API mvGLTFModel mvLoadGLTF      (const char* root, const char* file);
MV_IMPORTER_API void        mvCleanupGLTF   (mvGLTFModel& model);

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

enum mvGLTFAlphaMode
{
	MV_ALPHA_MODE_OPAQUE,
	MV_ALPHA_MODE_MASK,
	MV_ALPHA_MODE_BLEND
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
	std::string semantic;
	mvS32       index = -1; // accessor index
};

struct mvGLTFAccessor
{
	std::string         name;
	mvGLTFAccessorType  type = MV_IMP_SCALAR;
	mvS32               buffer_view_index = -1;
	mvGLTFComponentType component_type = MV_IMP_FLOAT;
	mvS32               byteOffset = 0;
	mvS32               count = -1;
	mvF32               maxes[16];
	mvF32               mins[16];
};

struct mvGLTFTexture
{
	std::string name;
	mvS32       image_index   = -1;
	mvS32       sampler_index = -1;
};

struct mvGLTFSampler
{
	std::string name;
	mvS32       mag_filter = -1;
	mvS32       min_filter = -1;
	mvS32       wrap_s     = MV_IMP_WRAP_REPEAT;
	mvS32       wrap_t     = MV_IMP_WRAP_REPEAT;
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

struct mvGLTFMorphTarget
{
	mvGLTFAttribute* attributes = nullptr;
	mvU32            attribute_count = 0u;
};

struct mvGLTFMeshPrimitive
{
	mvS32              indices_index = -1; // accessor index
	mvS32              material_index = -1;
	mvGLTFPrimMode     mode = MV_IMP_TRIANGLES;
	mvGLTFAttribute*   attributes = nullptr;
	mvU32              attribute_count = 0u;
	mvGLTFMorphTarget* targets = nullptr;
	mvU32              target_count = 0u;
};

struct mvGLTFMesh
{
	std::string          name;
	mvGLTFMeshPrimitive* primitives = nullptr;
	mvU32                primitives_count = 0u;
	mvF32*               weights = nullptr;
	mvU32                weights_count = 0u;
};

struct mvGLTFMaterial
{
	std::string     name;
	mvS32           base_color_texture             = -1;
	mvS32           metallic_roughness_texture     = -1;
	mvS32           normal_texture                 = -1;
	mvS32           occlusion_texture              = -1;
	mvS32           emissive_texture               = -1;
	mvS32           clearcoat_texture              = -1;
	mvS32           clearcoat_roughness_texture    = -1;
	mvS32           clearcoat_normal_texture       = -1;
	mvF32           normal_texture_scale           = 1.0f;
	mvF32           clearcoat_normal_texture_scale = 1.0f;
	mvF32           occlusion_texture_strength     = 1.0f;
	mvF32           metallic_factor                = 1.0f;
	mvF32           roughness_factor               = 1.0f;
	mvF32           base_color_factor[4]           = { 1.0f, 1.0f, 1.0f, 1.0f };
	mvF32           emissive_factor[3]             = { 0.0f, 0.0f, 0.0f };
	mvF32           alphaCutoff                    = 0.5;
	bool            double_sided                   = false;
	mvGLTFAlphaMode alphaMode                      = MV_ALPHA_MODE_OPAQUE;
	mvF32           clearcoat_factor               = 0.0;
	mvF32           clearcoat_roughness_factor     = 0.0;

	// extensions & models
	bool pbrMetallicRoughness  = false;
	bool clearcoat_extension   = false;
};

struct mvGLTFPerspective
{
	mvF32 aspectRatio = 0.0f;
	mvF32 yfov = 0.0f;
	mvF32 zfar = 0.0f;
	mvF32 znear = 0.0f;
};

struct mvGLTFOrthographic
{
	mvF32 xmag = 0.0f;
	mvF32 ymag = 0.0f;
	mvF32 zfar = 0.0f;
	mvF32 znear = 0.0f;
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
	mvS32       mesh_index   = -1;
	mvS32       skin_index   = -1;
	mvS32       camera_index = -1;
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

struct mvGLTFAnimationChannelTarget
{
	mvS32       node = -1;
	std::string path;
};

struct mvGLTFAnimationChannel
{
	mvS32                        sampler = -1;
	mvGLTFAnimationChannelTarget target;
};

struct mvGLTFAnimationSampler
{
	mvS32       input = -1;
	mvS32       output = -1;
	std::string interpolation = "LINEAR";
};

struct mvGLTFAnimation
{
	std::string             name;
	mvGLTFAnimationChannel* channels = nullptr;
	mvU32                   channel_count = 0u;
	mvGLTFAnimationSampler* samplers = nullptr;
	mvU32                   sampler_count = 0u;
};

struct mvGLTFScene
{
	mvU32* nodes = nullptr;
	mvU32  node_count = 0u;
};

struct mvGLTFSkin
{
	std::string name;
	mvS32       inverseBindMatrices = -1;
	mvS32       skeleton = -1;
	mvU32*      joints = nullptr;
	mvU32       joints_count = 0u;
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

	mvS32 scene            = -1;
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
	mvU32 camera_count     = 0u;
	mvU32 animation_count  = 0u;
	mvU32 skin_count       = 0u;
	mvU32 extension_count  = 0u;
};

//-----------------------------------------------------------------------------
// mvImporter Implementation
//-----------------------------------------------------------------------------

#ifdef MV_IMPORTER_IMPLEMENTATION

struct mvJsonContext; // "root" json object
struct mvJsonObject;  // json object or array
struct mvJsonValue;   // primitive/string json value
struct mvJsonMember;  // json member (name + primitive/string value)
struct mvStack;       // simple stack-like object helper
struct mvToken;       // token from parsing

enum mvJsonType
{
	MV_JSON_TYPE_NONE,
	MV_JSON_TYPE_STRING,
	MV_JSON_TYPE_ARRAY,
	MV_JSON_TYPE_PRIMITIVE,
	MV_JSON_TYPE_OBJECT
};

enum mvTokenType
{
	MV_JSON_NONE,
	MV_JSON_LEFT_BRACE,
	MV_JSON_RIGHT_BRACE,
	MV_JSON_LEFT_BRACKET,
	MV_JSON_RIGHT_BRACKET,
	MV_JSON_COMMA,
	MV_JSON_COLON,
	MV_JSON_STRING,
	MV_JSON_PRIMITIVE,
	MV_JSON_MEMBER, // string but on the left of ":"
};

struct mvStack
{
	void push(int value);
	void pop();
	int  top();
	bool empty();

	int currentIndex = 0;
	std::vector<int> data;
};

struct mvToken
{
	std::string value;
	mvTokenType type = MV_JSON_NONE;
};

struct mvJsonValue
{
	std::string value;
};

struct mvJsonMember
{
	std::string name;
	mvJsonType  type = MV_JSON_TYPE_NONE;
	int         index = -1;
	mvJsonContext* context = nullptr;

	operator char* ();
	operator int();
	operator mvU32         ();
	operator float();
	operator mvJsonObject& ();
};

struct mvJsonObject
{
	mvJsonType                type = MV_JSON_TYPE_NONE;
	std::vector<mvJsonMember> members;
	mvJsonContext*            context = nullptr;

	mvJsonMember& getMember(const char* member);
	bool          doesMemberExist(const char* member);

	mvJsonObject& operator[](const char* member);
	mvJsonMember& operator[](int i);

};

struct mvJsonContext
{
	std::vector<mvJsonValue>  primitiveValues;
	std::vector<mvJsonObject> jsonObjects;

	bool doesMemberExist(const char* member);
	mvJsonObject& operator[](const char* member);
};

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

bool
DecodeDataURI(std::vector<unsigned char>* out, std::string& mime_type,
	const std::string& in, size_t reqBytes, bool checkSize)
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
	if (data.empty()) {
		return false;
	}

	if (checkSize) {
		if (data.size() != reqBytes) {
			return false;
		}
		out->resize(reqBytes);
	}
	else {
		out->resize(data.size());
	}
	std::copy(data.begin(), data.end(), out->begin());
	return true;
}

static void
ParseForTokens(char* rawData, std::vector<mvToken>& tokens)
{
	int currentPos = 0u;
	char currentChar = rawData[currentPos];
	char basicTokens[] = { '{', '}', '[', ']', ':', ',' };

	bool inString = false;
	std::vector<char> buffer;

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
						mvToken primitivetoken{};
						primitivetoken.type = MV_JSON_PRIMITIVE;
						buffer.push_back('\0');
						for (int i = 0; i < buffer.size(); i++)
						{
							primitivetoken.value += buffer[i];
						}
						tokens.push_back(primitivetoken);
						buffer.clear();
					}

					mvToken token{};
					if (currentChar == '{') token.type = MV_JSON_LEFT_BRACE;
					else if (currentChar == '}') token.type = MV_JSON_RIGHT_BRACE;
					else if (currentChar == '[') token.type = MV_JSON_LEFT_BRACKET;
					else if (currentChar == ']') token.type = MV_JSON_RIGHT_BRACKET;
					else if (currentChar == ',') token.type = MV_JSON_COMMA;
					else if (currentChar == ':') token.type = MV_JSON_COLON;

					char cc[2];
					cc[0] = currentChar;
					cc[1] = 0;
					token.value = std::string(cc);
					tokens.push_back(token);
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
					mvToken token{};
					buffer.push_back('\0');
					for (int i = 0; i < buffer.size(); i++)
					{
						token.value += buffer[i];
					}


					if (rawData[currentPos + 1] == ':')
						token.type = MV_JSON_MEMBER;
					else
						token.type = MV_JSON_STRING;
					tokens.push_back(token);
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
RemoveWhiteSpace(char* rawData, char* spacesRemoved, size_t size)
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

static mvJsonContext*
ParseJSON(char* rawData, int size)
{
	mvJsonContext* ptrContext = new mvJsonContext();
	mvJsonContext& context = *ptrContext;
	mvStack jsonObjectStack;
	mvStack jsonMemberStack;

	char* spacesRemoved = new char[size];
	RemoveWhiteSpace(rawData, spacesRemoved, size);

	std::vector<mvToken>* tokens = new std::vector<mvToken>;
	ParseForTokens(spacesRemoved, *tokens);

	//for (auto& token : tokens)
	//	std::cout << token.value << std::endl;

	mvJsonObject rootObject{};
	rootObject.type = MV_JSON_TYPE_OBJECT;
	rootObject.members.push_back({});
	context.jsonObjects.push_back(rootObject);
	jsonObjectStack.push(0);
	jsonMemberStack.push(0);

	int i = 0;
	while (true)
	{

		if (i >= tokens->size())
			break;

		switch ((*tokens)[i].type)
		{

		case MV_JSON_LEFT_BRACE:
		{
			int parentId = jsonObjectStack.top();
			mvJsonObject& parent = context.jsonObjects[parentId];

			if (parent.type == MV_JSON_TYPE_OBJECT)
			{
				mvJsonObject jobject{};
				jobject.type = MV_JSON_TYPE_OBJECT;
				int newObjectIndex = context.jsonObjects.size();
				int memberId = jsonMemberStack.top();
				parent.members[memberId].index = newObjectIndex;
				context.jsonObjects.push_back(jobject);
				jsonObjectStack.push(newObjectIndex);
				if (!jsonMemberStack.empty())
					jsonMemberStack.pop();
				i++;
			}

			else if (parent.type == MV_JSON_TYPE_ARRAY)
			{

				mvJsonObject jobject{};
				jobject.type = MV_JSON_TYPE_OBJECT;
				int newObjectIndex = context.jsonObjects.size();

				mvJsonMember member{};
				member.type = MV_JSON_TYPE_PRIMITIVE;
				member.index = newObjectIndex;
				parent.members.push_back(member);
				context.jsonObjects.push_back(jobject);
				jsonObjectStack.push(newObjectIndex);
				//jsonMemberStack.pop();
				i++;
			}
			break;
		}

		case MV_JSON_LEFT_BRACKET:
		{
			mvJsonObject jobject{};
			jobject.type = MV_JSON_TYPE_ARRAY;
			int newObjectIndex = context.jsonObjects.size();
			context.jsonObjects.push_back(jobject);
			jsonObjectStack.push(newObjectIndex);
			jsonMemberStack.pop();
			i++;
			break;
		}

		case MV_JSON_RIGHT_BRACE:
		{
			int finishedObject = jsonObjectStack.top();
			jsonObjectStack.pop();
			i++;
			break;
		}

		case MV_JSON_RIGHT_BRACKET:
		{
			int finishedArray = jsonObjectStack.top();
			jsonObjectStack.pop();
			i++;
			break;
		}

		case MV_JSON_MEMBER:
		{
			int parentId = jsonObjectStack.top();
			mvJsonObject& parent = context.jsonObjects[parentId];
			mvJsonMember member{};
			member.name = (*tokens)[i].value;
			//if(i+)
			mvTokenType valueType = (*tokens)[i + 2].type;
			if (valueType == MV_JSON_LEFT_BRACKET)
			{
				member.index = context.jsonObjects.size();
				member.type = MV_JSON_TYPE_ARRAY;
			}
			else if (valueType == MV_JSON_LEFT_BRACE)
			{
				member.index = context.jsonObjects.size();
				member.type = MV_JSON_TYPE_OBJECT;
			}
			else if (valueType == MV_JSON_PRIMITIVE)
			{
				member.index = context.primitiveValues.size();
				member.type = MV_JSON_TYPE_PRIMITIVE;
			}
			else if (valueType == MV_JSON_STRING)
			{
				member.index = context.primitiveValues.size();
				member.type = MV_JSON_TYPE_STRING;
			}
			int memberId = parent.members.size();
			parent.members.push_back(member);
			jsonMemberStack.push(memberId);
			i++;
			break;
		}

		case MV_JSON_STRING:
		{
			int parentId = jsonObjectStack.top();
			mvJsonObject& parent = context.jsonObjects[parentId];

			if (parent.type == MV_JSON_TYPE_OBJECT)
			{
				int valueId = context.primitiveValues.size();
				context.primitiveValues.push_back({});
				context.primitiveValues.back().value = (*tokens)[i].value;
				int memberId = jsonMemberStack.top();
				parent.members[memberId].index = valueId;
				jsonMemberStack.pop();
			}
			else if (parent.type == MV_JSON_TYPE_ARRAY)
			{

				int valueId = context.primitiveValues.size();
				context.primitiveValues.push_back({});
				context.primitiveValues.back().value = (*tokens)[i].value;

				mvJsonMember member{};
				member.type = MV_JSON_TYPE_PRIMITIVE;
				member.index = valueId;
				parent.members.push_back(member);
			}
			i++;
			break;
		}

		case MV_JSON_PRIMITIVE:
		{
			int parentId = jsonObjectStack.top();
			mvJsonObject& parent = context.jsonObjects[parentId];
			if (parent.type == MV_JSON_TYPE_OBJECT)
			{
				int valueId = context.primitiveValues.size();
				context.primitiveValues.push_back({});
				context.primitiveValues.back().value = (*tokens)[i].value;
				int memberId = jsonMemberStack.top();
				parent.members[memberId].index = valueId;
				jsonMemberStack.pop();
			}

			else if (parent.type == MV_JSON_TYPE_ARRAY)
			{

				int valueId = context.primitiveValues.size();
				context.primitiveValues.push_back({});
				context.primitiveValues.back().value = (*tokens)[i].value;

				mvJsonMember member{};
				member.type = MV_JSON_TYPE_PRIMITIVE;
				member.index = valueId;
				parent.members.push_back(member);
			}
			i++;
			break;
		}

		default:
			i++;
			break;
		}

	}
	jsonObjectStack.pop();

	for (auto& object : context.jsonObjects)
	{
		object.context = ptrContext;
		for (auto& member : object.members)
			member.context = ptrContext;
	}

	return ptrContext;
}

bool
mvJsonContext::doesMemberExist(const char* member)
{
	for (int i = 0; i < jsonObjects[1].members.size(); i++)
	{
		const char* name = jsonObjects[1].members[i].name.c_str();

		if (strcmp(member, name) == 0)
			return true;
	}

	return false;
}

mvJsonObject&
mvJsonContext::operator[](const char* member)
{
	for (int i = 0; i < jsonObjects[1].members.size(); i++)
	{
		const char* name = jsonObjects[1].members[i].name.c_str();

		if (strcmp(member, name) == 0)
		{
			int index = jsonObjects[1].members[i].index;
			return jsonObjects[index];
		}
	}

	return jsonObjects[0];
}

void
mvStack::push(int value)
{
	if (data.empty())
		data.resize(2048);

	if (value == data.size())
	{
		data.resize(value * 2);
	}
	data[currentIndex] = value;
	currentIndex++;
}

void
mvStack::pop()
{
	data[currentIndex] = -1;
	currentIndex--;
}

int
mvStack::top()
{
	return data[currentIndex - 1];
}

bool
mvStack::empty()
{
	return currentIndex == 0;
}

bool
mvJsonObject::doesMemberExist(const char* member)
{
	for (int i = 0; i < members.size(); i++)
	{
		const char* name = members[i].name.c_str();

		if (strcmp(member, name) == 0)
			return true;
	}

	return false;
}

mvJsonMember&
mvJsonObject::operator[](int i)
{
	return members[i];
}

mvJsonObject&
mvJsonObject::operator[](const char* member)
{
	for (int i = 0; i < members.size(); i++)
	{
		const char* name = members[i].name.c_str();

		if (strcmp(member, name) == 0)
			return context->jsonObjects[members[i].index];
	}

	return context->jsonObjects[0];
}

mvJsonMember&
mvJsonObject::getMember(const char* member)
{
	for (int i = 0; i < members.size(); i++)
	{
		const char* name = members[i].name.c_str();

		if (strcmp(member, name) == 0)
			return members[i];
	}

	return members[0];
}

mvJsonMember::operator int()
{
	return atoi(context->primitiveValues[index].value.c_str());
}

mvJsonMember::operator mvU32()
{
	int value = atoi(context->primitiveValues[index].value.c_str());
	return (mvU32)value;
}

mvJsonMember::operator float()
{
	return atof(context->primitiveValues[index].value.c_str());
}

mvJsonMember::operator char* ()
{
	return (char*)context->primitiveValues[index].value.c_str();
}

mvJsonMember::operator mvJsonObject& ()
{
	return context->jsonObjects[index];
}

namespace mvImp {

	static std::string*
	_LoadExtensions(mvJsonContext& j, mvU32& size)
	{
		if (!j.doesMemberExist("extensionsUsed"))
			return nullptr;

		mvU32 extensionCount = j["extensionsUsed"].members.size();

		std::string* extensions = new std::string[extensionCount];

		for (int i = 0; i < extensionCount; i++)
		{
			//mvJsonObject& jExtension = j["extensionsUsed"][i];
			extensions[i] = j["extensionsUsed"][i];
			size++;
		}

		return extensions;
	}

	static mvGLTFAnimation*
	_LoadAnimations(mvJsonContext& j, mvU32& size)
	{
		if (!j.doesMemberExist("animations"))
			return nullptr;

		mvU32 animationCount = j["animations"].members.size();

		mvGLTFAnimation* animations = new mvGLTFAnimation[animationCount];

		for (int i = 0; i < animationCount; i++)
		{
			mvJsonObject& janimation = j["animations"][i];
			mvGLTFAnimation& animation = animations[i];

			if (janimation.doesMemberExist("name"))
			{
				animation.name = janimation.getMember("name");
			}

			if (janimation.doesMemberExist("samplers"))
			{
				mvU32 samplerCount = janimation["samplers"].members.size();
				animation.samplers = new mvGLTFAnimationSampler[samplerCount];
				animation.sampler_count = samplerCount;

				for (int i = 0; i < samplerCount; i++)
				{
					mvJsonObject& jsampler = janimation["samplers"][i];
					mvGLTFAnimationSampler& sampler = animation.samplers[i];

					if (jsampler.doesMemberExist("input"))
					{
						sampler.input = jsampler.getMember("input");
					}

					if (jsampler.doesMemberExist("output"))
					{
						sampler.output = jsampler.getMember("output");
					}

					if (jsampler.doesMemberExist("interpolation"))
					{
						sampler.interpolation = jsampler.getMember("interpolation");
					}
				}
			}

			if (janimation.doesMemberExist("channels"))
			{
				mvU32 channelCount = janimation["channels"].members.size();
				animation.channels = new mvGLTFAnimationChannel[channelCount];
				animation.channel_count = channelCount;

				for (int i = 0; i < channelCount; i++)
				{
					mvJsonObject& jchannel = janimation["channels"][i];
					mvGLTFAnimationChannel& channel = animation.channels[i];

					if (jchannel.doesMemberExist("sampler"))
					{
						channel.sampler = jchannel.getMember("sampler");
					}

					if (jchannel.doesMemberExist("target"))
					{

						mvJsonObject& jchanneltarget = jchannel.getMember("target");
						mvGLTFAnimationChannelTarget target{};

						if (jchanneltarget.doesMemberExist("node"))
						{
							channel.target.node = jchanneltarget.getMember("node");
						}

						if (jchanneltarget.doesMemberExist("path"))
						{
							channel.target.path = jchanneltarget.getMember("path");
						}

					}
				}
			}

			size++;
		}

		return animations;
	}

	static mvGLTFCamera*
	_LoadCameras(mvJsonContext& j, mvU32& size)
	{
		if (!j.doesMemberExist("cameras"))
			return nullptr;

		mvU32 cameraCount = j["cameras"].members.size();

		mvGLTFCamera* cameras = new mvGLTFCamera[cameraCount];

		for (int i = 0; i < cameraCount; i++)
		{
			mvJsonObject& jcamera = j["cameras"][i];
			mvGLTFCamera& camera = cameras[i];

			if (jcamera.doesMemberExist("name"))
			{
				camera.name = jcamera.getMember("name");
			}

			if (jcamera.doesMemberExist("type"))
			{

				std::string type = jcamera.getMember("type");
				if (type == "perspective")
				{
					camera.type = MV_IMP_PERSPECTIVE;
				}
				else
				{
					camera.type = MV_IMP_ORTHOGRAPHIC;
				}
			}

			if (jcamera.doesMemberExist("perspective"))
			{

				mvJsonObject perspective = jcamera["perspective"];

				if (perspective.doesMemberExist("aspectRatio"))
					camera.perspective.aspectRatio = perspective.getMember("aspectRatio");

				if (perspective.doesMemberExist("yfov"))
					camera.perspective.yfov = perspective.getMember("yfov");

				if (perspective.doesMemberExist("zfar"))
					camera.perspective.zfar = perspective.getMember("zfar");

				if (perspective.doesMemberExist("znear"))
					camera.perspective.znear = perspective.getMember("znear");

			}

			if (jcamera.doesMemberExist("orthographic"))
			{

				mvJsonObject orthographic = jcamera["orthographic"];

				if (orthographic.doesMemberExist("xmag"))
					camera.orthographic.xmag = orthographic.getMember("xmag");

				if (orthographic.doesMemberExist("ymag"))
					camera.orthographic.ymag = orthographic.getMember("ymag");

				if (orthographic.doesMemberExist("zfar"))
					camera.orthographic.zfar = orthographic.getMember("zfar");

				if (orthographic.doesMemberExist("znear"))
					camera.orthographic.znear = orthographic.getMember("znear");

			}

			size++;
		}

		return cameras;
	}

	static mvGLTFScene*
	_LoadScenes(mvJsonContext& j, mvU32& size)
	{

		if (!j.doesMemberExist("scenes"))
			return nullptr;

		mvU32 count = j["scenes"].members.size();
		mvGLTFScene* scenes = new mvGLTFScene[count];

		for (int i = 0; i < count; i++)
		{
			mvJsonObject& jscene = j["scenes"][0];
			mvGLTFScene& scene = scenes[size];

			if (jscene.doesMemberExist("nodes"))
			{
				mvU32 nodeCount = jscene["nodes"].members.size();
				scene.nodes = new mvU32[nodeCount];
				for (int j = 0; j < nodeCount; j++)
				{
					int node = jscene["nodes"][j];
					scene.nodes[scene.node_count] = node;
					scene.node_count++;
				}
			}

			size++;
		}

		return scenes;
	}

	static mvGLTFNode*
	_LoadNodes(mvJsonContext& j, mvU32& size)
	{
		if (!j.doesMemberExist("nodes"))
			return nullptr;

		mvU32 count = j["nodes"].members.size();
		mvGLTFNode* nodes = new mvGLTFNode[count];

		for (int i = 0; i < count; i++)
		{
			mvJsonObject& jnode = j["nodes"][i];
			mvGLTFNode& node = nodes[size];

			if (jnode.doesMemberExist("name"))
				node.name = jnode.getMember("name");

			if (jnode.doesMemberExist("mesh"))
				node.mesh_index = jnode.getMember("mesh");

			if (jnode.doesMemberExist("camera"))
				node.camera_index = jnode.getMember("camera");

			if (jnode.doesMemberExist("skin"))
				node.skin_index = jnode.getMember("skin");

			if (jnode.doesMemberExist("children"))
			{
				mvU32 childCount = jnode["children"].members.size();
				node.children = new mvU32[childCount];

				for (int j = 0; j < childCount; j++)
				{
					mvU32 child = jnode["children"][j];
					node.children[node.child_count] = child;
					node.child_count++;
				}
			}

			if (jnode.doesMemberExist("translation"))
			{
				mvU32 compCount = jnode["translation"].members.size();

				for (int j = 0; j < compCount; j++)
				{
					node.translation[j] = jnode["translation"][j];
				}
			}

			if (jnode.doesMemberExist("scale"))
			{
				mvU32 compCount = jnode["scale"].members.size();

				for (int j = 0; j < compCount; j++)
				{
					node.scale[j] = jnode["scale"][j];
				}
			}

			if (jnode.doesMemberExist("rotation"))
			{
				mvU32 compCount = jnode["rotation"].members.size();

				for (int j = 0; j < compCount; j++)
				{
					node.rotation[j] = jnode["rotation"][j];
				}
			}

			if (jnode.doesMemberExist("matrix"))
			{
				node.hadMatrix = true;
				mvU32 compCount = jnode["matrix"].members.size();

				for (int j = 0; j < compCount; j++)
				{
					node.matrix[j] = jnode["matrix"][j];
				}
			}

			size++;
		}

		return nodes;
	}

	static mvGLTFMesh*
	_LoadMeshes(mvJsonContext& j, mvU32& size)
	{
		if (!j.doesMemberExist("meshes"))
			return nullptr;

		mvU32 meshCount = j["meshes"].members.size();

		mvGLTFMesh* meshes = new mvGLTFMesh[meshCount];

		for (int i = 0; i < meshCount; i++)
		{
			mvJsonObject& jmesh = j["meshes"][i];
			mvGLTFMesh& mesh = meshes[i];

			if (jmesh.doesMemberExist("name"))
				mesh.name = jmesh.getMember("name");

			if (jmesh.doesMemberExist("weights"))
			{
				mesh.weights_count = jmesh["weights"].members.size();
				mesh.weights = new mvF32[mesh.weights_count];
				for (int j = 0; j < mesh.weights_count; j++)
				{
					mvJsonMember m = jmesh["weights"][j];
					//mvGLTFMeshPrimitive& primitive = mesh.primitives[j];
					mesh.weights[j] = m;
				}
			}

			if (jmesh.doesMemberExist("primitives"))
			{

				mesh.primitives_count = jmesh["primitives"].members.size();
				mesh.primitives = new mvGLTFMeshPrimitive[mesh.primitives_count];

				for (int j = 0; j < mesh.primitives_count; j++)
				{

					mvGLTFMeshPrimitive& primitive = mesh.primitives[j];

					mvJsonObject jprimitive = jmesh["primitives"][j];

					if (jprimitive.doesMemberExist("indices"))
						primitive.indices_index = jprimitive.getMember("indices");

					if (jprimitive.doesMemberExist("material"))
						primitive.material_index = jprimitive.getMember("material");

					if (jprimitive.doesMemberExist("attributes"))
					{
						mvJsonObject jattributes = jprimitive["attributes"];
						mvU32 attrCount = jattributes.members.size();
						primitive.attributes = new mvGLTFAttribute[attrCount];
						

						for (int k = 0; k < attrCount; k++)
						{
							mvJsonMember m = jattributes.members[k];
							
							primitive.attributes[primitive.attribute_count] = { m.name , (int)m };
							primitive.attribute_count++;
						}

					}

					if (jprimitive.doesMemberExist("targets"))
					{
						mvJsonObject jtargets = jprimitive["targets"];
						mvU32 targetCount = jtargets.members.size();
						primitive.targets = new mvGLTFMorphTarget[targetCount];

						for (int k = 0; k < targetCount; k++)
						{
							mvGLTFMorphTarget& target = primitive.targets[k];

							mvJsonObject jtarget = jtargets.members[k];
							
							mvU32 attrCount = jtarget.members.size();
							target.attributes = new mvGLTFAttribute[attrCount];
							for (int x = 0; x < attrCount; x++)
							{
								mvJsonMember jattribute = jtarget.members[x];
								target.attributes[target.attribute_count] = { jattribute.name , (int)jattribute };
								target.attribute_count++;
							}

							primitive.target_count++;
						}

					}


				}
			}

			size++;
		}

		return meshes;
	}

	static mvGLTFMaterial*
	_LoadMaterials(mvJsonContext& j, mvU32& size)
	{
		if (!j.doesMemberExist("materials"))
			return nullptr;

		mvU32 count = j["materials"].members.size();
		mvGLTFMaterial* materials = new mvGLTFMaterial[count];

		for (int i = 0; i < count; i++)
		{
			mvJsonObject& jmaterial = j["materials"][i];
			mvGLTFMaterial& material = materials[size];

			if (jmaterial.doesMemberExist("name"))
				material.name = jmaterial.getMember("name");

			if (jmaterial.doesMemberExist("alphaMode"))
			{
				std::string alphaMode = jmaterial.getMember("alphaMode");
				if (alphaMode == "OPAQUE")
					material.alphaMode = MV_ALPHA_MODE_OPAQUE;
				else if(alphaMode == "MASK")
					material.alphaMode = MV_ALPHA_MODE_MASK;
				else
					material.alphaMode = MV_ALPHA_MODE_BLEND;
			}

			if (jmaterial.doesMemberExist("pbrMetallicRoughness"))
			{
				material.pbrMetallicRoughness = true;

				if (jmaterial["pbrMetallicRoughness"].doesMemberExist("baseColorTexture"))
				{
					if (jmaterial["pbrMetallicRoughness"]["baseColorTexture"].doesMemberExist("index"))
						material.base_color_texture = jmaterial["pbrMetallicRoughness"]["baseColorTexture"].getMember("index");
				}

				if (jmaterial["pbrMetallicRoughness"].doesMemberExist("baseColorFactor"))
				{
					material.base_color_factor[0] = jmaterial["pbrMetallicRoughness"]["baseColorFactor"][0];
					material.base_color_factor[1] = jmaterial["pbrMetallicRoughness"]["baseColorFactor"][1];
					material.base_color_factor[2] = jmaterial["pbrMetallicRoughness"]["baseColorFactor"][2];
					material.base_color_factor[3] = jmaterial["pbrMetallicRoughness"]["baseColorFactor"][3];
				}

				if (jmaterial["pbrMetallicRoughness"].doesMemberExist("metallicRoughnessTexture"))
				{
					if (jmaterial["pbrMetallicRoughness"]["metallicRoughnessTexture"].doesMemberExist("index"))
						material.metallic_roughness_texture = jmaterial["pbrMetallicRoughness"]["metallicRoughnessTexture"].getMember("index");
				}

				if (jmaterial["pbrMetallicRoughness"].doesMemberExist("metallicFactor"))
					material.metallic_factor = jmaterial["pbrMetallicRoughness"].getMember("metallicFactor");

				if (jmaterial["pbrMetallicRoughness"].doesMemberExist("roughnessFactor"))
					material.roughness_factor = jmaterial["pbrMetallicRoughness"].getMember("roughnessFactor");
			}

			if (jmaterial.doesMemberExist("normalTexture"))
			{
				if (jmaterial["normalTexture"].doesMemberExist("index"))
					material.normal_texture = jmaterial["normalTexture"].getMember("index");

				if (jmaterial["normalTexture"].doesMemberExist("scale"))
					material.normal_texture_scale = jmaterial["normalTexture"].getMember("scale");
			}

			if (jmaterial.doesMemberExist("occlusionTexture"))
			{
				if (jmaterial["occlusionTexture"].doesMemberExist("index"))
					material.occlusion_texture = jmaterial["occlusionTexture"].getMember("index");

				if (jmaterial["occlusionTexture"].doesMemberExist("scale"))
					material.occlusion_texture_strength = jmaterial["occlusionTexture"].getMember("strength");
			}

			if (jmaterial.doesMemberExist("emissiveTexture"))
			{
				if (jmaterial["emissiveTexture"].doesMemberExist("index"))
					material.emissive_texture = jmaterial["emissiveTexture"].getMember("index");
			}

			if (jmaterial.doesMemberExist("emissiveFactor"))
			{

				material.emissive_factor[0] = jmaterial["emissiveFactor"][0];
				material.emissive_factor[1] = jmaterial["emissiveFactor"][1];
				material.emissive_factor[2] = jmaterial["emissiveFactor"][2];
			}

			if (jmaterial.doesMemberExist("doubleSided"))
				material.double_sided = jmaterial.getMember("doubleSided")[0] == 't';

			if (jmaterial.doesMemberExist("alphaCutoff"))
				material.alphaCutoff = jmaterial.getMember("alphaCutoff");

			if (jmaterial.doesMemberExist("extensions"))
			{
				if (jmaterial["extensions"].doesMemberExist("KHR_materials_clearcoat"))
				{
					material.clearcoat_extension = true;

					if (jmaterial["extensions"]["KHR_materials_clearcoat"].doesMemberExist("clearcoatFactor"))
						material.clearcoat_factor = jmaterial["extensions"]["KHR_materials_clearcoat"].getMember("clearcoatFactor");
					if (jmaterial["extensions"]["KHR_materials_clearcoat"].doesMemberExist("clearcoatRoughnessFactor"))
						material.clearcoat_roughness_factor = jmaterial["extensions"]["KHR_materials_clearcoat"].getMember("clearcoatRoughnessFactor");
					if (jmaterial["extensions"]["KHR_materials_clearcoat"].doesMemberExist("clearcoatTexture"))
					{
						if (jmaterial["extensions"]["KHR_materials_clearcoat"]["clearcoatTexture"].doesMemberExist("index"))
							material.clearcoat_texture = jmaterial["extensions"]["KHR_materials_clearcoat"]["clearcoatTexture"].getMember("index");
						if (jmaterial["extensions"]["KHR_materials_clearcoat"]["clearcoatTexture"].doesMemberExist("scale"))
							material.clearcoat_normal_texture_scale = jmaterial["extensions"]["KHR_materials_clearcoat"]["clearcoatTexture"].getMember("scale");
					}
					if (jmaterial["extensions"]["KHR_materials_clearcoat"].doesMemberExist("clearcoatRoughnessTexture"))
					{
						if (jmaterial["extensions"]["KHR_materials_clearcoat"]["clearcoatRoughnessTexture"].doesMemberExist("index"))
							material.clearcoat_roughness_texture = jmaterial["extensions"]["KHR_materials_clearcoat"]["clearcoatRoughnessTexture"].getMember("index");
					}
					if (jmaterial["extensions"]["KHR_materials_clearcoat"].doesMemberExist("clearcoatNormalTexture"))
					{
						if (jmaterial["extensions"]["KHR_materials_clearcoat"]["clearcoatNormalTexture"].doesMemberExist("index"))
							material.clearcoat_normal_texture = jmaterial["extensions"]["KHR_materials_clearcoat"]["clearcoatNormalTexture"].getMember("index");
					}
				}

				if (jmaterial["pbrMetallicRoughness"].doesMemberExist("baseColorFactor"))
				{
					material.base_color_factor[0] = jmaterial["pbrMetallicRoughness"]["baseColorFactor"][0];
					material.base_color_factor[1] = jmaterial["pbrMetallicRoughness"]["baseColorFactor"][1];
					material.base_color_factor[2] = jmaterial["pbrMetallicRoughness"]["baseColorFactor"][2];
					material.base_color_factor[3] = jmaterial["pbrMetallicRoughness"]["baseColorFactor"][3];
				}

				if (jmaterial["pbrMetallicRoughness"].doesMemberExist("metallicRoughnessTexture"))
				{
					if (jmaterial["pbrMetallicRoughness"]["metallicRoughnessTexture"].doesMemberExist("index"))
						material.metallic_roughness_texture = jmaterial["pbrMetallicRoughness"]["metallicRoughnessTexture"].getMember("index");
				}

				if (jmaterial["pbrMetallicRoughness"].doesMemberExist("metallicFactor"))
					material.metallic_factor = jmaterial["pbrMetallicRoughness"].getMember("metallicFactor");

				if (jmaterial["pbrMetallicRoughness"].doesMemberExist("roughnessFactor"))
					material.roughness_factor = jmaterial["pbrMetallicRoughness"].getMember("roughnessFactor");
			}

			size++;
		}

		return materials;
	}

	static mvGLTFTexture*
	_LoadTextures(mvJsonContext& j, mvU32& size)
	{
		if (!j.doesMemberExist("textures"))
			return nullptr;

		mvU32 count = j["textures"].members.size();
		mvGLTFTexture* textures = new mvGLTFTexture[count];

		for (int i = 0; i < count; i++)
		{
			mvJsonObject& jtexture = j["textures"][i];
			mvGLTFTexture& texture = textures[size];

			if (jtexture.doesMemberExist("sampler"))
				texture.sampler_index = jtexture.getMember("sampler");

			if (jtexture.doesMemberExist("source"))
				texture.image_index = jtexture.getMember("source");

			if (jtexture.doesMemberExist("name"))
				texture.name = jtexture.getMember("name");

			size++;
		}

		return textures;
	}

	static mvGLTFSampler*
	_LoadSamplers(mvJsonContext& j, mvU32& size)
	{
		if (!j.doesMemberExist("samplers"))
			return nullptr;

		mvU32 count = j["samplers"].members.size();
		mvGLTFSampler* samplers = new mvGLTFSampler[count];

		for (int i = 0; i < count; i++)
		{
			mvJsonObject& jsampler = j["samplers"][i];
			mvGLTFSampler& sampler = samplers[size];

			if (jsampler.doesMemberExist("name"))
				sampler.name = jsampler.getMember("name");

			if (jsampler.doesMemberExist("magFilter"))
				sampler.mag_filter = jsampler.getMember("magFilter");

			if (jsampler.doesMemberExist("minFilter"))
				sampler.min_filter = jsampler.getMember("minFilter");

			if (jsampler.doesMemberExist("wrapS"))
				sampler.wrap_s = jsampler.getMember("wrapS");

			if (jsampler.doesMemberExist("wrapT"))
				sampler.wrap_t = jsampler.getMember("wrapT");

			size++;
		}

		return samplers;
	}

	static mvGLTFImage*
	_LoadImages(mvJsonContext& j, mvU32& size)
	{
		if (!j.doesMemberExist("images"))
			return nullptr;

		mvU32 count = j["images"].members.size();
		mvGLTFImage* images = new mvGLTFImage[count];

		for (int i = 0; i < count; i++)
		{
			mvJsonObject& jimage = j["images"][i];
			mvGLTFImage& image = images[size];

			if (jimage.doesMemberExist("uri"))
				image.uri = jimage.getMember("uri");

			if (jimage.doesMemberExist("mimeType"))
				image.mimeType = jimage.getMember("mimeType");

			if (jimage.doesMemberExist("bufferView"))
				image.buffer_view_index = jimage.getMember("bufferView");

			size++;
		}

		return images;
	}

	static mvGLTFBuffer*
	_LoadBuffers(mvJsonContext& j, mvU32& size)
	{
		if (!j.doesMemberExist("buffers"))
			return nullptr;

		mvU32 count = j["buffers"].members.size();
		mvGLTFBuffer* buffers = new mvGLTFBuffer[count];

		for (int i = 0; i < count; i++)
		{
			mvJsonObject& jbuffer = j["buffers"][i];
			mvGLTFBuffer& buffer = buffers[size];

			if (jbuffer.doesMemberExist("uri"))
				buffer.uri = jbuffer.getMember("uri");

			if (jbuffer.doesMemberExist("byteLength"))
				buffer.byte_length = (int)jbuffer.getMember("byteLength");

			size++;
		}

		return buffers;
	}

	static mvGLTFBufferView*
	_LoadBufferViews(mvJsonContext& j, mvU32& size)
	{
		if (!j.doesMemberExist("bufferViews"))
			return nullptr;

		mvU32 count = j["bufferViews"].members.size();
		mvGLTFBufferView* bufferviews = new mvGLTFBufferView[count];

		for (int i = 0; i < count; i++)
		{

			mvJsonObject& jbufferview = j["bufferViews"][i];
			mvGLTFBufferView& bufferview = bufferviews[size];

			if (jbufferview.doesMemberExist("name"))
				bufferview.name = jbufferview.getMember("name");

			if (jbufferview.doesMemberExist("buffer"))
				bufferview.buffer_index = jbufferview.getMember("buffer");

			if (jbufferview.doesMemberExist("byteOffset"))
				bufferview.byte_offset = jbufferview.getMember("byteOffset");

			if (jbufferview.doesMemberExist("byteLength"))
				bufferview.byte_length = jbufferview.getMember("byteLength");

			if (jbufferview.doesMemberExist("byteStride"))
				bufferview.byte_stride = jbufferview.getMember("byteStride");

			size++;
		}

		return bufferviews;
	}

	static mvGLTFAccessor*
	_LoadAccessors(mvJsonContext& j, mvU32& size)
	{
		if (!j.doesMemberExist("accessors"))
			return nullptr;

		mvU32 count = j["accessors"].members.size();
		mvGLTFAccessor* accessors = new mvGLTFAccessor[count];

		for (int i = 0; i < count; i++)
		{
			mvJsonObject& jaccessor = j["accessors"][i];
			mvGLTFAccessor& accessor = accessors[i];

			if (jaccessor.doesMemberExist("name"))
			{
				//strcpy(accessor.name, jaccessor.getMember("name"));
				accessor.name = jaccessor.getMember("name");
			}

			if (jaccessor.doesMemberExist("byteOffset"))
				accessor.byteOffset = jaccessor.getMember("byteOffset");

			if (jaccessor.doesMemberExist("count"))
				accessor.count = jaccessor.getMember("count");

			if (jaccessor.doesMemberExist("componentType"))
				accessor.component_type = (mvGLTFComponentType)(int)jaccessor.getMember("componentType");

			if (jaccessor.doesMemberExist("bufferView"))
				accessor.buffer_view_index = jaccessor.getMember("bufferView");

			if (jaccessor.doesMemberExist("type"))
			{
				std::string strtype = jaccessor.getMember("type");
				if (strtype == "SCALAR")
					accessor.type = MV_IMP_SCALAR;

				else if (strtype == "VEC2")
					accessor.type = MV_IMP_VEC2;

				else if (strtype == "VEC3")
					accessor.type = MV_IMP_VEC3;

				else if (strtype == "VEC4")
					accessor.type = MV_IMP_VEC4;

				else if (strtype == "MAT2")
					accessor.type = MV_IMP_MAT2;

				else if (strtype == "MAT3")
					accessor.type = MV_IMP_MAT3;

				else if (strtype == "MAT4")
					accessor.type = MV_IMP_MAT4;
			}

			if (jaccessor.doesMemberExist("max"))
			{

				mvU32 min_count = jaccessor["max"].members.size();
				for (mvU32 min_entry = 0u; min_entry < min_count; min_entry++)
				{
					accessor.maxes[min_entry] = jaccessor["max"][min_entry];
				}
			}

			if (jaccessor.doesMemberExist("min"))
			{

				mvU32 min_count = jaccessor["min"].members.size();
				for (mvU32 min_entry = 0u; min_entry < min_count; min_entry++)
				{
					accessor.mins[min_entry] = jaccessor["min"][min_entry];
				}
			}

			size++;
		}

		return accessors;
	}

	static mvGLTFSkin*
	_LoadSkins(mvJsonContext& j, mvU32& size)
	{
		if (!j.doesMemberExist("skins"))
			return nullptr;

		mvU32 count = j["skins"].members.size();
		mvGLTFSkin* skins = new mvGLTFSkin[count];

		for (int i = 0; i < count; i++)
		{
			mvJsonObject& jnode = j["skins"][i];
			mvGLTFSkin& skin = skins[size];

			if (jnode.doesMemberExist("name"))
				skin.name = jnode.getMember("name");

			if (jnode.doesMemberExist("inverseBindMatrices"))
				skin.inverseBindMatrices = jnode.getMember("inverseBindMatrices");

			if (jnode.doesMemberExist("skeleton"))
				skin.skeleton = jnode.getMember("skeleton");

			if (jnode.doesMemberExist("joints"))
			{
				mvU32 childCount = jnode["joints"].members.size();
				skin.joints = new mvU32[childCount];

				for (int j = 0; j < childCount; j++)
				{
					mvU32 child = jnode["joints"][j];
					skin.joints[skin.joints_count] = child;
					skin.joints_count++;
				}
			}

			size++;
		}

		return skins;
	}

	static char*
	_ReadFile(const char* file, mvU32& size, const char* mode)
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

	mvU32 dataSize = 0u;
	char* data = (char*)mvImp::_ReadFile(file, dataSize, "rb");

	mvU32 magic = *(mvU32*)&data[0];
	mvU32 version = *(mvU32*)&data[4];
	mvU32 length = *(mvU32*)&data[8];

	mvU32 chunkLength = *(mvU32*)&data[12];
	mvU32 chunkType = *(mvU32*)&data[16];
	char* chunkData = &data[20];

	if (dataSize == 0u)
	{
		assert(false && "File not found.");
		return model;
	}

	mvJsonContext& context = *ParseJSON(chunkData, chunkLength);

	if (context.doesMemberExist("scene"))
	{
		for (int i = 0; i < context.jsonObjects[1].members.size(); i++)
		{
			const char* name = context.jsonObjects[1].members[i].name.c_str();

			if (strcmp("scene", name) == 0)
			{
				int index = context.jsonObjects[1].members[i].index;
				mvJsonValue value = context.primitiveValues[index];
				model.scene = stoi(value.value);
			}
		}
	}

	model.scenes = mvImp::_LoadScenes(context, model.scene_count);
	model.nodes = mvImp::_LoadNodes(context, model.node_count);
	model.materials = mvImp::_LoadMaterials(context, model.material_count);
	model.meshes = mvImp::_LoadMeshes(context, model.mesh_count);
	model.textures = mvImp::_LoadTextures(context, model.texture_count);
	model.samplers = mvImp::_LoadSamplers(context, model.sampler_count);
	model.images = mvImp::_LoadImages(context, model.image_count);
	model.buffers = mvImp::_LoadBuffers(context, model.buffer_count);
	model.bufferviews = mvImp::_LoadBufferViews(context, model.bufferview_count);
	model.accessors = mvImp::_LoadAccessors(context, model.accessor_count);
	model.cameras = mvImp::_LoadCameras(context, model.camera_count);
	model.animations = mvImp::_LoadAnimations(context, model.animation_count);
	model.extensions = mvImp::_LoadExtensions(context, model.extension_count);
	model.skins = mvImp::_LoadSkins(context, model.skin_count);

	if (chunkLength + 20 != length)
	{
		mvU32 datachunkLength = *(mvU32*)&data[20 + chunkLength];
		mvU32 datachunkType = *(mvU32*)&data[24 + chunkLength];
		char* datachunkData = &data[28 + chunkLength];

		model.buffers[0].data.resize(model.buffers[0].byte_length);
		memcpy(model.buffers[0].data.data(), datachunkData, model.buffers[0].byte_length);
		int a = 6;
	}

	for (mvU32 i = 0; i < model.image_count; i++)
	{
		mvGLTFImage& image = model.images[i];

		if (image.buffer_view_index > -1)
		{
			image.embedded = true;
			//image.data = model.buffers
			mvGLTFBufferView bufferView = model.bufferviews[image.buffer_view_index];
			char* bufferRawData = (char*)model.buffers[bufferView.buffer_index].data.data();
			char* bufferRawSection = &bufferRawData[bufferView.byte_offset]; // start of buffer section
			image.data.resize(bufferView.byte_length);
			memcpy(image.data.data(), bufferRawSection, bufferView.byte_length);
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

	for (mvU32 i = 0; i < model.buffer_count; i++)
	{
		mvGLTFBuffer& buffer = model.buffers[i];

		if (!buffer.data.empty())
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
			mvU32 dataSize = 0u;
			void* data = mvImp::_ReadFile(combinedFile.c_str(), dataSize, "rb");
			buffer.data.resize(dataSize);
			memcpy(buffer.data.data(), data, dataSize);
		}

	}

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

	mvU32 dataSize = 0u;
	char* data = mvImp::_ReadFile(file, dataSize, "rb");

	if (dataSize == 0u)
	{
		assert(false && "File not found.");
		return model;
	}

	mvJsonContext& context = *ParseJSON(data, dataSize);
	delete[] data;

	if (context.doesMemberExist("scene"))
	{
		for (int i = 0; i < context.jsonObjects[1].members.size(); i++)
		{
			const char* name = context.jsonObjects[1].members[i].name.c_str();

			if (strcmp("scene", name) == 0)
			{
				int index = context.jsonObjects[1].members[i].index;
				mvJsonValue value = context.primitiveValues[index];
				model.scene = stoi(value.value);
			}
		}
	}

	model.scenes = mvImp::_LoadScenes(context, model.scene_count);
	model.nodes = mvImp::_LoadNodes(context, model.node_count);
	model.materials = mvImp::_LoadMaterials(context, model.material_count);
	model.meshes = mvImp::_LoadMeshes(context, model.mesh_count);
	model.textures = mvImp::_LoadTextures(context, model.texture_count);
	model.samplers = mvImp::_LoadSamplers(context, model.sampler_count);
	model.images = mvImp::_LoadImages(context, model.image_count);
	model.buffers = mvImp::_LoadBuffers(context, model.buffer_count);
	model.bufferviews = mvImp::_LoadBufferViews(context, model.bufferview_count);
	model.accessors = mvImp::_LoadAccessors(context, model.accessor_count);
	model.cameras = mvImp::_LoadCameras(context, model.camera_count);
	model.animations = mvImp::_LoadAnimations(context, model.animation_count);
	model.extensions = mvImp::_LoadExtensions(context, model.extension_count);
	model.skins = mvImp::_LoadSkins(context, model.skin_count);

	for (mvU32 i = 0; i < model.image_count; i++)
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

	for (mvU32 i = 0; i < model.buffer_count; i++)
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
			mvU32 dataSize = 0u;
			void* data = mvImp::_ReadFile(combinedFile.c_str(), dataSize, "rb");
			buffer.data.resize(dataSize);
			memcpy(buffer.data.data(), data, dataSize);
		}

	}

	return model;
}

void
mvCleanupGLTF(mvGLTFModel& model)
{

	for (mvU32 i = 0; i < model.mesh_count; i++)
	{
		for (mvU32 j = 0; j < model.meshes[i].primitives_count; j++)
		{
			
			for (mvU32 k = 0; k < model.meshes[i].primitives[j].target_count; k++)
			{
				delete[] model.meshes[i].primitives[j].targets[k].attributes;
			}
			delete[] model.meshes[i].primitives[j].targets;
			delete[] model.meshes[i].primitives[j].attributes;
		}
		delete[] model.meshes[i].weights;
		delete[] model.meshes[i].primitives;
	}


	for (mvU32 i = 0; i < model.node_count; i++)
	{
		if (model.nodes[i].children)
			delete[] model.nodes[i].children;
	}

	for (mvU32 i = 0; i < model.scene_count; i++)
	{
		if (model.scenes[i].nodes)
			delete[] model.scenes[i].nodes;
	}

	for (mvU32 i = 0; i < model.animation_count; i++)
	{
		if (model.animations[i].sampler_count > 0)
			delete[] model.animations[i].samplers;
		if (model.animations[i].channel_count > 0)
			delete[] model.animations[i].channels;
	}

	for (mvU32 i = 0; i < model.skin_count; i++)
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