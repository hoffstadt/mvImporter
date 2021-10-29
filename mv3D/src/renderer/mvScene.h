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

struct mvSceneInfo
{

    mvVec3 ambientColor = { 0.04f, 0.04f, 0.04f };
    b32    useShadows = false;
    //-------------------------- ( 16 bytes )

    b32  useSkybox = true;
    char _pad[12];
    //-------------------------- ( 2*16 = 32 bytes )
};

struct mvScene
{
    mvConstBuffer buffer;
    mvSceneInfo   info;
    mvAssetID     nodes[256];
    u32           nodeCount = 0u;
};

mvScene mvCreateScene();
void    mvBindSlot_bPS(uint32_t slot, mvScene& scene);

