#include "mvGraphics.h"
#include "mvFileIO.h"

mvGraphics*
setup_graphics(MTL::Device* device, unsigned int sampleCount)
{
    auto graphics = new mvGraphics();
    graphics->device = device;

    // Create depth state.
    MTL::DepthStencilDescriptor* depthDesc = MTL::DepthStencilDescriptor::alloc()->init();
    depthDesc->setDepthCompareFunction(MTL::CompareFunctionLess);
    depthDesc->setDepthWriteEnabled(true);
    graphics->depthState = graphics->device->newDepthStencilState(depthDesc);

    // Create vertices.
    Vertex verts[] = {
            Vertex{{-0.5, -0.5, 0.0}, {255,   0,   0, 255}},
            Vertex{{ 0.0,  0.5, 0.0}, {  0, 255,   0, 255}},
            Vertex{{ 0.5, -0.5, 0.0}, {  0,   0, 255, 255}}
    };
    graphics->vertexBuffer = graphics->device->newBuffer(verts, sizeof(verts), MTL::ResourceStorageModePrivate);

    // Create uniform buffers.
    for (int i = 0; i < uniformBufferCount; i++)
    {
        graphics->uniformBuffers[i] = graphics->device->newBuffer(sizeof(FrameUniforms), MTL::ResourceCPUCacheModeWriteCombined);
    }
    graphics->frame = 0;

    // Create semaphore for each uniform buffer.
    graphics->semaphore = dispatch_semaphore_create(uniformBufferCount);
    graphics->uniformBufferIndex = 0;

    // Create command queue
    graphics->commandQueue = graphics->device->newCommandQueue()->retain();
    return graphics;
}