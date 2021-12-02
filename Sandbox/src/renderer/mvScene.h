#pragma once

#include <string>
#include "mvMath.h"
#include "mvBuffers.h"

struct mvNode
{
    std::string name;
    mvAssetID   mesh = -1;
    mvAssetID   children[256];
    u32         childCount = 0u;
    mvMat4      matrix      = mvIdentityMat4();
    mvVec3      translation = { 0.0f, 0.0f, 0.0f };
    mvVec4      rotation    = { 0.0f, 0.0f, 0.0f, 1.0f };
    mvVec3      scale       = { 1.0f, 1.0f, 1.0f };
};

struct mvScene
{
    mvAssetID     nodes[256];
    u32           nodeCount = 0u;
    u32           meshOffset = 0u;
};

