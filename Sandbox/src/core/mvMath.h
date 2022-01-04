#pragma once

#include <cmath>
#include "mvTypes.h"

static constexpr f32 PI = 3.14159265f;
static constexpr f64 PI_D = 3.1415926535897932;

template<typename T>
T clamp(T value, T minvalue, T maxvalue)
{
	if (value < minvalue)
		return minvalue;
	if (value > maxvalue)
		return maxvalue;
	return value;
}

struct mvVec2
{
	union { f32 x, r, u; };
	union { f32 y, g, v; };

	float& operator[](i32 index);
};

struct mvVec3
{
	union { f32 x, r; };
	union { f32 y, g; };
	union { f32 z, b; };

	float& operator[](i32 index);
};

struct mvVec4
{
	union { f32 x, r; };
	union { f32 y, g; };
	union { f32 z, b; };
	union { f32 w, a; };

	float& operator[](i32 index);

	mvVec3 xyz()
	{
		return mvVec3{ x, y, z };
	}
};

struct mvMat4
{
	mvVec4 cols[4];

	mvVec4& operator[](i32 index);
};

mvMat4 identity_mat4();
mvMat4 construct_mat4(mvVec4 c0, mvVec4 c1, mvVec4 c2, mvVec4 c3);

struct mvTransforms
{
	mvMat4 model               = identity_mat4();
	mvMat4 modelView           = identity_mat4();
	mvMat4 modelViewProjection = identity_mat4();
};

mvVec2 operator+(mvVec2 left, mvVec2 right);
mvVec2 operator-(mvVec2 left, mvVec2 right);
mvVec2 operator*(mvVec2 left, mvVec2 right);
mvVec2 operator*(mvVec2 left, f32 right);

mvVec3 operator+(mvVec3 left, mvVec3 right);
mvVec3 operator-(mvVec3 left, mvVec3 right);
mvVec3 operator*(mvVec3 left, mvVec3 right);
mvVec3 operator*(mvVec3 left, f32 right);

mvVec4 operator+(mvVec4 left, mvVec4 right);
mvVec4 operator-(mvVec4 left, mvVec4 right);
mvVec4 operator*(mvVec4 left, mvVec4 right);
mvVec4 operator*(mvVec4 left, f32 right);
mvVec4 operator*(mvMat4 left, mvVec4 right);

mvMat4 operator*(mvMat4 left, mvMat4 right);
mvMat4 operator*(mvMat4 left, f32 right);

mvVec2 normalize(mvVec2 v);
mvVec3 normalize(mvVec3 v);
mvVec4 normalize(mvVec4 v);
mvVec3 cross    (mvVec3 v1, mvVec3 v2);
f32    dot      (mvVec3 v1, mvVec3 v2);
f32    radians  (f32 degrees);

mvVec4 slerpQuat(mvVec4 q1, mvVec4 q2, f32 t);
mvMat4 yaw_pitch_roll(f32 yaw, f32 pitch, f32 roll);
mvMat4 lookat        (mvVec3 eye, mvVec3 center, mvVec3 up); // world space eye, center, up
mvMat4 fps           (mvVec3 eye, float pitch, float yaw);   // world space eye, center, up
mvMat4 translate     (mvMat4 m, mvVec3 v);
mvMat4 rotate        (mvMat4 m, f32 angle, mvVec3 v);
mvMat4 scale         (mvMat4 m, mvVec3 v);
mvMat4 ortho         (f32 left, f32 right, f32 bottom, f32 top, f32 zNear, f32 zFar);
mvMat4 perspective   (f32 fovy, f32 aspect, f32 zNear, f32 zFar);
mvMat4 invert        (mvMat4& m);
mvMat4 rotation_translation_scale(mvVec4& q, mvVec3& t, mvVec3& s);
mvMat4 create_matrix(
	f32 m00, f32 m01, f32 m02, f32 m03,
	f32 m10, f32 m11, f32 m12, f32 m13,
	f32 m20, f32 m21, f32 m22, f32 m23,
	f32 m30, f32 m31, f32 m32, f32 m33
	);
