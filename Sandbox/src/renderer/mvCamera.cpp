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
mvCreateOrthoCamera(mvVec3 pos, mvVec3 dir, f32 width, f32 height, f32 near, f32 far)
{
    mvCamera camera{};
    camera.type = MV_CAMERA_ORTHOGRAPHIC;
    camera.pos = pos;
    camera.dir = dir;
    camera.width = width;
    camera.height = height;
    camera.nearZ = near;
    camera.farZ = far;

    return camera;
}

mvCamera
mvCreatePerspectiveCamera(mvVec3 pos, f32 fov, f32 aspect, f32 near, f32 far)
{
    mvCamera camera{};
    camera.type = MV_CAMERA_PERSPECTIVE;
    camera.pos = pos;
    camera.aspectRatio = aspect;
    camera.fieldOfView = fov;
    camera.nearZ = near;
    camera.farZ = far;
    camera.front = { 0.0f, 0.0f, 1.0f };

    return camera;
}

mvMat4 
mvCreateFPSView(mvCamera& camera)
{
    return mvFPSViewRH(camera.pos, camera.pitch, camera.yaw);
}

mvMat4
mvCreateOrthoView(mvCamera& camera)
{
    return mvLookAtRH(camera.pos, camera.pos + camera.dir, camera.up);
}

mvMat4
mvCreateOrthoProjection(mvCamera& camera)
{
    return mvOrthoRH(-camera.width/2.0f, camera.width / 2.0f, -camera.height / 2.0f, -camera.height / 2.0f, camera.nearZ, camera.farZ);
}

mvMat4
mvCreateLookAtView(mvCamera& camera)
{
    mvVec3 direction{};
    direction.x = cos((camera.yaw)) * cos((camera.pitch));
    direction.y = sin((camera.pitch));
    direction.z = sin((camera.yaw)) * cos((camera.pitch));
    direction = mvNormalize(direction);
    return mvLookAtRH(camera.pos, camera.pos + direction, camera.up);
}

mvMat4
mvCreateLookAtProjection(mvCamera& camera)
{
    return mvPerspectiveRH(camera.fieldOfView, camera.aspectRatio, camera.nearZ, camera.farZ);
}

void 
mvUpdateCameraFPSCamera(mvCamera& camera, f32 dt, f32 travelSpeed, f32 rotationSpeed)
{
    // for now, we will just use imgui's input
    if (ImGui::GetIO().KeysDown['W'])
    {
        camera.pos.x = camera.pos.x - dt * travelSpeed * sin(camera.yaw);
        camera.pos.z = camera.pos.z - dt * travelSpeed * cos(camera.yaw);
    }

    if (ImGui::GetIO().KeysDown['S'])
    {
        camera.pos.x = camera.pos.x + dt * travelSpeed * sin(camera.yaw);
        camera.pos.z = camera.pos.z + dt * travelSpeed * cos(camera.yaw);
    }

    if (ImGui::GetIO().KeysDown['A'])
    {
        camera.pos.x = camera.pos.x - dt * travelSpeed * cos(camera.yaw);
        camera.pos.z = camera.pos.z + dt * travelSpeed * sin(camera.yaw);
    }

    if (ImGui::GetIO().KeysDown['D'])
    {
        camera.pos.x = camera.pos.x + dt * travelSpeed * cos(camera.yaw);
        camera.pos.z = camera.pos.z - dt * travelSpeed * sin(camera.yaw);
    }

    if (ImGui::GetIO().KeysDown['R'])
    {
        camera.pos.y = camera.pos.y + dt * travelSpeed;
    }

    if (ImGui::GetIO().KeysDown['F'])
    {
        camera.pos.y = camera.pos.y - dt * travelSpeed;
    }

    if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && ImGui::GetIO().WantCaptureMouse)
    {

        camera.yaw = wrap_angle(camera.yaw + -ImGui::GetIO().MouseDelta.x * rotationSpeed);
        camera.pitch = std::clamp(camera.pitch + -ImGui::GetIO().MouseDelta.y * rotationSpeed, 0.995f * -PI / 2.0f, 0.995f * PI / 2.0f);
    }
}

static void
mvTranslateCamera(mvCamera& camera, f32 dx, f32 dy, f32 dz, f32 travelSpeed)
{
    mvVec3 direction{};
    direction.x = cos((camera.yaw)) * cos((camera.pitch));
    direction.y = sin((camera.pitch));
    direction.z = sin((camera.yaw)) * cos((camera.pitch));
    direction = mvNormalize(direction);
    camera.front = direction;

    if (dz != 0.0f)
        camera.pos = camera.pos - camera.front * travelSpeed * dz;

    if (dx != 0.0f)
        camera.pos = camera.pos - mvNormalize(mvCross(camera.front, camera.up)) * travelSpeed * dx;

    if (dy != 0.0f)
        camera.pos.y += travelSpeed * dy;
}

void
mvUpdateCameraLookAtCamera(mvCamera& camera, f32 dt, f32 travelSpeed, f32 rotationSpeed)
{
    if (ImGui::GetIO().KeysDown['W']) mvTranslateCamera(camera, 0.0f, 0.0f, dt, travelSpeed);
    if (ImGui::GetIO().KeysDown['S']) mvTranslateCamera(camera, 0.0f, 0.0f, -dt, travelSpeed);
    if (ImGui::GetIO().KeysDown['D']) mvTranslateCamera(camera, dt, 0.0f, 0.0f, travelSpeed);
    if (ImGui::GetIO().KeysDown['A']) mvTranslateCamera(camera, -dt, 0.0f, 0.0f, travelSpeed);
    if (ImGui::GetIO().KeysDown['R']) mvTranslateCamera(camera, 0.0f, dt, 0.0f, travelSpeed);
    if (ImGui::GetIO().KeysDown['F']) mvTranslateCamera(camera, 0.0f, -dt, 0.0f, travelSpeed);

    if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && !ImGui::GetIO().WantCaptureMouse)
    {

        camera.yaw = wrap_angle(camera.yaw + ImGui::GetIO().MouseDelta.x * rotationSpeed);
        camera.pitch = std::clamp(camera.pitch + ImGui::GetIO().MouseDelta.y * rotationSpeed, 0.995f * -PI / 2.0f, 0.995f * PI / 2.0f);
    }
}