#pragma once

#include "mvMath.h"

struct mvCamera
{
    mvVec3 pos;
    mvVec3 front       = {0.0f, 0.0f, 1.0f};
    mvVec3 up          = {0.0f, 1.0f, 0.0f};
    f32    pitch       = 0.0f;
    f32    yaw         = 0.0f;
    f32    aspect      = 1.0f;
    f32    fieldOfView = (f32)M_PI_4;
    f32    nearZ       = 0.1f;
    f32    farZ        = 400.0f;
};

struct mvOrthoCamera
{
    mvVec3 pos    = { 0.0f, 100.0f, 0.0f };
    mvVec3 dir    = { 0.0f, -1.0f, 0.0f };
    mvVec3 up     = { 1.0f, 0.0f, 0.0f };
    f32    left   = -100.0f;
    f32    right  =  100.0f;
    f32    bottom = -100.0f;
    f32    top    =  100.0f;
    f32    nearZ  = -101.0f;
    f32    farZ   =  101.0f;
};

mvMat4 mvCreateOrthoProjection (mvOrthoCamera& camera);
mvMat4 mvCreateLookAtProjection(mvCamera& camera);

mvMat4 mvCreateOrthoView       (mvOrthoCamera& camera);
mvMat4 mvCreateFPSView         (mvCamera& camera);
mvMat4 mvCreateLookAtView      (mvCamera& camera);

void mvUpdateCameraLookAtCamera(mvCamera& camera, f32 dt, f32 travelSpeed, f32 rotationSpeed);
void mvUpdateCameraFPSCamera   (mvCamera& camera, f32 dt, f32 travelSpeed, f32 rotationSpeed);
