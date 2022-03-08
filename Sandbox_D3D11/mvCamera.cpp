#include "mvCamera.h"
#include <algorithm>
#include <imgui.h>

template<typename T>
static T
wrap_angle(T theta) noexcept
{
    constexpr T twoPi = (T)2 * (T)PI_D;
    const T mod = (T)fmod(theta, twoPi);
    if (mod > (T)PI_D)
        return mod - twoPi;
    else if (mod < -(T)PI_D)
        return mod + twoPi;
    return mod;
}

mvCamera 
create_ortho_camera(mvVec3 pos, mvVec3 dir, float width, float height, float nearZ, float farZ)
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
create_perspective_camera(mvVec3 pos, float fov, float aspect, float nearZ, float farZ)
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

mvMat4 
create_fps_view(mvCamera& camera)
{
    return fps(camera.pos, camera.pitch, camera.yaw);
}

mvMat4
create_ortho_view(mvCamera& camera)
{
    return lookat(camera.pos, camera.pos + camera.dir, camera.up);
}

mvMat4
create_projection(mvCamera& camera)
{
    if(camera.type == MV_CAMERA_ORTHOGRAPHIC)
        return ortho(-camera.width/2.0f, camera.width / 2.0f, -camera.height / 2.0f, -camera.height / 2.0f, camera.nearZ, camera.farZ);
    return perspective(camera.fieldOfView, camera.aspectRatio, camera.nearZ, camera.farZ);
}

mvMat4
create_arcball_view(mvCamera& camera)
{
    mvMat4 xRot = rotate(identity_mat4(), camera.pitch, mvVec3{ 1.0f, 0.0f, 0.0f });
    mvMat4 yRot = rotate(identity_mat4(), camera.yaw, mvVec3{ 0.0f, 1.0f, 0.0f });
    mvMat4 trans = translate(identity_mat4(), mvVec3{ camera.pos.x, camera.pos.y, camera.pos.z });
    mvMat4 view = trans * (yRot * xRot);
    return invert(view);
}

mvMat4
create_lookat_view(mvCamera& camera)
{
    mvVec3 direction{};
    direction.x = cos((camera.yaw)) * cos((camera.pitch));
    direction.y = sin((camera.pitch));
    direction.z = sin((camera.yaw)) * cos((camera.pitch));
    direction = normalize(direction);
    return lookat(camera.pos, camera.pos + direction, camera.up);
}

void
update_arcball_camera(mvCamera& camera, float dt)
{

    if (!ImGui::GetIO().WantCaptureMouse)
        return;

    if (ImGui::GetIO().MouseWheel != 0.0f)
    {
        mvMat4 xRot = rotate(identity_mat4(), camera.pitch, mvVec3{ 1.0f, 0.0f, 0.0f });
        mvMat4 yRot = rotate(identity_mat4(), camera.yaw, mvVec3{ 0.0f, 1.0f, 0.0f });
        mvMat4 trans = translate(identity_mat4(), mvVec3{ camera.pos.x, camera.pos.y, -camera.pos.z });
        mvMat4 view = trans * (yRot * xRot);


        float zoomDistance = pow(camera.distance/ camera.baseDistance, 1.0f / camera.zoomExponent);
        zoomDistance += camera.zoomFactor * ImGui::GetIO().MouseWheel;
        zoomDistance = get_max(zoomDistance, 0.0001f);
        camera.distance = pow(zoomDistance, camera.zoomExponent)* camera.baseDistance;

        // set distance from target
        mvVec3 lookDirection = normalize(mvVec3{-view.at(8), -view.at(9), -view.at(10) });
        mvVec3 distVec = mvVec3{ lookDirection.x * -camera.distance , lookDirection.y * -camera.distance , lookDirection.z * camera.distance };
        camera.pos = camera.target + distVec;
        camera.pos.z *= -1.0f;

        // fit camera planes
        float longestDistance = 10.0f * sqrt(camera.minBound.x * camera.minBound.x + camera.minBound.y * camera.minBound.y + camera.minBound.z * camera.minBound.z);
        camera.nearZ = camera.distance - longestDistance * 0.6;
        camera.farZ = camera.distance + longestDistance * 0.6;
        camera.nearZ = get_max(camera.nearZ, camera.farZ / 10000.0f);
    }

    // for now, we will just use imgui's input
    if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
    {
        float xPosDelta = ImGui::GetIO().MouseDelta.x;
        float yPosDelta = ImGui::GetIO().MouseDelta.y;

        camera.yaw = wrap_angle(camera.yaw - xPosDelta * camera.orbitSpeed);
        camera.pitch = std::clamp(camera.pitch - yPosDelta * camera.orbitSpeed, 0.995f * -PI / 2.0f, 0.995f * PI / 2.0f);

        float projlength = abs(camera.distance * cos(camera.pitch));
        camera.pos.x = camera.target.x - projlength * cos(camera.yaw + PI/2);
        camera.pos.y = camera.target.y - camera.distance * sin(camera.pitch);
        camera.pos.z = camera.target.z + projlength * sin(camera.yaw + PI/2);

    }
    //else if (ImGui::GetIO().MouseWheel != 0)
    //{
    //    float projlength = abs(camera.distance * cos(camera.pitch));
    //    camera.pos.x = camera.target.x - projlength * cos(camera.yaw + M_PI_2);
    //    camera.pos.y = camera.target.y - camera.distance * sin(camera.pitch);
    //    camera.pos.z = camera.target.z + projlength * sin(camera.yaw + M_PI_2);
    //}

}
