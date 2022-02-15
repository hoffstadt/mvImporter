#pragma once
#include <wrl.h>

typedef bool             b8;
typedef int              b32;

typedef float            f32;
typedef double           f64;

typedef signed char      i8;
typedef signed short     i16;
typedef signed int       i32;
typedef signed __int64   i64;

typedef i8               s8;
typedef i16              s16;
typedef i32              s32;
typedef i64              s64;

typedef unsigned char    u8;
typedef unsigned short   u16;
typedef unsigned int     u32;
typedef unsigned __int64 u64;

typedef s32 mvAssetID;

template <typename T>
using mvComPtr = Microsoft::WRL::ComPtr<T>;