#include "mvMath.h"

f32 
mvRadians(f32 degrees)
{
	return degrees * 0.01745329251994329576923690768489f;
}

f32&
mvVec2::operator[](i32 index)
{
	switch (index)
	{
	case 0: return x;
	case 1: return y;
	default: return y;
	}
}

f32&
mvVec3::operator[](i32 index)
{
	switch (index)
	{
	case 0: return x;
	case 1: return y;
	case 2: return z;
	default: return z;
	}
}

f32&
mvVec4::operator[](i32 index)
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
mvMat4::operator[](i32 index)
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

mvVec2
operator+(mvVec2& left, mvVec2& right)
{
	mvVec2 result = left;

	result.x += right.x;
	result.y += right.y;

	return result;
}

mvVec2
operator-(mvVec2& left, mvVec2& right)
{
	mvVec2 result = left;

	result.x -= right.x;
	result.y -= right.y;

	return result;
}

mvVec2
operator*(mvVec2& left, mvVec2& right)
{
	mvVec2 result = left;

	result.x *= right.x;
	result.y *= right.y;

	return result;
}

mvVec2
operator*(mvVec2& left, f32 right)
{
	mvVec2 result = left;

	result[0] *= right;
	result[1] *= right;

	return result;
}

mvVec3 
operator+(mvVec3& left, mvVec3& right)
{
	mvVec3 result = left;

	result.x += right.x;
	result.y += right.y;
	result.z += right.z;

	return result;
}

mvVec3 
operator-(mvVec3& left, mvVec3& right)
{
	mvVec3 result = left;

	result.x -= right.x;
	result.y -= right.y;
	result.z -= right.z;

	return result;
}

mvVec3 
operator*(mvVec3& left, mvVec3& right)
{
	mvVec3 result = left;

	result.x *= right.x;
	result.y *= right.y;
	result.z *= right.z;

	return result;
}

mvVec3
operator*(mvVec3& left, f32 right)
{
	mvVec3 result = left;

	result[0] *= right;
	result[1] *= right;
	result[2] *= right;

	return result;
}

mvVec4 
operator+(mvVec4& left, mvVec4& right)
{
	mvVec4 result = left;

	result.x += right.x;
	result.y += right.y;
	result.z += right.z;
	result.w += right.w;

	return result;
}

mvVec4 
operator-(mvVec4& left, mvVec4& right)
{
	mvVec4 result = left;

	result.x -= right.x;
	result.y -= right.y;
	result.z -= right.z;
	result.w -= right.w;

	return result;
}

mvVec4 
operator*(mvVec4& left, mvVec4& right)
{
	mvVec4 result = left;

	result.x = left.x * right.x;
	result.y = left.y * right.y;
	result.z = left.z * right.z;
	result.w = left.w * right.w;

	return result;
}

mvVec4 
operator*(mvMat4& left, mvVec4& right)
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

mvVec4 
operator*(mvVec4& left, f32 right)
{
	mvVec4 result = left;

	result[0] *= right;
	result[1] *= right;
	result[2] *= right;
	result[3] *= right;

	return result;
}

mvMat4 
operator*(mvMat4& left, mvMat4& right)
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
mvIdentityMat4()
{
	mvMat4 result{};

	result[0][0] = 1.0f;
	result[1][1] = 1.0f;
	result[2][2] = 1.0f;
	result[3][3] = 1.0f;

	return result;
}

