#pragma once

#include <optional>
#include <vector>
#include "mvWindows.h"

struct mvViewport
{
    int  width   = 500;
    int  height  = 500;
    HWND hWnd    = nullptr;
    bool resized = false;    
};

mvViewport*        initialize_viewport(int width, int height);
std::optional<int> process_viewport_events();
