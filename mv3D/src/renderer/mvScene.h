#pragma once

#include "mvMath.h"
#include "mvBuffers.h"

struct mvSceneInfo
{

    mvVec3 ambientColor = { 0.05f, 0.05f, 0.05f };
    b32    useShadows = true;
    //-------------------------- ( 16 bytes )

    b32  useSkybox = true;
    char _pad[12];
    //-------------------------- ( 2*16 = 32 bytes )
};

struct mvScene
{
    mvConstBuffer buffer;
    mvSceneInfo   info;
};

mvScene mvCreateScene();
void    mvBindSlot_bPS(uint32_t slot, mvScene& scene);

