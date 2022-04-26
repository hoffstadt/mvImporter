#include "mvCamera.h"
#include <algorithm>
#include <imgui.h>

template<typename T>
static T
wrap_angle(T theta) noexcept
{
    constexpr T twoPi = (T)2 * (T)S_PI_D;
    const T mod = (T)fmod(theta, twoPi);
    if (mod > (T)S_PI_D)
        return mod - twoPi;
    else if (mod < -(T)S_PI_D)
        return mod + twoPi;
    return mod;
}

static sMat4
fps(sVec3 eye, float pitch, float yaw)
{

	// I assume the values are already converted to radians.
	float cosPitch = cos(pitch);
	float sinPitch = sin(pitch);
	float cosYaw = cos(yaw);
	float sinYaw = sin(yaw);

	sVec3 xaxis = { cosYaw, 0, -sinYaw };
	sVec3 yaxis = { sinYaw * sinPitch, cosPitch, cosYaw * sinPitch };
	sVec3 zaxis = { sinYaw * cosPitch, -sinPitch, cosPitch * cosYaw };

	sMat4 viewMatrix = sMat4(
		sVec4{ xaxis.x, yaxis.x, zaxis.x, 0 },
		sVec4{ xaxis.y, yaxis.y, zaxis.y, 0 },
		sVec4{ xaxis.z, yaxis.z, zaxis.z, 0 },
		sVec4{ -Semper::dot(xaxis, eye), -Semper::dot(yaxis, eye), -Semper::dot(zaxis, eye), 1 }
	);

	return viewMatrix;
}

static sMat4
lookat(sVec3 eye, sVec3 center, sVec3 up)
{
	sVec3 zaxis = Semper::normalize(center - eye);
	sVec3 xaxis = Semper::normalize(Semper::cross(up, zaxis));
	sVec3 yaxis = Semper::cross(zaxis, xaxis);

	sMat4 viewMatrix = sMat4(1.0f);

	// row 0
	viewMatrix[0][0] = xaxis.x;
	viewMatrix[1][0] = xaxis.y;
	viewMatrix[2][0] = xaxis.z;
	viewMatrix[3][0] = -Semper::dot(xaxis, eye);

	// row 1
	viewMatrix[0][1] = yaxis.x;
	viewMatrix[1][1] = yaxis.y;
	viewMatrix[2][1] = yaxis.z;
	viewMatrix[3][1] = -Semper::dot(yaxis, eye);

	// row 2
	viewMatrix[0][2] = zaxis.x;
	viewMatrix[1][2] = zaxis.y;
	viewMatrix[2][2] = zaxis.z;
	viewMatrix[3][2] = -Semper::dot(zaxis, eye);

	return viewMatrix;
}

static sMat4
perspective(float fovy, float aspect, float zNear, float zFar)
{
	const float tanHalfFovy = tan(fovy / 2.0f);

	sMat4 result{};
	result[0][0] = 1.0f / (aspect * tanHalfFovy);
	result[1][1] = 1.0f / (tanHalfFovy);
	result[2][2] = -zFar/ (zFar - zNear);
	result[2][3] = -1.0f;
	result[3][2] = -(zFar * zNear) / (zFar - zNear);
	return result;
}

static sMat4
ortho(float left, float right, float bottom, float top, float zNear, float zFar)
{
	sMat4 result = sMat4(1.0f);
	result[0][0] = 2.0f / (right - left);
	result[1][1] = 2.0f / (top - bottom);
	result[2][2] = -2.0f / (zFar - zNear);
	result[3][0] = -(right + left) / (right - left);
	result[3][1] = -(top + bottom) / (top - bottom);
	result[3][2] = -(zFar + zNear) / (zFar - zNear);
	return result;
}

mvCamera 
create_ortho_camera(sVec3 pos, sVec3 dir, float width, float height, float nearZ, float farZ)
{
    mvCamera camera{};
    camera.type = MV_CAMERA_ORTHOGRAPHIC;
    camera.pos = pos;
    camera.dir = dir;
    camera.width = width;
    camera.height = height;
    camera.nearZ = nearZ;
    camera.farZ = farZ;
    return camera;
}

mvCamera
create_perspective_camera(sVec3 pos, float fov, float aspect, float nearZ, float farZ)
{
    mvCamera camera{};
    camera.type = MV_CAMERA_PERSPECTIVE;
    camera.pos = pos;
    camera.aspectRatio = aspect;
    camera.fieldOfView = fov;
    camera.nearZ = nearZ;
    camera.farZ = farZ;
    camera.front = { 0.0f, 0.0f, -1.0f };
    return camera;
}

