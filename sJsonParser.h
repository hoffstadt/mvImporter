/*
   sJsonParser, v0.1 (WIP)
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
   #include "sJsonParser.h"
*/

#ifndef SEMPER_JSON_H
#define SEMPER_JSON_H

#ifndef S_JSON_MAX_NAME_LENGTH
#define S_JSON_MAX_NAME_LENGTH 256
#endif

#include <assert.h>
#include <stdlib.h>
#include <string.h> // memcpy, strcmp

// forward declarations
struct sJsonObject;
typedef int sJsonType;

// public API
namespace Semper
{
	sJsonObject* parse_json(char* rawData, int size);
}

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
	inline int      asInt()    { assert(type == S_JSON_TYPE_NUMBER); return (int)strtod(value, nullptr);}
	inline unsigned asUInt()   { assert(type == S_JSON_TYPE_NUMBER); return (unsigned)strtod(value, nullptr);}
	inline float    asFloat()  { assert(type == S_JSON_TYPE_NUMBER); return (float)asDouble();}
	inline double   asDouble() { assert(type == S_JSON_TYPE_NUMBER); return strtod(value, nullptr);}
	inline char*    asString() { assert(type == S_JSON_TYPE_STRING); return value;}
	inline bool    	asBool()   { assert(type == S_JSON_TYPE_BOOL);   return value[0] == 't';}
	
	// cast array values
	inline void asIntArray   (int*      out, int size) { assert(type == S_JSON_TYPE_ARRAY); assert(out); if(size > childCount) size = childCount; for(int i = 0; i < size; i++) out[i] = children[i].asInt();}
	inline void asUIntArray  (unsigned* out, int size) { assert(type == S_JSON_TYPE_ARRAY); assert(out); if(size > childCount) size = childCount; for(int i = 0; i < size; i++) out[i] = children[i].asUInt();}
	inline void asFloatArray (float*    out, int size) { assert(type == S_JSON_TYPE_ARRAY); assert(out); if(size > childCount) size = childCount; for(int i = 0; i < size; i++) out[i] = children[i].asFloat();}
	inline void asDoubleArray(double*   out, int size) { assert(type == S_JSON_TYPE_ARRAY); assert(out); if(size > childCount) size = childCount; for(int i = 0; i < size; i++) out[i] = children[i].asDouble();}
	inline void asBoolArray  (bool*     out, int size) { assert(type == S_JSON_TYPE_ARRAY); assert(out); if(size > childCount) size = childCount; for(int i = 0; i < size; i++) out[i] = children[i].asBool();}
	inline void asStringArray(char**    out, int size) { assert(type == S_JSON_TYPE_ARRAY); assert(out); if(size > childCount) size = childCount; for(int i = 0; i < size; i++) out[i] = children[i].asString();}

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

	inline sJsonObject& operator[](const char* member) { auto m = getMember(member); assert(m!=nullptr); return m==nullptr ? *this : *m;}
	inline sJsonObject& operator[](int i)              { assert(children != nullptr); assert(i < childCount); return children[i]; };
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
	inline T&   operator[](int i) { assert(i >= 0 && i < size); return data[i]; }
	inline void clear() { if (data) { size = capacity = 0; free(data); data = nullptr; } }
	inline T*   begin() { return data; }
    inline T*   end() { return data + size; }
	inline T&   back() { assert(size > 0); return data[size - 1]; }
	inline void swap(mvVector<T>& rhs) { int rhs_size = rhs.size; rhs.size = size; size = rhs_size; int rhs_cap = rhs.capacity; rhs.capacity = capacity; capacity = rhs_cap; T* rhs_data = rhs.data; rhs.data = data; data = rhs_data; }
	inline int  _grow_capacity(int sz) { int new_capacity = capacity ? (capacity + capacity / 2) : 8; return new_capacity > sz ? new_capacity : sz; }
	inline void resize(int new_size) { if (new_size > capacity) reserve(_grow_capacity(new_size)); size = new_size; }
	inline void reserve(int new_capacity) { if (new_capacity <= capacity) return; T* new_data = (T*)malloc((size_t)new_capacity * sizeof(T)); if (data) { memcpy(new_data, data, (size_t)size * sizeof(T)); free(data); } data = new_data; capacity = new_capacity; }
	inline void push_back(const T& v) { if (size == capacity) reserve(_grow_capacity(size*2)); memcpy(&data[size], &v, sizeof(v)); size++;}
	inline void pop_back() { assert(size > 0); size--; }
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
	assert(object->_internal);

	if((*(mvVector<int>*)(object->_internal)).empty())
	{
		delete object->_internal;
		object->_internal = nullptr;
		return;
	}

	object->childCount = (*(mvVector<int>*)(object->_internal)).size;
	object->children = (sJsonObject*)malloc(sizeof(sJsonObject)*object->childCount);
	for(int i = 0; i < (*(mvVector<int>*)(object->_internal)).size; i++)
	{
		object->children[i] = *(*objects)[(*(mvVector<int>*)(object->_internal))[i]];
		_update_children_pointers(&object->children[i], objects);
	}

	delete object->_internal;
	object->_internal = nullptr;
}

sJsonObject*
Semper::parse_json(char* rawData, int size)
{
	sStack parentIDStack;
	mvVector<sJsonObject*> objectArray;

	char* spacesRemoved = (char*)malloc(sizeof(char)*size);
	_remove_whitespace(rawData, spacesRemoved, size);

	mvVector<sToken_>* tokens = new mvVector<sToken_>[size];
	_parse_for_tokens(spacesRemoved, *tokens);

	sJsonObject *rootObject = (sJsonObject *)malloc(sizeof(sJsonObject));
	rootObject->type = S_JSON_TYPE_OBJECT;
	rootObject->_internal = new mvVector<int>();
	
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
				sJsonObject *newObject = (sJsonObject*)malloc(sizeof(sJsonObject));
				newObject->type = S_JSON_TYPE_OBJECT;
				newObject->_internal = new mvVector<int>();
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
				sJsonObject *newObject = (sJsonObject*)malloc(sizeof(sJsonObject));
				newObject->type = S_JSON_TYPE_ARRAY;
				newObject->_internal = new mvVector<int>();
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

			sJsonObject* newObject = (sJsonObject*)malloc(sizeof(sJsonObject));
			objectArray.push_back(newObject);
			newObject->_internal = new mvVector<int>();
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
				sJsonObject *newObject = (sJsonObject*)malloc(sizeof(sJsonObject));
				newObject->type = S_JSON_TYPE_STRING;
				newObject->_internal = new mvVector<int>();
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
				sJsonObject *newObject = (sJsonObject*)malloc(sizeof(sJsonObject));
				if((*tokens)[i].value.data[0] == 't')      newObject->type = S_JSON_TYPE_BOOL;
				else if((*tokens)[i].value.data[0] == 'f') newObject->type = S_JSON_TYPE_BOOL;
				else if((*tokens)[i].value.data[0] == 'n') newObject->type = S_JSON_TYPE_NULL;
				else                                       newObject->type = S_JSON_TYPE_NUMBER;
				newObject->_internal = new mvVector<int>();
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

#endif