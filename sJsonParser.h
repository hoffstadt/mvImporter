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

struct sJsonObject;   // json object or array
typedef int sJsonType;

enum sJsonType_
{
	S_JSON_TYPE_NONE,
	S_JSON_TYPE_STRING,
	S_JSON_TYPE_ARRAY,
	S_JSON_TYPE_PRIMITIVE,
	S_JSON_TYPE_OBJECT
};

struct sJsonObject
{
	sJsonType    type;
	sJsonObject* children;
	int          childCount;	
	char         name[S_JSON_MAX_NAME_LENGTH];
	char*        value;
	void*        _internal;

	// retrieve & cast values
	inline int      asInt()    { assert(type == S_JSON_TYPE_PRIMITIVE); return atoi(value);}
	inline unsigned asUInt()   { assert(type == S_JSON_TYPE_PRIMITIVE); return (unsigned)atoi(value);}
	inline float    asFloat()  { assert(type == S_JSON_TYPE_PRIMITIVE); return atof(value);}
	inline double   asDouble() { assert(type == S_JSON_TYPE_PRIMITIVE); return strtod(value, nullptr);}
	inline char*    asString() { assert(type == S_JSON_TYPE_STRING);    return value;}
	inline bool    	asBool()   { assert(type == S_JSON_TYPE_PRIMITIVE); return value[0] == 't';}

	// retrieve members
	sJsonObject*       getMember      (const char* member);
	inline const char* getStringMember(const char* member, const char* defaultValue=0)     { auto m = getMember(member); return m==0 ? defaultValue : m->asString();}
	inline int         getIntMember   (const char* member, int         defaultValue=0)     { auto m = getMember(member); return m==nullptr ? defaultValue : m->asInt();}
	inline unsigned    getUIntMember  (const char* member, unsigned    defaultValue=0u)    { auto m = getMember(member); return m==nullptr ? defaultValue : m->asUInt();}
	inline float       getFloatMember (const char* member, float       defaultValue=0.0f)  { auto m = getMember(member); return m==nullptr ? defaultValue : m->asFloat();}
	inline double      getDoubleMember(const char* member, double      defaultValue=0.0)   { auto m = getMember(member); return m==nullptr ? defaultValue : m->asDouble();}
	inline bool        getBoolMember  (const char* member, bool        defaultValue=false) { auto m = getMember(member); return m==nullptr ? defaultValue : m->asBool();}
	inline bool        doesMemberExist(const char* member)                                 { return getMember(member) != nullptr;}

	inline sJsonObject& operator[](const char* member) { auto m = getMember(member); assert(m!=nullptr); return m==nullptr ? *this : *m;}
	inline sJsonObject& operator[](int i)              { assert(children != nullptr); assert(i < childCount); return children[i]; };
};

sJsonObject* ParseJSON(char* rawData, int size);

#endif