#pragma once

#include <string>
#include "mvViewport.h"
#include "mvGraphics.h"
#include "mvTypes.h"

struct mvIO
{
    std::string shaderDirectory;
    std::string resourceDirectory;
};

struct mvContext
{
    mvIO       IO;
    mvGraphics graphics;
};

extern mvContext* GContext;

void mvCreateContext();
void mvDestroyContext();