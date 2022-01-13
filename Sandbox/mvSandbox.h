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
    b8 imageBasedLighting = true;
    b8 clearcoat = true;
};

struct mvContext
{
    mvIO       IO;
    mvGraphics graphics;
};

