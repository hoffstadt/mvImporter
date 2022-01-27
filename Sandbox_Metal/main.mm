// NOTE: not even close to ready

#include "mvGraphics.h"
#include "mvPipeline.h"
#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>

@interface HelloMetalView : MTKView
@property mvGraphics* graphics;
@property mvPipeline pipeline;
@end

struct mvPass
{
    MTL::Drawable* drawable = nullptr;
    MTL::RenderPipelineState* pipelineState = nullptr;
    double width = 0.0;
    double height = 0.0;
};

void main_loop(mvGraphics* graphics, mvPass pass, MTL::RenderPassDescriptor* currentRenderPassDescriptor)
{
    // Wait for an available uniform buffer.
    dispatch_semaphore_wait(graphics->semaphore, DISPATCH_TIME_FOREVER);

    // Animation.
    float rad = 0.00f;
    float sin = std::sin(rad), cos = std::cos(rad);
    simd::float4x4 rot(simd::float4{cos, -sin, 0.0f, 0.0f},
                       simd::float4{sin, cos, 0, 0},
                       simd::float4{0, -0, 1, 0},
                       simd::float4{0, 0, 0, 1});

    // Update the current uniform buffer.
    graphics->uniformBufferIndex = (graphics->uniformBufferIndex + 1) % uniformBufferCount;
    auto uniforms = (FrameUniforms *)graphics->uniformBuffers[graphics->uniformBufferIndex]->contents();
    uniforms->projectionViewModel = rot;

    // Create a command buffer.
    MTL::CommandBuffer* commandBuffer = graphics->commandQueue->commandBuffer();

    // Encode render command.
    MTL::RenderCommandEncoder* encoder = commandBuffer->renderCommandEncoder(currentRenderPassDescriptor);
    encoder->setViewport({0, 0, pass.width, pass.height, 0, 1});
    encoder->setDepthStencilState(graphics->depthState);
    encoder->setRenderPipelineState(reinterpret_cast<const MTL::RenderPipelineState *>(pass.pipelineState));
    encoder->setVertexBuffer(graphics->uniformBuffers[graphics->uniformBufferIndex], 0, FrameUniformBuffer);
    encoder->setVertexBuffer(graphics->vertexBuffer, 0, MeshVertexBuffer);
    encoder->drawPrimitives(MTL::PrimitiveTypeTriangle, (NS::UInteger)0u, 3);
    encoder->endEncoding();

    // Set callback for semaphore.
    dispatch_semaphore_t  semaphore = graphics->semaphore;
     MTL::HandlerFunction callback = [semaphore](MTL::CommandBuffer* buffer){ dispatch_semaphore_signal(semaphore);};
    commandBuffer->addCompletedHandler(callback);
    commandBuffer->presentDrawable(pass.drawable);
    commandBuffer->commit();
}

int main ()
{
    @autoreleasepool {
        // Application.
        [NSApplication sharedApplication];
        [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
        [NSApp activateIgnoringOtherApps:YES];

        // Window.
        NSRect frame = NSMakeRect(0, 0, 256, 256);
        NSWindow *window = [[NSWindow alloc] initWithContentRect:frame styleMask:NSWindowStyleMaskTitled |
                                                                                 NSWindowStyleMaskClosable |
                                                                                 NSWindowStyleMaskResizable |
                                                                                 NSWindowStyleMaskMiniaturizable backing:NSBackingStoreBuffered defer:NO];
        //[window cascadeTopLeftFromPoint:NSMakePoint(20, 20)];
        window.title = [[NSProcessInfo processInfo] processName];
        [window orderFront:nil];
        [window center];
        [window becomeKeyWindow];

        // Custom MTKView.
        HelloMetalView *view = [[HelloMetalView alloc] initWithFrame:frame];
        window.contentView = view;

        // Run.
        [NSApp run];
    }
}

// The main view.
@implementation HelloMetalView
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
    self.graphics = setup_graphics((__bridge MTL::Device*)self.device, self.sampleCount);

    mvPipelineSpec spec{};
    spec.sampleCount = self.sampleCount;
    spec.vertexShader = "../../Sandbox_Metal/shaders/shaders.vert.metal";
    spec.pixelShader = "../../Sandbox_Metal/shaders/shaders.frag.metal";
    self.pipeline = finalize_pipeline(*self.graphics, spec);
}

- (void)drawRect:(CGRect)rect
{
    mvPass pass{};
    pass.width = self.drawableSize.width;
    pass.height = self.drawableSize.height;
    pass.drawable = (__bridge MTL::Drawable*)self.currentDrawable;
    pass.pipelineState = self.pipeline.pipelineState;
    main_loop(self.graphics, pass, (__bridge MTL::RenderPassDescriptor*)self.currentRenderPassDescriptor);

    // Draw children.
    [super drawRect:rect];
}

@end