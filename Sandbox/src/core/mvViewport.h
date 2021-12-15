#pragma once

#include <optional>
#include <vector>
#include "mvWindows.h"
#include "mvTypes.h"

struct mvViewport
{
    i32  width   = 500;
    i32  height  = 500;
    HWND hWnd    = nullptr;
    b8   resized = false;    
};

mvViewport*        initialize_viewport(i32 width, i32 height);
std::optional<i32> process_viewport_events();
