#pragma once

#include <string>
#include "mvMath.h"
#include "mvBuffers.h"
#include "mvMesh.h"

struct mvNode
{
    std::string name;
    mvAssetID   skin   = -1;
    mvAssetID   mesh   = -1;
    mvAssetID   camera = -1;
    mvAssetID   children[256];
    u32         childCount = 0u;
    mvMat4      matrix               = identity_mat4();
    mvVec3      translation          = { 0.0f, 0.0f, 0.0f };
    mvVec4      rotation             = { 0.0f, 0.0f, 0.0f, 1.0f };
    mvVec3      scale                = { 1.0f, 1.0f, 1.0f };
    mvVec3      animationTranslation = { 0.0f, 0.0f, 0.0f };
    mvVec4      animationRotation    = { 0.0f, 0.0f, 0.0f, 1.0f };
    mvVec3      animationScale       = { 1.0f, 1.0f, 1.0f };
    b8          translationAnimated = false;
    b8          rotationAnimated = false;
    b8          scaleAnimated = false;
    b8          animated = false;

    mvMat4      transform = identity_mat4();
    mvMat4      worldTransform = identity_mat4();
    mvMat4      inverseWorldTransform = identity_mat4();
};

struct mvScene
{
    mvAssetID nodes[256];
    u32       nodeCount = 0u;
    u32       meshOffset = 0u;
};

