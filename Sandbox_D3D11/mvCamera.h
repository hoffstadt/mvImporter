#pragma once

#include "mvMath.h"

// forward declarations
struct mvCamera;

enum mvCameraType
{
    MV_CAMERA_PERSPECTIVE,
    MV_CAMERA_ORTHOGRAPHIC,
};

mvCamera create_ortho_camera      (mvVec3 pos, mvVec3 dir, float width, float height, float nearZ, float farZ);
mvCamera create_perspective_camera(mvVec3 pos, float fov, float aspect, float nearZ, float farZ);
mvMat4   create_projection        (mvCamera& camera);                            
mvMat4   create_ortho_view        (mvCamera& camera);
mvMat4   create_fps_view          (mvCamera& camera);
mvMat4   create_arcball_view      (mvCamera& camera);        
mvMat4   create_lookat_view       (mvCamera& camera);        
void     update_arcball_camera    (mvCamera& camera, float dt);

struct mvCamera
{
    mvCameraType type;
    mvVec3       pos;
    mvVec3       up          = {0.0f, 1.0f, 0.0f};
    float          nearZ = 0.1f;
    float          farZ = 400.0f;

    float orbitSpeed = 1.0f / 180.0f;
    float panSpeed = 1.0f;
    float distance = 1.0f;
    float baseDistance = 1.0f;
    float zoomExponent = 5.0f;
    float zoomFactor = 0.01f;
    mvVec3 minBound = {0.0f, 1.0f, 0.0f};
    mvVec3 maxBound = {0.0f, 1.0f, 0.0f};

    union
    {
        float pitch = 0.0f;
        float xRot;
    };
    union
    {
        float yaw = 0.0f;
        float yRot;
    };
    union
    {
        float roll = 0.0f;
        float zRot;
    };

    union
    {
        float aspectRatio;
        float width;
    };

    union
    {
        float fieldOfView;
        float height;
    };

    union
    {
        mvVec3 front = { 0.0f, 0.0f, 0.0f };
        mvVec3 dir;
        mvVec3 target;
    };
};