sMat4
create_projection(mvCamera& camera)
{
    if(camera.type == MV_CAMERA_ORTHOGRAPHIC)
        return ortho(-camera.width/2.0f, camera.width / 2.0f, -camera.height / 2.0f, -camera.height / 2.0f, camera.nearZ, camera.farZ);
    return perspective(camera.fieldOfView, camera.aspectRatio, camera.nearZ, camera.farZ);
}

sMat4
create_arcball_view(mvCamera& camera)
{
    sMat4 xRot = Semper::rotate(camera.pitch, sVec3{ 1.0f, 0.0f, 0.0f });
    sMat4 yRot = Semper::rotate(camera.yaw, sVec3{ 0.0f, 1.0f, 0.0f });
    sMat4 trans = Semper::translate(camera.pos.x, camera.pos.y, camera.pos.z);
    sMat4 view = trans * (yRot * xRot);
    return Semper::invert(view);
}

sMat4
create_lookat_view(mvCamera& camera)
{
    sVec3 direction{};
    direction.x = cos((camera.yaw)) * cos((camera.pitch));
    direction.y = sin((camera.pitch));
    direction.z = sin((camera.yaw)) * cos((camera.pitch));
    direction = Semper::normalize(direction);
    return lookat(camera.pos, camera.pos + direction, camera.up);
}

sMat4
create_ortho_view(mvCamera& camera) 
{ 
    return lookat(camera.pos, camera.pos + camera.dir, camera.up);
}

sMat4
create_fps_view(mvCamera& camera)
{
    return fps(camera.pos, camera.pitch, camera.yaw);
}

void
update_arcball_camera(mvCamera& camera, float dt)
{

    if (!ImGui::GetIO().WantCaptureMouse)
        return;

    if (ImGui::GetIO().MouseWheel != 0.0f)
    {
        sMat4 xRot = Semper::rotate(camera.pitch, sVec3{ 1.0f, 0.0f, 0.0f });
        sMat4 yRot = Semper::rotate(camera.yaw, sVec3{ 0.0f, 1.0f, 0.0f });
        sMat4 trans = Semper::translate(camera.pos.x, camera.pos.y, -camera.pos.z);
        sMat4 view = trans * (yRot * xRot);


        float zoomDistance = pow(camera.distance/ camera.baseDistance, 1.0f / camera.zoomExponent);
        zoomDistance += camera.zoomFactor * ImGui::GetIO().MouseWheel;
        zoomDistance = Semper::get_max(zoomDistance, 0.0001f);
        camera.distance = pow(zoomDistance, camera.zoomExponent)* camera.baseDistance;

        // set distance from target
        sVec3 lookDirection = Semper::normalize(sVec3{-view.at(8), -view.at(9), -view.at(10) });
        sVec3 distVec = sVec3{ lookDirection.x * -camera.distance , lookDirection.y * -camera.distance , lookDirection.z * camera.distance };
        camera.pos = camera.target + distVec;
        camera.pos.z *= -1.0f;

        // fit camera planes
        float longestDistance = 10.0f * sqrt(camera.minBound.x * camera.minBound.x + camera.minBound.y * camera.minBound.y + camera.minBound.z * camera.minBound.z);
        camera.nearZ = camera.distance - longestDistance * 0.6;
        camera.farZ = camera.distance + longestDistance * 0.6;
        camera.nearZ = Semper::get_max(camera.nearZ, camera.farZ / 10000.0f);
    }

    // for now, we will just use imgui's input
    if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
    {
        float xPosDelta = ImGui::GetIO().MouseDelta.x;
        float yPosDelta = ImGui::GetIO().MouseDelta.y;

        camera.yaw = wrap_angle(camera.yaw - xPosDelta * camera.orbitSpeed);
        camera.pitch = Semper::clamp(0.995f * -S_PI / 2.0f, camera.pitch - yPosDelta * camera.orbitSpeed, 0.995f * S_PI / 2.0f);

        float projlength = abs(camera.distance * cos(camera.pitch));
        camera.pos.x = camera.target.x - projlength * cos(camera.yaw + S_PI/2);
        camera.pos.y = camera.target.y - camera.distance * sin(camera.pitch);
        camera.pos.z = camera.target.z + projlength * sin(camera.yaw + S_PI/2);

    }

}
