#pragma once

#include "mvMath.h"

// forward declarations
struct mvCamera;

enum mvCameraType
{
    MV_CAMERA_PERSPECTIVE,
    MV_CAMERA_ORTHOGRAPHIC,
};

mvCamera create_ortho_camera      (mvVec3 pos, mvVec3 dir, f32 width, f32 height, f32 nearZ, f32 farZ);
mvCamera create_perspective_camera(mvVec3 pos, f32 fov, f32 aspect, f32 nearZ, f32 farZ);
mvMat4   create_projection        (mvCamera& camera);                            
mvMat4   create_ortho_view        (mvCamera& camera);
mvMat4   create_fps_view          (mvCamera& camera);
mvMat4   create_arcball_view      (mvCamera& camera);        
mvMat4   create_lookat_view       (mvCamera& camera);        
void     update_arcball_camera    (mvCamera& camera, f32 dt);

struct mvCamera
{
    mvCameraType type;
    mvVec3       pos;
    mvVec3       up          = {0.0f, 1.0f, 0.0f};
    f32          nearZ = 0.1f;
    f32          farZ = 400.0f;

    f32 orbitSpeed = 1.0f / 180.0f;
    f32 panSpeed = 1.0f;
    f32 distance = 1.0f;
    f32 baseDistance = 1.0f;
    f32 zoomExponent = 5.0f;
    f32 zoomFactor = 0.01f;
    mvVec3 minBound = {0.0f, 1.0f, 0.0f};
    mvVec3 maxBound = {0.0f, 1.0f, 0.0f};

    union
    {
        f32 pitch = 0.0f;
        f32 xRot;
    };
    union
    {
        f32 yaw = 0.0f;
        f32 yRot;
    };
    union
    {
        f32 roll = 0.0f;
        f32 zRot;
    };

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
        mvVec3 front = { 0.0f, 0.0f, 0.0f };
        mvVec3 dir;
        mvVec3 target;
    };
};



