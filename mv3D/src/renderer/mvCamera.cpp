#include "mvCamera.h"
#include <algorithm>

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

mvMat4
mvBuildCameraMatrix(mvOrthoCamera& camera)
{
    return mvLookAtRH(camera.pos, camera.pos + camera.dir, camera.up);
}

mvMat4
mvBuildProjectionMatrix(mvOrthoCamera& camera)
{
    return mvOrthoRH(camera.left, camera.right, camera.bottom, camera.top, camera.nearZ, camera.farZ);
}

mvMat4
mvBuildCameraMatrix(mvCamera& camera)
{
    //mvMat4 roll_pitch_yaw = mvYawPitchRoll(camera.yaw, camera.pitch, 0.0f);
    //mvVec4 forward_base_vector = { 0.0f, 0.0f, 1.0f, 0.0f };
    //mvVec4 look_vector = roll_pitch_yaw * forward_base_vector;
    //mvVec3 lpos = { look_vector.x, look_vector.y, look_vector.z };

    mvVec3 direction{};
    direction.x = cos((camera.yaw)) * cos((camera.pitch));
    direction.y = sin((camera.pitch));
    direction.z = sin((camera.yaw)) * cos((camera.pitch));
    direction = mvNormalize(direction);
    return mvLookAtRH(camera.pos, camera.pos + direction, camera.up);
}

mvMat4
mvBuildProjectionMatrix(mvCamera& camera)
{
    return mvPerspectiveRH(mvRadians(45.0f), camera.aspect, 0.1f, 400.0f);
}

void
mvRotateCamera(mvCamera& camera, f32 dx, f32 dy)
{
    camera.yaw = wrap_angle(camera.yaw + dx * CameraRotationSpeed);
    camera.pitch = std::clamp(camera.pitch + dy * CameraRotationSpeed, 0.995f * -PI / 2.0f, 0.995f * PI / 2.0f);
}

void
mvTranslateCamera(mvCamera& camera, f32 dx, f32 dy, f32 dz)
{
    mvVec3 direction{};
    direction.x = cos((camera.yaw)) * cos((camera.pitch));
    direction.y = sin((camera.pitch));
    direction.z = sin((camera.yaw)) * cos((camera.pitch));
    direction = mvNormalize(direction);
    camera.front = direction;

    if (dz != 0.0f)
        camera.pos = camera.pos - camera.front * CameraTravelSpeed*dz;

    if (dx != 0.0f)
        camera.pos = camera.pos - mvNormalize(mvCross(camera.front, camera.up)) * CameraTravelSpeed*dx;

    if (dy != 0.0f)
        camera.pos.y += CameraTravelSpeed * dy;
}
