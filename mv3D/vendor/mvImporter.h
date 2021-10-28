/*
 mvImporter, v0.0.1 (WIP)
   * no dependencies
   * assumes GLTF file is correct
   * assumes GLTF binary data is in .bin file (just for now)
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

#ifdef MV_IMPORTER_IMPLEMENTATION
#include <assert.h>
#include <stdio.h>
#include <vector>
#endif // MV_IMPORTER_IMPLEMENTATION

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
struct mvGLTFNode;       // GLTF -> "nodes"
struct mvGLTFScene;      // GLTF -> "scenes"
struct mvGLTFMesh;       // GLTF -> "meshes"
struct mvGLTFMaterial;   // GLTF -> "materials"
struct mvGLTFTexture;    // GLTF -> "textures"
struct mvGLTFSampler;    // GLTF -> "samplers"
struct mvGLTFImage;      // GLTF -> "images"
struct mvGLTFBuffer;     // GLTF -> "buffers"
struct mvGLTFBufferView; // GLTF -> "bufferViews"
struct mvGLTFAccessor;   // GLTF -> "accessors
struct mvGLTFAttribute;  // GLTF -> "meshes" > "primitives" > "attributes"
struct mvGLTFModel;      // contains arrays of the above and counts

//-----------------------------------------------------------------------------
// mvImporter End-User API
//-----------------------------------------------------------------------------

MV_IMPORTER_API mvGLTFModel mvLoadGLTF(const char* root, const char* file);
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
	char                name[1024];
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
	char  name[1024];
	mvS32 image_index   = -1;
	mvS32 sampler_index = -1;
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
	char uri[1024];
};

struct mvGLTFBuffer
{
	mvU32 byte_length = 0u;
	char  uri[1024];
	char* data = nullptr;
};

struct mvGLTFBufferView
{
	char  name[1024];
	mvS32 buffer_index = -1;
	mvS32 byte_offset  =  0;
	mvS32 byte_length  = -1;
	mvS32 byte_stride  = -1;
};

struct mvGLTFMesh
{
	char             name[1024];
	mvS32            indices_index  = -1; // accessor index
	mvS32            material_index = -1;
	mvGLTFPrimMode   mode = MV_IMP_TRIANGLES;
	mvGLTFAttribute* attributes = nullptr;
	mvU32            attribute_count = 0u;
};

struct mvGLTFMaterial
{
	char   name[1024];
	mvS32  base_color_texture         = -1;
	mvS32  metallic_roughness_texture = -1;
	mvS32  normal_texture             = -1;
	mvS32  occlusion_texture          = -1;
	mvS32  emissive_texture           = -1;
	mvF32  normal_texture_scale       = 0.8f;
	mvF32  occlusion_texture_strength = 0.9f;
	mvF32  metallic_factor            = 1.0f;
	mvF32  roughness_factor           = 0.0f;
	mvF32  base_color_factor[4]       = { 1.0f, 1.0f, 1.0f, 1.0f };
	mvF32  emissive_factor[3]         = { 0.0f, 0.0f, 0.0f };
	bool   double_sided               = false;
};

struct mvGLTFNode
{
	char        name[1024];
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
};

struct mvGLTFScene
{
	mvU32* nodes = nullptr;
	mvU32  node_count = 0u;
};

struct mvGLTFModel
{
	char              root[1024];
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
	void push (int value);
	void pop  ();
	int  top  ();
	bool empty();

	int currentIndex = 0;
	int data[1024];
};

struct mvToken
{
	char        value[256];
	mvTokenType type = MV_JSON_NONE;
};

struct mvJsonValue
{
	char value[256];
};

struct mvJsonMember
{
	char       name[256];
	mvJsonType type = MV_JSON_TYPE_NONE;
	int        index = -1;
	mvJsonContext* context = nullptr;

	operator char*         ();
	operator int           ();
	operator mvU32         ();
	operator float         ();
	operator mvJsonObject& ();
};

struct mvJsonObject
{
	mvJsonType                type = MV_JSON_TYPE_NONE;
	std::vector<mvJsonMember> members;
	mvJsonContext*            context = nullptr;

	mvJsonMember& getMember      (const char* member);
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

static void
ParseForTokens(char* rawData, std::vector<mvToken>& tokens)
{
	int currentPos = 0u;
	char currentChar = rawData[currentPos];
	char basicTokens[] = { '{', '}', '[', ']', ':', ',' };

	bool inString = false;
	char buffer[256];
	int  bufferPos = 0;

	while (currentChar != 0)
	{
		bool tokenFound = false;
		for (int i = 0; i < 6; i++)
		{
			if (currentChar == basicTokens[i])
			{
				if (bufferPos != 0)
				{
					mvToken primitivetoken{};
					primitivetoken.type = MV_JSON_PRIMITIVE;
					for (int i = 0; i < bufferPos; i++)
					{
						primitivetoken.value[i] = buffer[i];
					}
					tokens.push_back(primitivetoken);
					bufferPos = 0;
					buffer[0] = 0;
				}

				mvToken token{};
				if (currentChar == '{') token.type = MV_JSON_LEFT_BRACE;
				else if (currentChar == '}') token.type = MV_JSON_RIGHT_BRACE;
				else if (currentChar == '[') token.type = MV_JSON_LEFT_BRACKET;
				else if (currentChar == ']') token.type = MV_JSON_RIGHT_BRACKET;
				else if (currentChar == ',') token.type = MV_JSON_COMMA;
				else if (currentChar == ':') token.type = MV_JSON_COLON;

				token.value[0] = currentChar;
				tokens.push_back(token);
				tokenFound = true;
				break;
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
					for (int i = 0; i < bufferPos; i++)
					{
						token.value[i] = buffer[i];
					}

					if (rawData[currentPos + 1] == ':')
						token.type = MV_JSON_MEMBER;
					else
						token.type = MV_JSON_STRING;
					tokens.push_back(token);
					tokenFound = true;
					inString = false;
					bufferPos = 0;
					buffer[0] = 0;
				}
				else
				{
					inString = true;
					bufferPos = 0;
					buffer[0] = 0;
					tokenFound = true;
				}
			}
			else if (inString)
			{
				tokenFound = true;
				buffer[bufferPos] = currentChar;
				bufferPos++;
			}
		}

		// primitives
		if (!tokenFound)
		{
			buffer[bufferPos] = currentChar;
			bufferPos++;
		}

		currentPos++;
		currentChar = rawData[currentPos];
	}

}

static void
RemoveWhiteSpace(char* rawData, char* spacesRemoved)
{
	int currentPos = 0;
	int newCursor = 0;
	bool insideString = false;

	char currentChar = rawData[currentPos];

	while (currentChar != 0)
	{


		if (currentChar == '"' && insideString)
			insideString = false;
		else if (currentChar == '"')
			insideString = true;

		if (currentChar == ' ' || currentChar == '\n'
			|| currentChar == '\r' || currentChar == '\t' && !insideString)
		{
			currentPos++;
		}
		else
		{
			spacesRemoved[newCursor] = rawData[currentPos];
			currentPos++;
			newCursor++;
		}

		currentChar = rawData[currentPos];
	}

	spacesRemoved[newCursor] = 0;

}

static mvJsonContext*
ParseJSON(char* rawData, int size)
{
	mvJsonContext* ptrContext = new mvJsonContext();
	mvJsonContext& context = *ptrContext;
	mvStack jsonObjectStack;
	mvStack jsonMemberStack;

	char* spacesRemoved = new char[size];
	RemoveWhiteSpace(rawData, spacesRemoved);

	std::vector<mvToken> tokens;
	ParseForTokens(spacesRemoved, tokens);

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

		if (i >= tokens.size())
			break;

		switch (tokens[i].type)
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
			strcpy(member.name, tokens[i].value);
			mvTokenType valueType = tokens[i + 2].type;
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
				strcpy(context.primitiveValues.back().value, tokens[i].value);
				int memberId = jsonMemberStack.top();
				parent.members[memberId].index = valueId;
				jsonMemberStack.pop();
			}
			else if (parent.type == MV_JSON_TYPE_ARRAY)
			{

				int valueId = context.primitiveValues.size();
				context.primitiveValues.push_back({});
				strcpy(context.primitiveValues.back().value, tokens[i].value);

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
				strcpy(context.primitiveValues.back().value, tokens[i].value);
				int memberId = jsonMemberStack.top();
				parent.members[memberId].index = valueId;
				jsonMemberStack.pop();
			}

			else if (parent.type == MV_JSON_TYPE_ARRAY)
			{

				int valueId = context.primitiveValues.size();
				context.primitiveValues.push_back({});
				strcpy(context.primitiveValues.back().value, tokens[i].value);

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
		const char* name = jsonObjects[1].members[i].name;

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
		const char* name = jsonObjects[1].members[i].name;

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
		const char* name = members[i].name;

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
		const char* name = members[i].name;

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
		const char* name = members[i].name;

		if (strcmp(member, name) == 0)
			return members[i];
	}

	return members[0];
}

mvJsonMember::operator int()
{
	return atoi(context->primitiveValues[index].value);
}

mvJsonMember::operator mvU32()
{
	int value = atoi(context->primitiveValues[index].value);
	return (mvU32)value;
}

mvJsonMember::operator float()
{
	return atof(context->primitiveValues[index].value);
}

mvJsonMember::operator char* ()
{
	return context->primitiveValues[index].value;
}

mvJsonMember::operator mvJsonObject& ()
{
	return context->jsonObjects[index];
}

namespace mvImp {

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
				strcpy(node.name, jnode.getMember("name"));

			if (jnode.doesMemberExist("mesh"))
				node.mesh_index = jnode.getMember("mesh");

			if (jnode.doesMemberExist("skin"))
				node.skin_index = jnode.getMember("skin");

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

		mvU32 count = j["meshes"].members.size();
		mvGLTFMesh* meshes = new mvGLTFMesh[count];

		for (int i = 0; i < count; i++)
		{
			mvJsonObject& jmesh = j["meshes"][i];
			mvGLTFMesh& mesh = meshes[size];

			if (jmesh.doesMemberExist("name"))
				strcpy(mesh.name, jmesh.getMember("name"));

			if (jmesh.doesMemberExist("primitives"))
			{
				mvJsonObject jprimitive = jmesh["primitives"][0];

				if (jprimitive.doesMemberExist("indices"))
					mesh.indices_index = jprimitive.getMember("indices");

				if (jprimitive.doesMemberExist("material"))
					mesh.material_index = jprimitive.getMember("material");

				if (jprimitive.doesMemberExist("attributes"))
				{
					mvU32 attrCount = jprimitive["attributes"].members.size();
					mesh.attributes = new mvGLTFAttribute[attrCount];

					if (jprimitive["attributes"].doesMemberExist("POSITION"))
					{
						mesh.attributes[mesh.attribute_count] = { MV_IMP_POSITION , jprimitive["attributes"].getMember("POSITION") };
						mesh.attribute_count++;
					}


					if (jprimitive["attributes"].doesMemberExist("TANGENT"))
					{
						mesh.attributes[mesh.attribute_count] = { MV_IMP_TANGENT , jprimitive["attributes"].getMember("TANGENT") };
						mesh.attribute_count++;
					}

					if (jprimitive["attributes"].doesMemberExist("NORMAL"))
					{
						mesh.attributes[mesh.attribute_count] = { MV_IMP_NORMAL , jprimitive["attributes"].getMember("NORMAL") };
						mesh.attribute_count++;
					}

					if (jprimitive["attributes"].doesMemberExist("TEXCOORD_0"))
					{
						mesh.attributes[mesh.attribute_count] = { MV_IMP_TEXTCOORD , jprimitive["attributes"].getMember("TEXCOORD_0") };
						mesh.attribute_count++;
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
				strcpy(material.name, jmaterial.getMember("name"));

			if (jmaterial.doesMemberExist("pbrMetallicRoughness"))
			{
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
				strcpy(texture.name, jtexture.getMember("name"));

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
				strcpy(image.uri, jimage.getMember("uri"));

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
				strcpy(buffer.uri, jbuffer.getMember("uri"));

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
				strcpy(bufferview.name, jbufferview.getMember("name"));

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
			mvGLTFAccessor& accessor = accessors[size];

			if (jaccessor.doesMemberExist("name"))
			{
				strcpy(accessor.name, jaccessor.getMember("name"));
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
			}

			if (jaccessor.doesMemberExist("max"))
			{
				mvU32 max_count = jaccessor["max"].members.size();
				for (mvU32 max_entry = 0u; max_entry < max_count; max_entry++)
				{
					accessor.maxes[max_entry] = jaccessor["max"][max_entry];
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

mvGLTFModel
mvLoadGLTF(const char* root, const char* file)
{

	mvGLTFModel model{};

	strcpy(model.root, root);

	mvU32 dataSize = 0u;
	char* data = mvImp::_ReadFile(file, dataSize, "rb");

	if (dataSize == 0u)
	{
		assert(false && "File not found.");
		return model;
	}

	mvJsonContext& context = *ParseJSON(data, dataSize);
	delete[] data;

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

	for (mvU32 i = 0; i < model.buffer_count; i++)
	{
		mvGLTFBuffer& buffer = model.buffers[i];

		//std::string combinedFile = model.rootDirectory + buffer.uri;

		char combinedFile[1024];
		strcpy(combinedFile, model.root);
		strcat(combinedFile, buffer.uri);

		mvU32 dataSize = 0u;
		buffer.data = mvImp::_ReadFile(combinedFile, dataSize, "rb");

	}

	return model;
}

void 
mvCleanupGLTF(mvGLTFModel& model)
{

	for (mvU32 i = 0; i < model.mesh_count; i++)
		delete[] model.meshes[i].attributes;

	for (mvU32 i = 0; i < model.node_count; i++)
	{
		if(model.nodes[i].children)
			delete[] model.nodes[i].children;
	}

	for (mvU32 i = 0; i < model.scene_count; i++)
	{
		if(model.scenes[i].nodes)
			delete[] model.scenes[i].nodes;
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
}
#endif // MV_IMPORTER_IMPLEMENTATION