#pragma once

#include "mvMath.h"
#include "mvMesh.h"
#include "mvBuffers.h"
#include "mvBuffers.h"

struct mvAssetManager;

struct mvPointLightInfo
{

    mvVec4 viewLightPos = { 0.0f, 0.0f, 0.0f, 1.0f };
    //-------------------------- ( 16 bytes )

    mvVec3 diffuseColor = { 1.0f, 1.0f, 1.0f };
    f32    diffuseIntensity = 1.0f;
    //-------------------------- ( 16 bytes )

    f32  attConst = 1.0f;
    f32  attLin = 0.045f;
    f32  attQuad = 0.0075f;
    char _pad1[4];
    //-------------------------- ( 16 bytes )

    //-------------------------- ( 4*16 = 64 bytes )
};

struct mvDirectionLightInfo
{

    f32    diffuseIntensity = 1.0f;
    mvVec3 viewLightDir = { 0.0f, -1.0f, 0.0f };
    //-------------------------- ( 16 bytes )

    mvVec3 diffuseColor = { 1.0f, 1.0f, 1.0f };
    f32    padding = 0.0f;
    //-------------------------- ( 16 bytes )

    //-------------------------- ( 2*16 = 32 bytes )
};

struct mvPointLight
{
    mvConstBuffer    buffer;
    mvPointLightInfo info;
    mvMesh*          mesh;
};

struct mvDirectionLight
{
    mvConstBuffer        buffer;
    mvDirectionLightInfo info;
};

mvPointLight     mvCreatePointLight    (mvAssetManager* manager, mvVec3 pos = { 0.0f,0.0f,0.5f });
mvDirectionLight mvCreateDirectionLight(mvVec3 dir = { 0.0f,0.0f,0.5f });
    
void mvBindSlot_bPS(u32 slot, mvPointLight& light, mvMat4 viewMatrix);
void mvBindSlot_bPS(u32 slot, mvDirectionLight& light, mvMat4 viewMatrix);
