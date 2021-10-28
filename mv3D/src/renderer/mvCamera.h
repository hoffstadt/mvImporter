#pragma once

#include "mvMath.h"

mv_global constexpr f32 CameraTravelSpeed = 12.0f;
mv_global constexpr f32 CameraRotationSpeed = 0.004f;

struct mvCamera
{
    mvVec3 pos;
    f32    pitch  = 0.0f;
    f32    yaw    = 0.0f;
    f32    aspect = 1.0f;
};

struct mvOrthoCamera
{
    mvVec3 pos    = { 0.0f, 100.0f, 0.0f };
    mvVec3 dir    = { 0.0f, -1.0f, 0.0f };
    f32    left   = -100.0f;
    f32    right  =  100.0f;
    f32    bottom = -100.0f;
    f32    top    =  100.0f;
    f32    nearZ  = -101.0f;
    f32    farZ   =  101.0f;
};

mvMat4 mvBuildCameraMatrix    (mvOrthoCamera& camera);
mvMat4 mvBuildProjectionMatrix(mvOrthoCamera& camera);
mvMat4 mvBuildCameraMatrix    (mvCamera& camera);
mvMat4 mvBuildProjectionMatrix(mvCamera& camera);
void   mvRotateCamera         (mvCamera& camera, f32 dx, f32 dy);
void   mvTranslateCamera      (mvCamera& camera, f32 dx, f32 dy, f32 dz);