mvMat4 
mvTranslate(mvMat4& m, mvVec3& v)
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
mvRotate(mvMat4& m, f32 angle, mvVec3& v)
{
	const f32 a = angle;
	const f32 c = cos(a);
	const f32 s = sin(a);

	mvVec3 axis = mvNormalize(v);
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
mvYawPitchRoll(f32 yaw, f32 pitch, f32 roll)
{
	f32 tmp_ch = cos(yaw);
	f32 tmp_sh = sin(yaw);
	f32 tmp_cp = cos(pitch);
	f32 tmp_sp = sin(pitch);
	f32 tmp_cb = cos(roll);
	f32 tmp_sb = sin(roll);

	mvMat4 result{};
	result[0][0] = tmp_ch * tmp_cb + tmp_sh * tmp_sp * tmp_sb;
	result[0][1] = tmp_sb * tmp_cp;
	result[0][2] = -tmp_sh * tmp_cb + tmp_ch * tmp_sp * tmp_sb;
	result[0][3] = 0.0f;
	result[1][0] = -tmp_ch * tmp_sb + tmp_sh * tmp_sp * tmp_cb;
	result[1][1] = tmp_cb * tmp_cp;
	result[1][2] = tmp_sb * tmp_sh + tmp_ch * tmp_sp * tmp_cb;
	result[1][3] = 0.0f;
	result[2][0] = tmp_sh * tmp_cp;
	result[2][1] = -tmp_sp;
	result[2][2] = tmp_ch * tmp_cp;
	result[2][3] = 0.0f;
	result[3][0] = 0.0f;
	result[3][1] = 0.0f;
	result[3][2] = 0.0f;
	result[3][3] = 1.0f;
	return result;
}

mvMat4 
mvScale(mvMat4& m, mvVec3& v)
{
	mvMat4 result{};
	result[0] = m[0] * v[0];
	result[1] = m[1] * v[1];
	result[2] = m[2] * v[2];
	result[3] = m[3];
	return result;
}

mvVec3 
mvNormalize(mvVec3& v)
{
	f32 length = sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
	mvVec3 result{};
	result.x = v.x / length;
	result.y = v.y / length;
	result.z = v.z / length;
	return result;
}

mvVec3 
mvCross(mvVec3& v1, mvVec3& v2)
{
	mvVec3 result{};
	result.x = v1.y * v2.z - v2.y * v1.z;
	result.y = v1.z * v2.x - v2.z * v1.x;
	result.z = v1.x * v2.y - v2.x * v1.y;
	return result;
}

f32
mvDot(mvVec3& v1, mvVec3& v2)
{
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

mvMat4 
mvLookAtLH(mvVec3& eye, mvVec3& center, mvVec3& up)
{
	mvVec3 f = mvNormalize(center - eye);
	mvVec3 s = mvNormalize(mvCross(up, f));
	mvVec3 u = mvCross(f, s);

	mvMat4 result = mvIdentityMat4();
	result[0][0] = s.x;
	result[1][0] = s.y;
	result[2][0] = s.z;
	result[0][1] = u.x;
	result[1][1] = u.y;
	result[2][1] = u.z;
	result[0][2] = f.x;
	result[1][2] = f.y;
	result[2][2] = f.z;
	result[3][0] = -mvDot(s, eye);
	result[3][1] = -mvDot(u, eye);
	result[3][2] = -mvDot(f, eye);
	return result;
}

mvMat4
mvLookAtRH(mvVec3& eye, mvVec3& center, mvVec3& up)
{
	mvVec3 f = mvNormalize(center - eye);
	mvVec3 s = mvNormalize(mvCross(f, up));
	mvVec3 u = mvCross(s, f);

	mvMat4 result = mvIdentityMat4();
	result[0][0] = s.x;
	result[1][0] = s.y;
	result[2][0] = s.z;
	result[0][1] = u.x;
	result[1][1] = u.y;
	result[2][1] = u.z;
	result[0][2] = -f.x;
	result[1][2] = -f.y;
	result[2][2] = -f.z;
	result[3][0] = -mvDot(s, eye);
	result[3][1] = -mvDot(u, eye);
	result[3][2] = mvDot(f, eye);
	return result;
}

mvMat4 
mvOrthoLH(f32 left, f32 right, f32 bottom, f32 top, f32 zNear, f32 zFar)
{
	mvMat4 result = mvIdentityMat4();
	result[0][0] = 2.0f / (right - left);
	result[1][1] = 2.0f / (top - bottom);
	result[2][2] = 2.0f / (zFar - zNear);
	result[3][0] = -(right + left) / (right - left);
	result[3][1] = -(top + bottom) / (top - bottom);
	result[3][2] = -(zFar + zNear) / (zFar - zNear);
	return result;
}

mvMat4 
mvPerspectiveLH(f32 fovy, f32 aspect, f32 zNear, f32 zFar)
{
	const f32 tanHalfFovy = tan(fovy / 2.0f);

	mvMat4 result{};
	result[0][0] = 1.0f / (aspect * tanHalfFovy);
	result[1][1] = 1.0f / (tanHalfFovy);
	result[2][2] = (zFar + zNear) / (zFar - zNear);
	result[2][3] = 1.0f;
	result[3][2] = -(2.0f * zFar * zNear) / (zFar - zNear);
	return result;
}

mvMat4 
mvSwitchHand(mvMat4& m)
{
	mvMat4 result = mvIdentityMat4();
	result[0][0] = m[0][0];
	result[1][0] = m[2][0];
	result[2][0] = m[1][0];
	result[3][0] = m[3][0];

	result[0][1] = m[0][1];
	result[1][1] = m[2][1];
	result[2][1] = m[1][1];
	result[3][1] = m[3][1];

	result[0][2] = m[0][2];
	result[1][2] = m[2][2];
	result[2][2] = m[1][2];
	result[3][2] = m[3][2];

	result[0][3] = m[0][3];
	result[1][3] = m[2][3];
	result[2][3] = m[1][3];
	result[3][3] = m[3][3];

	return result;
}