
#ifdef MV_MATH_IMPLEMENTATION
#include <assert.h>
#endif

#ifndef MV_MATH_H
#define MV_MATH_H

#include <cmath>

static constexpr float PI = 3.14159265f;
static constexpr double PI_D = 3.1415926535897932;

template<typename T>
T clamp(T value, T minvalue, T maxvalue)
{
	if (value < minvalue)
		return minvalue;
	if (value > maxvalue)
		return maxvalue;
	return value;
}

template<typename T>
T get_max(T v1, T v2)
{
	if (v1 > v2)
		return v1;
	return v2;
}

template<typename T>
T get_min(T v1, T v2)
{
	if (v1 < v2)
		return v1;
	return v2;
}

struct mvVec2
{
	union { float x, r, u; };
	union { float y, g, v; };

	float& operator[](int index);
};

struct mvVec3
{
	union { float x, r; };
	union { float y, g; };
	union { float z, b; };

	float& operator[](int index);
};

struct mvVec4
{
	union { float x, r; };
	union { float y, g; };
	union { float z, b; };
	union { float w, a; };

	float& operator[](int index);

	mvVec3 xyz()
	{
		return mvVec3{ x, y, z };
	}
};

struct mvMat4
{
	mvVec4 cols[4];

	mvVec4& operator[](int index);
	float& at(int index);
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
mvVec2 operator*(mvVec2 left, float right);

mvVec3 operator+(mvVec3 left, mvVec3 right);
mvVec3 operator-(mvVec3 left, mvVec3 right);
mvVec3 operator*(mvVec3 left, mvVec3 right);
mvVec3 operator*(mvVec3 left, float right);

mvVec4 operator+(mvVec4 left, mvVec4 right);
mvVec4 operator-(mvVec4 left, mvVec4 right);
mvVec4 operator*(mvVec4 left, mvVec4 right);
mvVec4 operator*(mvVec4 left, float right);
mvVec4 operator*(mvMat4 left, mvVec4 right);
mvVec3 operator*(mvMat4 left, mvVec3 right);

mvMat4 operator*(mvMat4 left, mvMat4 right);
mvMat4 operator*(mvMat4 left, float right);

mvVec2 normalize(mvVec2 v);
mvVec3 normalize(mvVec3 v);
mvVec4 normalize(mvVec4 v);
mvVec3 cross    (mvVec3 v1, mvVec3 v2);
float  dot      (mvVec3 v1, mvVec3 v2);
float  radians  (float degrees);

mvVec4 slerpQuat(mvVec4 q1, mvVec4 q2, float t);
mvMat4 yaw_pitch_roll(float yaw, float pitch, float roll);
mvMat4 lookat        (mvVec3 eye, mvVec3 center, mvVec3 up); // world space eye, center, up
mvMat4 fps           (mvVec3 eye, float pitch, float yaw);   // world space eye, center, up
mvMat4 translate     (mvMat4 m, mvVec3 v);
mvMat4 rotate        (mvMat4 m, float angle, mvVec3 v);
mvMat4 scale         (mvMat4 m, mvVec3 v);
mvMat4 ortho         (float left, float right, float bottom, float top, float zNear, float zFar);
mvMat4 perspective   (float fovy, float aspect, float zNear, float zFar);
mvMat4 invert        (mvMat4& m);
mvMat4 transpose     (mvMat4& m);
mvMat4 rotation_translation_scale(mvVec4& q, mvVec3& t, mvVec3& s);
mvMat4 create_matrix(
	float m00, float m01, float m02, float m03,
	float m10, float m11, float m12, float m13,
	float m20, float m21, float m22, float m23,
	float m30, float m31, float m32, float m33
	);

#endif

#ifdef MV_MATH_IMPLEMENTATION

float 
radians(float degrees)
{
	return degrees * 0.01745329251994329576923690768489f;
}

float&
mvVec2::operator[](int index)
{
	switch (index)
	{
	case 0: return x;
	case 1: return y;
	default: return y;
	}
}

float&
mvVec3::operator[](int index)
{
	switch (index)
	{
	case 0: return x;
	case 1: return y;
	case 2: return z;
	default: return z;
	}
}

float&
mvVec4::operator[](int index)
{
	switch (index)
	{
	case 0: return x;
	case 1: return y;
	case 2: return z;
	case 3: return w;
	default: return w;
	}
}

mvVec4&
mvMat4::operator[](int index)
{
	switch (index)
	{
	case 0: return cols[0];
	case 1: return cols[1];
	case 2: return cols[2];
	case 3: return cols[3];
	default: return cols[3];
	}
}

float& 
mvMat4::at(int index)
{
	int col = index/4;
	int row = index % 4;
	return cols[col][row];
}

mvVec2
operator+(mvVec2 left, mvVec2 right)
{
	mvVec2 result = left;

	result.x += right.x;
	result.y += right.y;

	return result;
}

mvVec2
operator-(mvVec2 left, mvVec2 right)
{
	mvVec2 result = left;

	result.x -= right.x;
	result.y -= right.y;

	return result;
}

mvVec2
operator*(mvVec2 left, mvVec2 right)
{
	mvVec2 result = left;

	result.x *= right.x;
	result.y *= right.y;

	return result;
}

mvVec2
operator*(mvVec2 left, float right)
{
	mvVec2 result = left;

	result[0] *= right;
	result[1] *= right;

	return result;
}

mvVec3 
operator+(mvVec3 left, mvVec3 right)
{
	mvVec3 result = left;

	result.x += right.x;
	result.y += right.y;
	result.z += right.z;

	return result;
}

mvVec3 
operator-(mvVec3 left, mvVec3 right)
{
	mvVec3 result = left;

	result.x -= right.x;
	result.y -= right.y;
	result.z -= right.z;

	return result;
}

mvVec3 
operator*(mvVec3 left, mvVec3 right)
{
	mvVec3 result = left;

	result.x *= right.x;
	result.y *= right.y;
	result.z *= right.z;

	return result;
}

mvVec3
operator*(mvVec3 left, float right)
{
	mvVec3 result = left;

	result[0] *= right;
	result[1] *= right;
	result[2] *= right;

	return result;
}

mvVec4 
operator+(mvVec4 left, mvVec4 right)
{
	mvVec4 result = left;

	result.x += right.x;
	result.y += right.y;
	result.z += right.z;
	result.w += right.w;

	return result;
}

mvVec4 
operator-(mvVec4 left, mvVec4 right)
{
	mvVec4 result = left;

	result.x -= right.x;
	result.y -= right.y;
	result.z -= right.z;
	result.w -= right.w;

	return result;
}

mvVec4 
operator*(mvVec4 left, mvVec4 right)
{
	mvVec4 result = left;

	result.x = left.x * right.x;
	result.y = left.y * right.y;
	result.z = left.z * right.z;
	result.w = left.w * right.w;

	return result;
}

mvVec4 
operator*(mvMat4 left, mvVec4 right)
{
	mvVec4 Mov0 = { right[0], right[0], right[0], right[0] };
	mvVec4 Mov1 = { right[1], right[1], right[1], right[1] };
	mvVec4 Mul0 = left[0] * Mov0;
	mvVec4 Mul1 = left[1] * Mov1;
	mvVec4 Add0 = Mul0 + Mul1;
	mvVec4 Mov2 = { right[2], right[2], right[2], right[2] };
	mvVec4 Mov3 = { right[3], right[3], right[3], right[3] };
	mvVec4 Mul2 = left[2] * Mov2;
	mvVec4 Mul3 = left[3] * Mov3;
	mvVec4 Add1 = Mul2 + Mul3;
	mvVec4 Add2 = Add0 + Add1;
	return Add2;
}

mvVec3
operator*(mvMat4 left, mvVec3 right)
{
	mvVec4 Mov0 = { right[0], right[0], right[0], right[0] };
	mvVec4 Mov1 = { right[1], right[1], right[1], right[1] };
	mvVec4 Mul0 = left[0] * Mov0;
	mvVec4 Mul1 = left[1] * Mov1;
	mvVec4 Add0 = Mul0 + Mul1;
	mvVec4 Mov2 = { right[2], right[2], right[2], right[2] };
	mvVec4 Mov3 = { 1.0f, 1.0f, 1.0f, 1.0f};
	mvVec4 Mul2 = left[2] * Mov2;
	mvVec4 Mul3 = left[3] * Mov3;
	mvVec4 Add1 = Mul2 + Mul3;
	mvVec4 Add2 = Add0 + Add1;
	return Add2.xyz();
}

mvVec4 
operator*(mvVec4 left, float right)
{
	mvVec4 result = left;

	result[0] *= right;
	result[1] *= right;
	result[2] *= right;
	result[3] *= right;

	return result;
}

mvMat4 
operator*(mvMat4 left, mvMat4 right)
{
	mvVec4 SrcA0 = left[0];
	mvVec4 SrcA1 = left[1];
	mvVec4 SrcA2 = left[2];
	mvVec4 SrcA3 = left[3];

	mvVec4 SrcB0 = right[0];
	mvVec4 SrcB1 = right[1];
	mvVec4 SrcB2 = right[2];
	mvVec4 SrcB3 = right[3];

	mvMat4 result{};

	result[0] = SrcA0 * SrcB0[0] + SrcA1 * SrcB0[1] + SrcA2 * SrcB0[2] + SrcA3 * SrcB0[3];
	result[1] = SrcA0 * SrcB1[0] + SrcA1 * SrcB1[1] + SrcA2 * SrcB1[2] + SrcA3 * SrcB1[3];
	result[2] = SrcA0 * SrcB2[0] + SrcA1 * SrcB2[1] + SrcA2 * SrcB2[2] + SrcA3 * SrcB2[3];
	result[3] = SrcA0 * SrcB3[0] + SrcA1 * SrcB3[1] + SrcA2 * SrcB3[2] + SrcA3 * SrcB3[3];

	return result;
}

mvMat4
operator*(mvMat4 left, float right)
{
	mvMat4 result = left;

	for (unsigned int i = 0; i < 4; i++)
		for (unsigned int j = 0; j < 4; j++)
			result[i][j] *= right;

	return result;
}

mvMat4 
identity_mat4()
{
	static mvMat4 m = create_matrix(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	return m;
}

mvMat4 
translate(mvMat4 m, mvVec3 v)
{
	mvMat4 result = m;

	result.cols[3] =
		m[0] * v.x +
		m[1] * v.y +
		m[2] * v.z +
		m[3];

	return result;
}

mvMat4 
rotate(mvMat4 m, float angle, mvVec3 v)
{
	const float a = angle;
	const float c = cos(a);
	const float s = sin(a);

	mvVec3 axis = normalize(v);
	mvVec3 temp = axis * (1.0f - c);

	mvMat4 rotate{};

	rotate[0][0] = c + temp[0] * axis[0];
	rotate[0][1] = temp[0] * axis[1] + s * axis[2];
	rotate[0][2] = temp[0] * axis[2] - s * axis[1];

	rotate[1][0] = temp[1] * axis[0] - s * axis[2];
	rotate[1][1] = c + temp[1] * axis[1];
	rotate[1][2] = temp[1] * axis[2] + s * axis[0];

	rotate[2][0] = temp[2] * axis[0] + s * axis[1];
	rotate[2][1] = temp[2] * axis[1] - s * axis[0];
	rotate[2][2] = c + temp[2] * axis[2];

	mvMat4 result{};
	result[0] = m[0] * rotate[0][0] + m[1] * rotate[0][1] + m[2] * rotate[0][2];
	result[1] = m[0] * rotate[1][0] + m[1] * rotate[1][1] + m[2] * rotate[1][2];
	result[2] = m[0] * rotate[2][0] + m[1] * rotate[2][1] + m[2] * rotate[2][2];
	result[3] = m[3];
	
	return result;
}

mvMat4 
yaw_pitch_roll(float yaw, float pitch, float roll)
{
	// x = roll
	// y = pitch
	// z = yaw

	float tmp_ch = cos(yaw);
	float tmp_sh = sin(yaw);
	float tmp_cp = cos(pitch);
	float tmp_sp = sin(pitch);
	float tmp_cb = cos(roll);
	float tmp_sb = sin(roll);

	mvMat4 result{};

	// column 0
	result[0][0] = tmp_ch * tmp_cb + tmp_sh * tmp_sp * tmp_sb;
	result[0][1] = tmp_sb * tmp_cp;
	result[0][2] = -tmp_sh * tmp_cb + tmp_ch * tmp_sp * tmp_sb;
	result[0][3] = 0.0f;

	// column 1
	result[1][0] = -tmp_ch * tmp_sb + tmp_sh * tmp_sp * tmp_cb;
	result[1][1] = tmp_cb * tmp_cp;
	result[1][2] = tmp_sb * tmp_sh + tmp_ch * tmp_sp * tmp_cb;
	result[1][3] = 0.0f;

	// column 2
	result[2][0] = tmp_sh * tmp_cp;
	result[2][1] = -tmp_sp;
	result[2][2] = tmp_ch * tmp_cp;
	result[2][3] = 0.0f;

	// column 3
	result[3][0] = 0.0f;
	result[3][1] = 0.0f;
	result[3][2] = 0.0f;
	result[3][3] = 1.0f;

	return result;
}

mvMat4 
scale(mvMat4 m, mvVec3 v)
{
	mvMat4 result{};
	result[0] = m[0] * v[0];
	result[1] = m[1] * v[1];
	result[2] = m[2] * v[2];
	result[3] = m[3];
	return result;
}

mvVec2
normalize(mvVec2 v)
{
	float length = sqrt(v[0] * v[0] + v[1] * v[1]);
	mvVec2 result{};
	result.x = v.x / length;
	result.y = v.y / length;
	return result;
}

mvVec3 
normalize(mvVec3 v)
{
	float length = sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
	mvVec3 result{};
	result.x = v.x / length;
	result.y = v.y / length;
	result.z = v.z / length;
	return result;
}

mvVec4
normalize(mvVec4 v)
{
	float length = sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2] + v[3] * v[3]);
	mvVec4 result{};
	result.x = v.x / length;
	result.y = v.y / length;
	result.z = v.z / length;
	result.w = v.w / length;
	return result;
}

