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
	sJsonObject* getMember      (const char* member);
	inline bool  doesMemberExist(const char* member) { return getMember(member) != nullptr;}

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

sJsonObject* ParseJSON(char* rawData, int size);

#endif