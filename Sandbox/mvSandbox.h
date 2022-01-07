#pragma once

#include <string>
#include "mvViewport.h"
#include "mvGraphics.h"
#include "mvTypes.h"

struct mvIO;
struct mvContext;

extern mvContext* GContext;

void create_context();
void destroy_context();

struct mvIO
{
    std::string shaderDirectory;
    std::string resourceDirectory;

    b8 punctualLighting = true;
    b8 imageBasedLighting = false;
    b8 clearcoat = true;
    b8 directionalShadows = true;
    b8 omniShadows = true;
};

struct mvContext
{
    mvIO       IO;
    mvGraphics graphics;
};

