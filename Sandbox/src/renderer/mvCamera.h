#pragma once

#include "mvMath.h"

enum mvCameraType
{
    MV_CAMERA_PERSPECTIVE,
    MV_CAMERA_ORTHOGRAPHIC,
};

struct mvCamera
{
    mvCameraType type;
    mvVec3       pos;
    mvVec3       up          = {0.0f, 1.0f, 0.0f};
    f32          pitch       = 0.0f;
    f32          yaw         = 0.0f;
    f32          nearZ       = 0.1f;
    f32          farZ        = 400.0f;

    union
    {
        f32 aspectRatio;
        f32 width;
    };

    union
    {
        f32 fieldOfView;
        f32 height;
    };

    union
    {
        mvVec3 front;
        mvVec3 dir;
    };
};


mvCamera mvCreateOrthoCamera(mvVec3 pos, mvVec3 dir, f32 width, f32 height, f32 near, f32 far);
mvCamera mvCreatePerspectiveCamera(mvVec3 pos, f32 fov, f32 aspect, f32 near, f32 far);

mvMat4 mvCreateOrthoProjection (mvCamera& camera);
mvMat4 mvCreateLookAtProjection(mvCamera& camera);

mvMat4 mvCreateOrthoView       (mvCamera& camera);
mvMat4 mvCreateFPSView         (mvCamera& camera);
mvMat4 mvCreateLookAtView      (mvCamera& camera);

void mvUpdateCameraLookAtCamera(mvCamera& camera, f32 dt, f32 travelSpeed, f32 rotationSpeed);
void mvUpdateCameraFPSCamera   (mvCamera& camera, f32 dt, f32 travelSpeed, f32 rotationSpeed);
