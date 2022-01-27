#pragma once

// forward declarations
struct mvRendererContext;
struct mvGraphics;

void begin_frame(mvRendererContext& rctx);
void end_frame(mvRendererContext& rctx);

struct mvRendererContext
{
    mvGraphics* graphics = nullptr; // stored for convenience
};

