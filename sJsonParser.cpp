#include "sJsonParser.h"

// borrowed from Dear ImGui
template<typename T>
struct mvVector
{
	int size     = 0u;
	int capacity = 0u;
	T*  data     = nullptr;
	inline mvVector() { size = capacity = 0; data = nullptr; }
	inline mvVector<T>& operator=(const mvVector<T>& src) { clear(); resize(src.size); memcpy(data, src.data, (size_t)size * sizeof(T)); return *this; }
	//inline ~mvVector() { if (data) free(data); }
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

typedef int sTokenType;

enum sTokenType_
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

struct sToken
{
	sTokenType type = S_JSON_TOKEN_NONE;
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
ParseForTokens(char* rawData, mvVector<sToken>& tokens)
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
						sToken primitivetoken{};
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

					sToken token{};
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
					sToken token{};
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

static void
UpdateChildenPointers(sJsonObject* object, mvVector<sJsonObject*>* objects)
{
	assert(object->_internal);

	if((*(mvVector<int>*)(object->_internal)).empty())
	{
		delete object->_internal;
		object->_internal = nullptr;
		return;
	}

	object->childCount = (*(mvVector<int>*)(object->_internal)).size;
	object->children = new sJsonObject[object->childCount];
	for(int i = 0; i < (*(mvVector<int>*)(object->_internal)).size; i++)
	{
		object->children[i] = *(*objects)[(*(mvVector<int>*)(object->_internal))[i]];
		UpdateChildenPointers(&object->children[i], objects);
	}

	delete object->_internal;
	object->_internal = nullptr;
}

sJsonObject*
ParseJSON(char* rawData, int size)
{
	sStack parentIDStack;
	mvVector<sJsonObject*> objectArray;

	char* spacesRemoved = new char[size];
	RemoveWhiteSpace(rawData, spacesRemoved, size);

	mvVector<sToken>* tokens = new mvVector<sToken>;
	ParseForTokens(spacesRemoved, *tokens);

	sJsonObject *rootObject = new sJsonObject();
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
				sJsonObject *newObject = new sJsonObject();
				newObject->type = S_JSON_TYPE_OBJECT;
				newObject->_internal = new mvVector<int>();
				mvVector<int>          _childrenID;
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
				sJsonObject *newObject = new sJsonObject();
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

			sJsonObject* newObject = new sJsonObject();
			objectArray.push_back(newObject);
			newObject->_internal = new mvVector<int>();
			parentIDStack.push(objectArray.size-1);
			(*(mvVector<int>*)(parent->_internal)).push_back(objectArray.size-1);
			memcpy(newObject->name, (*tokens)[i].value.data, (*tokens)[i].value.size_in_bytes());

			// look ahead to 2 tokens to look at type (skipping over ':' )
			sToken valueToken = (*tokens)[i + 2];
			sTokenType valueType = valueToken.type;
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
				sJsonObject *newObject = new sJsonObject();
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
				sJsonObject *newObject = new sJsonObject();
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
	UpdateChildenPointers(rootObject, &objectArray);
	objectArray.clear();
	parentIDStack.data.clear();
	return rootObject;
}

sJsonObject*
sJsonObject::getMember(const char* member)
{
	for (int i = 0; i < childCount; i++)
	{
		if (strcmp(member, children[i].name) == 0)
			return &children[i];
	}
	return nullptr;
}

