#include "sSemper.h"

extern void main_loop(sApplePass pass, MTL::RenderPassDescriptor* currentRenderPassDescriptor);

// The main view.
@implementation SemperMetalView
{
}

- (id)initWithFrame:(CGRect)inFrame
{
    id<MTLDevice> device = MTLCreateSystemDefaultDevice();
    self = [super initWithFrame:inFrame device:device];
    if (self)
    {
        [self setup];
    }
    return self;
}

- (void)setup
{
    self.colorPixelFormat = MTLPixelFormatBGRA8Unorm;
    self.depthStencilPixelFormat = MTLPixelFormatDepth32Float_Stencil8;
}

- (void)drawRect:(CGRect)rect
{

    sApplePass pass{};
    pass.width = self.drawableSize.width;
    pass.height = self.drawableSize.height;
    pass.drawable = (__bridge MTL::Drawable*)self.currentDrawable;
    main_loop(pass, (__bridge MTL::RenderPassDescriptor*)self.currentRenderPassDescriptor);

    [super drawRect:rect];
}

@end

namespace Semper {

    sPlatformSpecifics
    initialize_platform(sWindow &window, int cwidth, int cheight)
    {
        sPlatformSpecifics specifics;

        // Window.
        NSRect frame = NSMakeRect(0, 0, 256, 256);
        specifics.handle = [[NSWindow alloc] initWithContentRect:frame styleMask:NSWindowStyleMaskTitled |
                                                                                 NSWindowStyleMaskClosable |
                                                                                 NSWindowStyleMaskResizable |
                                                                                 NSWindowStyleMaskMiniaturizable backing:NSBackingStoreBuffered defer:NO];
        //[window cascadeTopLeftFromPoint:NSMakePoint(20, 20)];
        specifics.handle.title = [[NSProcessInfo processInfo] processName];
        [specifics.handle orderFront:nil];
        [specifics.handle center];
        [specifics.handle becomeKeyWindow];

        // Custom MTKView.
        specifics.view = [[SemperMetalView alloc] initWithFrame:frame];
        specifics.handle.contentView = specifics.view;

        return specifics;
    }

    void
    cleanup_platform(sWindow &window)
    {

    }

    void
    process_platform_events(sWindow &window)
    {

    }

    void
    enable_platform_cursor(sWindow &window)
    {

    }

    void
    disable_platform_cursor(sWindow &window)
    {

    }
}