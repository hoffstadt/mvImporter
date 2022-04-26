#pragma once

#include "sMath.h"
#include "mvGraphics.h"
#include "mvCamera.h"

// forward declarations
struct mvPointLight;
struct mvDirectionalLight;

struct mvPointLightInfo
{

    sVec4 viewLightPos = { 0.0f, 15.0f, 0.0f, 1.0f };
    //-------------------------- ( 16 bytes )

    sVec3 diffuseColor = { 1.0f, 1.0f, 1.0f };
    float  diffuseIntensity = 1.0f;
    //-------------------------- ( 16 bytes )

    float attConst = 1.0f;
    float attLin = 0.045f;
    float attQuad = 0.0075f;
    char  _pad1[4];
    //-------------------------- ( 16 bytes )

    //-------------------------- ( 4*16 = 64 bytes )
};

struct mvDirectionLightInfo
{

    float  diffuseIntensity = 1.0f;
    sVec3 viewLightDir = { 0.0f, -1.0f, 0.0f };
    //-------------------------- ( 16 bytes )

    sVec3 diffuseColor = { 1.0f, 1.0f, 1.0f };
    float  padding = 0.0f;
    //-------------------------- ( 16 bytes )

    //-------------------------- ( 2*16 = 32 bytes )
};

struct mvPointLight
{
    mvConstBuffer    buffer{};
    mvPointLightInfo info{};
    mvMesh           mesh{};
};

struct mvDirectionalLight
{
    mvConstBuffer        buffer{};
    mvDirectionLightInfo info{};
};

static mvPointLight
create_point_light(mvGraphics& graphics)
{
    mvPointLight light;

    // create mesh
    light.mesh = create_cube(graphics, 0.25f);

    // create constant buffer
    light.buffer = create_const_buffer(graphics, &light.info, sizeof(mvPointLightInfo));

    return light;
}

static mvDirectionalLight
create_directional_light(mvGraphics& graphics)
{
    mvDirectionalLight light;

    light.buffer = create_const_buffer(graphics, &light.info, sizeof(mvDirectionLightInfo));

    return light;
}