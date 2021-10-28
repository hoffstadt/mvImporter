#pragma once

#include <cmath>
#include "mvTypes.h"

mv_global constexpr f32 PI = 3.14159265f;
mv_global constexpr f64 PI_D = 3.1415926535897932;

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
};

struct mvMat4
{
	mvVec4 cols[4];

	mvVec4& operator[](i32 index);
};

mvMat4 mvIdentityMat4();

struct mvTransforms
{
	mvMat4 model               = mvIdentityMat4();
	mvMat4 modelView           = mvIdentityMat4();
	mvMat4 modelViewProjection = mvIdentityMat4();
};

mvVec2 operator+(mvVec2& left, mvVec2& right);
mvVec2 operator-(mvVec2& left, mvVec2& right);
mvVec2 operator*(mvVec2& left, mvVec2& right);
mvVec2 operator*(mvVec2& left, f32 right);

mvVec3 operator+(mvVec3& left, mvVec3& right);
mvVec3 operator-(mvVec3& left, mvVec3& right);
mvVec3 operator*(mvVec3& left, mvVec3& right);
mvVec3 operator*(mvVec3& left, f32 right);

mvVec4 operator+(mvVec4& left, mvVec4& right);
mvVec4 operator-(mvVec4& left, mvVec4& right);
mvVec4 operator*(mvVec4& left, mvVec4& right);
mvVec4 operator*(mvVec4& left, f32 right);
mvVec4 operator*(mvMat4& left, mvVec4& right);

mvMat4 operator*(mvMat4& left, mvMat4& right);

mvVec3 mvNormalize(mvVec3& v);
mvVec3 mvCross    (mvVec3& v1, mvVec3& v2);
f32    mvDot      (mvVec3& v1, mvVec3& v2);
f32    mvRadians  (f32 degrees);

mvMat4 mvYawPitchRoll (f32 yaw, f32 pitch, f32 roll);
mvMat4 mvLookAtLH     (mvVec3& eye, mvVec3& center, mvVec3& up);
mvMat4 mvLookAtRH     (mvVec3& eye, mvVec3& center, mvVec3& up);
mvMat4 mvTranslate    (mvMat4& m, mvVec3& v);
mvMat4 mvRotate       (mvMat4& m, f32 angle, mvVec3& v);
mvMat4 mvScale        (mvMat4& m, mvVec3& v);
mvMat4 mvOrthoLH      (f32 left, f32 right, f32 bottom, f32 top, f32 zNear, f32 zFar);
mvMat4 mvPerspectiveLH(f32 fovy, f32 aspect, f32 zNear, f32 zFar);
