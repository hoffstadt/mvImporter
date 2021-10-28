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
    return mvLookAtLH(camera.pos, camera.pos + camera.dir, mvVec3{ 0.0f, 0.0f, 1.0f });
}

mvMat4
mvBuildProjectionMatrix(mvOrthoCamera& camera)
{
    return mvOrthoLH(camera.left, camera.right, camera.bottom, camera.top, camera.nearZ, camera.farZ);
}

mvMat4
mvBuildCameraMatrix(mvCamera& camera)
{
    mvMat4 roll_pitch_yaw = mvYawPitchRoll(camera.yaw, camera.pitch, 0.0f);
    mvVec4 forward_base_vector = { 0.0f, 0.0f, 1.0f, 0.0f };
    mvVec4 look_vector = roll_pitch_yaw * forward_base_vector;
    mvVec3 lpos = { look_vector.x, look_vector.y, look_vector.z };
    mvVec3 look_target = camera.pos + lpos;
    mvMat4 camera_matrix = mvLookAtLH(camera.pos, look_target, mvVec3{ 0.0f, 1.0f, 0.0f });
    return camera_matrix;
}

mvMat4
mvBuildProjectionMatrix(mvCamera& camera)
{
    return mvPerspectiveLH(mvRadians(45.0f), camera.aspect, 0.1f, 400.0f);
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
    mvMat4 roll_pitch_yaw = mvYawPitchRoll(camera.yaw, camera.pitch, 0.0f);
    mvMat4 scale = mvScale(mvIdentityMat4(), mvVec3{ CameraTravelSpeed, CameraTravelSpeed, CameraTravelSpeed });
    mvVec4 translation = { dx, dy, dz, 0.0f };

    translation = (roll_pitch_yaw * scale) * translation;

    camera.pos = {
        camera.pos.x + translation.x,
        camera.pos.y + translation.y,
        camera.pos.z + translation.z
    };
}
