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
create_ortho_camera(mvVec3 pos, mvVec3 dir, f32 width, f32 height, f32 nearZ, f32 farZ)
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
create_perspective_camera(mvVec3 pos, f32 fov, f32 aspect, f32 nearZ, f32 farZ)
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
    mvMat4 xRot = rotate(identity_mat4(), camera.xRot, mvVec3{ 1.0f, 0.0f, 0.0f });
    mvMat4 yRot = rotate(identity_mat4(), camera.yRot, mvVec3{ 0.0f, 1.0f, 0.0f });
    mvMat4 trans = translate(identity_mat4(), mvVec3{ camera.target.x, camera.target.y, camera.target.z - camera.radius });
    mvMat4 view = trans * xRot * yRot;
    return view;
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
update_fps_camera(mvCamera& camera, f32 dt, f32 travelSpeed, f32 rotationSpeed)
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

static int 
sgn(f32 val) {
    return (0.0f < val) - (val < 0.0f);
}

void
update_arcball_camera(mvCamera& camera, f32 dt, f32 translationSpeed, f32 rotationSpeed, f32 width, f32 height)
{

    if (!ImGui::GetIO().WantCaptureMouse)
        return;

    if (ImGui::IsMouseDragging(ImGuiMouseButton_Middle))
    {
        f32 xPosDelta = ImGui::GetIO().MouseDelta.x;
        f32 yPosDelta = ImGui::GetIO().MouseDelta.y;

        mvMat4 view = create_arcball_view(camera);
        mvVec4 worldDelta{ xPosDelta*translationSpeed*dt, -yPosDelta*translationSpeed*dt, 0.0f, 0.0f };
        mvMat4 projMatrix = create_projection(camera);

        worldDelta = invert(projMatrix) * invert(view) * worldDelta;

        camera.target.x += worldDelta.x;
        camera.target.y += worldDelta.y;
        camera.target.z += worldDelta.z;

        camera.pos.z = camera.target.z + camera.radius * cos(camera.pitch) * cos(camera.yaw);
        camera.pos.y = camera.target.y + camera.radius * sin(camera.pitch);
        camera.pos.x = camera.target.x + camera.radius * cos(camera.xRot) * -sin(camera.yaw);

    }

    if (ImGui::GetIO().MouseWheel != 0.0f)
    {
        camera.radius += ImGui::GetIO().MouseWheel * camera.radius * 0.1f;
        camera.radius = clamp(camera.radius, 0.0f, FLT_MAX);

        camera.farZ = abs(camera.radius * 2.0F);
        camera.nearZ = camera.radius / 100.0f;
    }

    // for now, we will just use imgui's input
    if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
    {
        f32 xPosDelta = ImGui::GetIO().MouseDelta.x;
        f32 yPosDelta = ImGui::GetIO().MouseDelta.y;

        float deltaAngleX = (2 * M_PI / width); // a movement from left to right = 2*PI = 360 deg
        float deltaAngleY = (M_PI / height);  // a movement from top to bottom = PI = 180 deg

        camera.yaw = wrap_angle(camera.yaw + deltaAngleX*xPosDelta * rotationSpeed);
        camera.pitch = std::clamp(camera.pitch + deltaAngleY*yPosDelta * rotationSpeed, 0.995f * -PI / 2.0f, 0.995f * PI / 2.0f);
        camera.pos.z = camera.target.z + camera.radius * cos(camera.pitch) * cos(camera.yaw);
        camera.pos.y = camera.target.y + camera.radius * sin(camera.pitch);
        camera.pos.x = camera.target.x + camera.radius * cos(camera.xRot) * -sin(camera.yaw);
    }
    else if (ImGui::GetIO().MouseWheel != 0 || ImGui::IsMouseDragging(ImGuiMouseButton_Middle))
    {
        camera.pos.z = camera.target.z + camera.radius * cos(camera.pitch) * cos(camera.yaw);
        camera.pos.y = camera.target.y + camera.radius * sin(camera.pitch);
        camera.pos.x = camera.target.x + camera.radius * cos(camera.xRot) * -sin(camera.yaw);
    }

}

static void
mvTranslateCamera(mvCamera& camera, f32 dx, f32 dy, f32 dz, f32 travelSpeed)
{
    mvVec3 direction{};
    direction.x = cos((camera.yaw)) * cos((camera.pitch));
    direction.y = sin((camera.pitch));
    direction.z = sin((camera.yaw)) * cos((camera.pitch));
    direction = normalize(direction);
    camera.front = direction;

    if (dz != 0.0f)
        camera.pos = camera.pos - camera.front * travelSpeed * dz;

    if (dx != 0.0f)
        camera.pos = camera.pos - normalize(cross(camera.front, camera.up)) * travelSpeed * dx;

    if (dy != 0.0f)
        camera.pos.y += travelSpeed * dy;
}

void
update_lookat_camera(mvCamera& camera, f32 dt, f32 travelSpeed, f32 rotationSpeed)
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