mvVec3 
cross(mvVec3 v1, mvVec3 v2)
{
	mvVec3 result{};
	result.x = v1.y * v2.z - v2.y * v1.z;
	result.y = v1.z * v2.x - v2.z * v1.x;
	result.z = v1.x * v2.y - v2.x * v1.y;
	return result;
}

float
dot(mvVec3 v1, mvVec3 v2)
{
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

mvMat4
lookat(mvVec3 eye, mvVec3 center, mvVec3 up)
{
	mvVec3 zaxis = normalize(center - eye);
	mvVec3 xaxis = normalize(cross(up, zaxis));
	mvVec3 yaxis = cross(zaxis, xaxis);

	mvMat4 viewMatrix = identity_mat4();

	// row 0
	viewMatrix[0][0] = xaxis.x;
	viewMatrix[1][0] = xaxis.y;
	viewMatrix[2][0] = xaxis.z;
	viewMatrix[3][0] = -dot(xaxis, eye);

	// row 1
	viewMatrix[0][1] = yaxis.x;
	viewMatrix[1][1] = yaxis.y;
	viewMatrix[2][1] = yaxis.z;
	viewMatrix[3][1] = -dot(yaxis, eye);

	// row 2
	viewMatrix[0][2] = zaxis.x;
	viewMatrix[1][2] = zaxis.y;
	viewMatrix[2][2] = zaxis.z;
	viewMatrix[3][2] = -dot(zaxis, eye);

	return viewMatrix;
}

mvMat4 
fps(mvVec3 eye, float pitch, float yaw)
{

	// I assume the values are already converted to radians.
	float cosPitch = cos(pitch);
	float sinPitch = sin(pitch);
	float cosYaw = cos(yaw);
	float sinYaw = sin(yaw);

	mvVec3 xaxis = { cosYaw, 0, -sinYaw };
	mvVec3 yaxis = { sinYaw * sinPitch, cosPitch, cosYaw * sinPitch };
	mvVec3 zaxis = { sinYaw * cosPitch, -sinPitch, cosPitch * cosYaw };

	mvMat4 viewMatrix = construct_mat4(
		mvVec4{ xaxis.x, yaxis.x, zaxis.x, 0 },
		mvVec4{ xaxis.y, yaxis.y, zaxis.y, 0 },
		mvVec4{ xaxis.z, yaxis.z, zaxis.z, 0 },
		mvVec4{ -dot(xaxis, eye), -dot(yaxis, eye), -dot(zaxis, eye), 1 }
	);

	return viewMatrix;
}

mvMat4
ortho(float left, float right, float bottom, float top, float zNear, float zFar)
{
	mvMat4 result = identity_mat4();
	result[0][0] = 2.0f / (right - left);
	result[1][1] = 2.0f / (top - bottom);
	result[2][2] = -2.0f / (zFar - zNear);
	result[3][0] = -(right + left) / (right - left);
	result[3][1] = -(top + bottom) / (top - bottom);
	result[3][2] = -(zFar + zNear) / (zFar - zNear);
	return result;
}

mvMat4
perspective(float fovy, float aspect, float zNear, float zFar)
{
	const float tanHalfFovy = tan(fovy / 2.0f);

	mvMat4 result{};
	result[0][0] = 1.0f / (aspect * tanHalfFovy);
	result[1][1] = 1.0f / (tanHalfFovy);
	result[2][2] = -zFar/ (zFar - zNear);
	result[2][3] = -1.0f;
	result[3][2] = -(zFar * zNear) / (zFar - zNear);
	return result;
}

mvMat4
invert(mvMat4& m)
{
	mvMat4 invout = identity_mat4();
	float det = 0.0f;

	float a00 = m.at(0);
	float a01 = m.at(1);
	float a02 = m.at(2);
	float a03 = m.at(3);
	float a10 = m.at(4);
	float a11 = m.at(5);
	float a12 = m.at(6);
	float a13 = m.at(7);
	float a20 = m.at(8);
	float a21 = m.at(9);
	float a22 = m.at(10);
	float a23 = m.at(11);
	float a30 = m.at(12);
	float a31 = m.at(13);
	float a32 = m.at(14);
	float a33 = m.at(15);

	float b00 = a00 * a11 - a01 * a10;
	float b01 = a00 * a12 - a02 * a10;
	float b02 = a00 * a13 - a03 * a10;
	float b03 = a01 * a12 - a02 * a11;
	float b04 = a01 * a13 - a03 * a11;
	float b05 = a02 * a13 - a03 * a12;
	float b06 = a20 * a31 - a21 * a30;
	float b07 = a20 * a32 - a22 * a30;
	float b08 = a20 * a33 - a23 * a30;
	float b09 = a21 * a32 - a22 * a31;
	float b10 = a21 * a33 - a23 * a31;
	float b11 = a22 * a33 - a23 * a32;

	det = b00 * b11 - b01 * b10 + b02 * b09 + b03 * b08 - b04 * b07 + b05 * b06;

	if (det == 0)
	{
		assert(false);
		return invout;
	}

	det = 1.0f / det;

	invout.at(0) = (a11 * b11 - a12 * b10 + a13 * b09) * det;
	invout.at(1) = (a02 * b10 - a01 * b11 - a03 * b09) * det;
	invout.at(2) = (a31 * b05 - a32 * b04 + a33 * b03) * det;
	invout.at(3) = (a22 * b04 - a21 * b05 - a23 * b03) * det;
	invout.at(4) = (a12 * b08 - a10 * b11 - a13 * b07) * det;
	invout.at(5) = (a00 * b11 - a02 * b08 + a03 * b07) * det;
	invout.at(6) = (a32 * b02 - a30 * b05 - a33 * b01) * det;
	invout.at(7) = (a20 * b05 - a22 * b02 + a23 * b01) * det;
	invout.at(8) = (a10 * b10 - a11 * b08 + a13 * b06) * det;
	invout.at(9) = (a01 * b08 - a00 * b10 - a03 * b06) * det;
	invout.at(10) = (a30 * b04 - a31 * b02 + a33 * b00) * det;
	invout.at(11) = (a21 * b02 - a20 * b04 - a23 * b00) * det;
	invout.at(12) = (a11 * b07 - a10 * b09 - a12 * b06) * det;
	invout.at(13) = (a00 * b09 - a01 * b07 + a02 * b06) * det;
	invout.at(14) = (a31 * b01 - a30 * b03 - a32 * b00) * det;
	invout.at(15) = (a20 * b03 - a21 * b01 + a22 * b00) * det;

	return invout;
}

mvMat4
construct_mat4(mvVec4 c0, mvVec4 c1, mvVec4 c2, mvVec4 c3)
{
	mvMat4 result{};
	result[0] = c0;
	result[1] = c1;
	result[2] = c2;
	result[3] = c3;
	return result;
}

mvVec4
slerpQuat(mvVec4 q1, mvVec4 q2, float t)
{

	// from https://glmatrix.net/docs/quat.js.html
	mvVec4 qn1 = normalize(q1);
	mvVec4 qn2 = normalize(q2);

	mvVec4 qresult{};

	float ax = qn1.x;
	float ay = qn1.y;
	float az = qn1.z;
	float aw = qn1.w;

	float bx = qn2.x;
	float by = qn2.y;
	float bz = qn2.z;
	float bw = qn2.w;

	float omega = 0.0f;
	float cosom = 0.0f;
	float sinom = 0.0f;
	float scale0 = 0.0f;
	float scale1 = 0.0f;

	// calc cosine
	cosom = ax * bx + ay * by + az * bz + aw * bw;

	// adjust signs (if necessary)
	if (cosom < 0.0f) 
	{
		cosom = -cosom;
		bx = -bx;
		by = -by;
		bz = -bz;
		bw = -bw;
	}

	// calculate coefficients
	if (1.0f - cosom > 0.000001f)
	{
		// standard case (slerp)
		omega = acos(cosom);
		sinom = sin(omega);
		scale0 = sin((1.0f - t) * omega) / sinom;
		scale1 = sin(t * omega) / sinom;
	}
	else 
	{
		// "from" and "to" quaternions are very close
		//  ... so we can do a linear interpolation
		scale0 = 1.0f - t;
		scale1 = t;
	}

	// calculate final values
	qresult[0] = scale0 * ax + scale1 * bx;
	qresult[1] = scale0 * ay + scale1 * by;
	qresult[2] = scale0 * az + scale1 * bz;
	qresult[3] = scale0 * aw + scale1 * bw;

	qresult = normalize(qresult);

	return qresult;
}

mvMat4 
create_matrix(
	float m00, float m01, float m02, float m03,
	float m10, float m11, float m12, float m13,
	float m20, float m21, float m22, float m23,
	float m30, float m31, float m32, float m33
	)
{
	mvMat4 m{};

	// column 0
	m[0][0] = m00;
	m[0][1] = m10;
	m[0][2] = m20;
	m[0][3] = m30;

	// column 1
	m[1][0] = m01;
	m[1][1] = m11;
	m[1][2] = m21;
	m[1][3] = m31;

	// column 2
	m[2][0] = m02;
	m[2][1] = m12;
	m[2][2] = m22;
	m[2][3] = m32;

	// column 3
	m[3][0] = m03;
	m[3][1] = m13;
	m[3][2] = m23;
	m[3][3] = m33;

	return m;
}

mvMat4 
transpose(mvMat4& m)
{
	mvMat4 mr{};

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			mr[i][j] = m[j][i];
		}
	}

	return mr;
}

mvMat4
rotation_translation_scale(mvVec4& q, mvVec3& t, mvVec3& s)
{
	//mvMat4 m{};

	// Quaternion math
	float x = q[0];
	float y = q[1];
	float z = q[2];
	float w = q[3];
	float x2 = x + x;
	float y2 = y + y;
	float z2 = z + z;
	float xx = x * x2;
	float xy = x * y2;
	float xz = x * z2;
	float yy = y * y2;
	float yz = y * z2;
	float zz = z * z2;
	float wx = w * x2;
	float wy = w * y2;
	float wz = w * z2;
	float sx = s[0];
	float sy = s[1];
	float sz = s[2];

	mvMat4 m = create_matrix(
		(1.0f - (yy + zz)) * sx,
		(xy - wz) * sy,
		(xz + wy) * sz,
		t[0],
		(xy + wz) * sx,
		(1 - (xx + zz)) * sy,
		(yz - wx) * sz,
		t[1],
		(xz - wy) * sx,
		(yz + wx) * sy,
		(1.0f - (xx + yy)) * sz,
		t[2],
		0.0f,
		0.0f,
		0.0f,
		1.0f
	);


	return m;
}
#endif