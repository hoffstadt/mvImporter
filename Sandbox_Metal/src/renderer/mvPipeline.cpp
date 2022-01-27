#include "mvPipeline.h"
#include "mvGraphics.h"
#include "mvFileIO.h"

static MTL::Library*
compile_shader(mvGraphics& graphics, const char* filepath)
{
    MTL::Library* library = nullptr;

    NS::Error* error = nullptr;
    unsigned fileSize = 0u;
    char* shaderSource = read_file(filepath, fileSize, "rb");
    NS::String* fileSource = NS::String::string(shaderSource, NS::ASCIIStringEncoding);
    library = graphics.device->newLibrary(fileSource, nullptr, &error);
    if (!library)
    {
        printf("Failed to load library. error: %s", error->localizedDescription()->utf8String());
        exit(0);
    }

    return library;
}

mvPipeline
finalize_pipeline(mvGraphics graphics, mvPipelineSpec& spec)
{
    mvPipeline pipeline{};
    pipeline.spec = spec;

    // load shaders
    pipeline.vertexLibrary = compile_shader(graphics, spec.vertexShader.c_str());
    pipeline.pixelLibrary = compile_shader(graphics, spec.pixelShader.c_str());

    MTL::Function* vertFunc = pipeline.vertexLibrary->newFunction(NS::String::string( "vert", NS::ASCIIStringEncoding ));
    MTL::Function* fragFunc = pipeline.pixelLibrary->newFunction(NS::String::string( "frag", NS::ASCIIStringEncoding ));

    // Create vertex descriptor.
    MTL::VertexDescriptor* vertDesc = MTL::VertexDescriptor::alloc()->init();
    vertDesc->attributes()->object(0)->setFormat(MTL::VertexFormatFloat3);
    vertDesc->attributes()->object(0)->setOffset(0);
    vertDesc->attributes()->object(0)->setBufferIndex(0);
    vertDesc->attributes()->object(1)->setFormat(MTL::VertexFormatUChar4);
    vertDesc->attributes()->object(1)->setOffset(sizeof(Vertex::position));
    vertDesc->attributes()->object(1)->setBufferIndex(0);
    vertDesc->layouts()->object(0)->setStride(sizeof(Vertex));
    vertDesc->layouts()->object(0)->setStepRate(1);
    vertDesc->layouts()->object(0)->setStepFunction(MTL::VertexStepFunctionPerVertex);

    // Create pipeline state.
    NS::Error* error = nullptr;
    MTL::RenderPipelineDescriptor* pipelineDesc = MTL::RenderPipelineDescriptor::alloc()->init();
    pipelineDesc->setSampleCount(spec.sampleCount);
    pipelineDesc->setVertexFunction(vertFunc);
    pipelineDesc->setFragmentFunction(fragFunc);
    pipelineDesc->setVertexDescriptor(vertDesc);
    pipelineDesc->colorAttachments()->object(0)->setPixelFormat(MTL::PixelFormatBGRA8Unorm);
    pipelineDesc->setDepthAttachmentPixelFormat(MTL::PixelFormatDepth32Float_Stencil8);
    pipelineDesc->setStencilAttachmentPixelFormat(MTL::PixelFormatDepth32Float_Stencil8);
    pipeline.pipelineState = graphics.device->newRenderPipelineState(pipelineDesc, &error);

    return pipeline;
}

