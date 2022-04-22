#include "sJsonParser.h"

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

struct mvStack
{
	void push(int value);
	void pop();
	int  top();
	bool empty();

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
					if (currentChar == '{') token.type = S_JSON_TOKEN_LEFT_BRACE;
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

sJsonDocument*
ParseJSON(char* rawData, int size)
{
	sJsonDocument* ptrContext = new sJsonDocument();
	sJsonDocument& context = *ptrContext;
	mvStack jsonObjectStack;
	mvStack jsonMemberStack;

	char* spacesRemoved = new char[size];
	RemoveWhiteSpace(rawData, spacesRemoved, size);

	mvVector<sToken>* tokens = new mvVector<sToken>;
	ParseForTokens(spacesRemoved, *tokens);

	//for (auto& token : tokens)
	//	std::cout << token.value << std::endl;

	sJsonObject rootObject{};
	rootObject.type = S_JSON_TYPE_OBJECT;
	rootObject.members.push_back({});
	context.jsonObjects.push_back(rootObject);
	jsonObjectStack.push(0);
	jsonMemberStack.push(0);

	int i = 0;
	while (true)
	{

		if (i >= tokens->size)
			break;

		switch ((*tokens)[i].type)
		{

		case S_JSON_TOKEN_LEFT_BRACE:
		{
			int parentId = jsonObjectStack.top();
			sJsonObject& parent = context.jsonObjects[parentId];

			if (parent.type == S_JSON_TYPE_OBJECT)
			{
				sJsonObject jobject{};
				jobject.type = S_JSON_TYPE_OBJECT;
				int newObjectIndex = context.jsonObjects.size;
				int memberId = jsonMemberStack.top();
				parent.members[memberId].index = newObjectIndex;
				context.jsonObjects.push_back(jobject);
				jsonObjectStack.push(newObjectIndex);
				if (!jsonMemberStack.empty())
					jsonMemberStack.pop();
				i++;
			}

			else if (parent.type == S_JSON_TYPE_ARRAY)
			{

				sJsonObject jobject{};
				jobject.type = S_JSON_TYPE_OBJECT;
				int newObjectIndex = context.jsonObjects.size;

				sJsonMember member{};
				member.type = S_JSON_TYPE_PRIMITIVE;
				member.index = newObjectIndex;
				parent.members.push_back(member);
				context.jsonObjects.push_back(jobject);
				jsonObjectStack.push(newObjectIndex);
				//jsonMemberStack.pop();
				i++;
			}
			break;
		}

		case S_JSON_TOKEN_LEFT_BRACKET:
		{
			sJsonObject jobject{};
			jobject.type = S_JSON_TYPE_ARRAY;
			int newObjectIndex = context.jsonObjects.size;
			context.jsonObjects.push_back(jobject);
			jsonObjectStack.push(newObjectIndex);
			jsonMemberStack.pop();
			i++;
			break;
		}

		case S_JSON_TOKEN_RIGHT_BRACE:
		{
			int finishedObject = jsonObjectStack.top();
			jsonObjectStack.pop();
			i++;
			break;
		}

		case S_JSON_TOKEN_RIGHT_BRACKET:
		{
			int finishedArray = jsonObjectStack.top();
			jsonObjectStack.pop();
			i++;
			break;
		}

		case S_JSON_TOKEN_MEMBER:
		{
			int parentId = jsonObjectStack.top();
			sJsonObject& parent = context.jsonObjects[parentId];
			sJsonMember member{};
			memcpy(member.name, (*tokens)[i].value.data, (*tokens)[i].value.size_in_bytes());


			sTokenType valueType = (*tokens)[i + 2].type;
			if (valueType == S_JSON_TOKEN_LEFT_BRACKET)
			{
				member.index = context.jsonObjects.size;
				member.type = S_JSON_TYPE_ARRAY;
			}
			else if (valueType == S_JSON_TOKEN_LEFT_BRACE)
			{
				member.index = context.jsonObjects.size;
				member.type = S_JSON_TYPE_OBJECT;
			}
			else if (valueType == S_JSON_TOKEN_PRIMITIVE)
			{
				member.index = context.primitiveValues.size;
				member.type = S_JSON_TYPE_PRIMITIVE;
			}
			else if (valueType == S_JSON_TOKEN_STRING)
			{
				member.index = context.primitiveValues.size;
				member.type = S_JSON_TYPE_STRING;
			}
			int memberId = parent.members.size;
			parent.members.push_back(member);
			jsonMemberStack.push(memberId);
			i++;
			break;
		}

		case S_JSON_TOKEN_STRING:
		{
			int parentId = jsonObjectStack.top();
			sJsonObject& parent = context.jsonObjects[parentId];

			if (parent.type == S_JSON_TYPE_OBJECT)
			{
				int valueId = context.primitiveValues.size;
				context.primitiveValues.push_back({});
				context.primitiveValues.back().value = (*tokens)[i].value;
				int memberId = jsonMemberStack.top();
				parent.members[memberId].index = valueId;
				jsonMemberStack.pop();
			}
			else if (parent.type == S_JSON_TYPE_ARRAY)
			{

				int valueId = context.primitiveValues.size;
				context.primitiveValues.push_back({});
				context.primitiveValues.back().value = (*tokens)[i].value;

				sJsonMember member{};
				member.type = S_JSON_TYPE_PRIMITIVE;
				member.index = valueId;
				parent.members.push_back(member);
			}
			i++;
			break;
		}

		case S_JSON_TOKEN_PRIMITIVE:
		{
			int parentId = jsonObjectStack.top();
			sJsonObject& parent = context.jsonObjects[parentId];
			if (parent.type == S_JSON_TYPE_OBJECT)
			{
				int valueId = context.primitiveValues.size;
				context.primitiveValues.push_back({});
				context.primitiveValues.back().value = (*tokens)[i].value;

				int memberId = jsonMemberStack.top();
				parent.members[memberId].index = valueId;
				jsonMemberStack.pop();
			}

			else if (parent.type == S_JSON_TYPE_ARRAY)
			{

				int valueId = context.primitiveValues.size;
				context.primitiveValues.push_back({});
				context.primitiveValues.back().value = (*tokens)[i].value;

				sJsonMember member{};
				member.type = S_JSON_TYPE_PRIMITIVE;
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
sJsonDocument::doesMemberExist(const char* member)
{
	for (int i = 0; i < jsonObjects[1].members.size; i++)
	{
		if (strcmp(member, jsonObjects[1].members[i].name) == 0)
			return true;
	}

	return false;
}

sJsonObject&
sJsonDocument::operator[](const char* member)
{
	for (int i = 0; i < jsonObjects[1].members.size; i++)
	{

		if (strcmp(member, jsonObjects[1].members[i].name) == 0)
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

	if (value == data.size)
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
sJsonObject::doesMemberExist(const char* member)
{
	for (int i = 0; i < members.size; i++)
	{
		if (strcmp(member, members[i].name) == 0)
			return true;
	}

	return false;
}

sJsonMember&
sJsonObject::operator[](int i)
{
	return members[i];
}

sJsonObject&
sJsonObject::operator[](const char* member)
{
	for (int i = 0; i < members.size; i++)
	{

		if (strcmp(member, members[i].name) == 0)
			return context->jsonObjects[members[i].index];
	}

	return context->jsonObjects[0];
}

sJsonMember&
sJsonObject::getMember(const char* member)
{
	for (int i = 0; i < members.size; i++)
	{
		if (strcmp(member, members[i].name) == 0)
			return members[i];
	}

	return members[0];
}

sJsonMember::operator int()
{
	return atoi(context->primitiveValues[index].value.data);
}

sJsonMember::operator unsigned()
{
	int value = atoi(context->primitiveValues[index].value.data);
	return (unsigned)value;
}

sJsonMember::operator float()
{
	return atof(context->primitiveValues[index].value.data);
}

// mvJsonMember::operator char*()
// {
// 	return (char*)context->primitiveValues[index].value.data;
// }

sJsonMember::operator std::string()
{
	return std::string((char*)context->primitiveValues[index].value.data);
}

sJsonMember::operator sJsonObject& ()
{
	return context->jsonObjects[index];
}