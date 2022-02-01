// NOTE: not even close to ready

#include "mvGraphics.h"
#include "mvPipeline.h"
#include "Semper/sSemper.h"

struct TempUserData
{
    mvGraphics* graphics;
    mvPipeline pipeline;
};

void main_loop(sApplePass pass,  MTL::RenderPassDescriptor* currentRenderPassDescriptor)
{

    auto userData = (TempUserData*)Semper::get_io().userData;
    mvGraphics* graphics = userData->graphics;
    MTL::RenderPipelineState* pipelineState = userData->pipeline.pipelineState;

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
    encoder->setRenderPipelineState(reinterpret_cast<const MTL::RenderPipelineState *>(pipelineState));
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
    Semper::create_context();

    @autoreleasepool {
        // Application.
        [NSApplication sharedApplication];
        [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
        [NSApp activateIgnoringOtherApps:YES];

        sWindow* window = Semper::create_window(256, 256);

        TempUserData userData;
        userData.graphics = setup_graphics((__bridge MTL::Device*)window->platform.view.device, window->platform.view.sampleCount);

        mvPipelineSpec spec{};
        spec.sampleCount = window->platform.view.sampleCount;
        spec.vertexShader = "../../Sandbox_Metal/shaders/shaders.vert.metal";
        spec.pixelShader = "../../Sandbox_Metal/shaders/shaders.frag.metal";
        userData.pipeline = finalize_pipeline(*userData.graphics, spec);

        sIO& io = Semper::get_io();
        io.userData = &userData;

        // Run.
        [NSApp run];
    }

    Semper::destroy_context();
}