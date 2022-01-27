#include "mvRenderer.h"
#include "mvGraphics.h"

void
begin_frame(mvRendererContext& rctx)
{
    // Wait for an available uniform buffer.
    dispatch_semaphore_wait(rctx.graphics->semaphore, DISPATCH_TIME_FOREVER);
}

void
end_frame(mvRendererContext& rctx)
{

}