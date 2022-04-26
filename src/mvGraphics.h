#pragma once

#include <d3d11_1.h>
#include <d3d11.h>
#include <wrl.h>
#include <thread>
#include <vector>
#include <string>
#include "mvWindows.h"
#include "sGltf.h"
#include "sMath.h"

typedef int mvAssetID;
typedef int mvVertexElement;

// forward declarations
struct mvViewport;
struct mvTransforms;
struct mvModel;
struct mvRenderJob;
struct mvRendererContext;
struct mvCamera;
struct mvBuffer;
struct mvConstBuffer;
struct mvGLTFModel;
struct mvMeshPrimitive;
struct mvMesh;
struct mvSkin;
struct mvNode;
struct mvTexture;
struct mvCubeTexture;
struct mvEnvironment;
struct mvVertexLayout;
struct mvPipelineInfo;
struct mvPipeline;
struct mvPixelShader;
struct mvVertexShader;
struct mvComputeShader;
struct mvShaderOptions;
struct mvShaderMacro;
struct mvGraphics;
struct mvScene;

// graphics
mvGraphics setup_graphics    (mvViewport& viewport, const char* shaderDirectory);
void       recreate_swapchain(mvGraphics& graphics, unsigned width, unsigned height);
void       set_pipeline_state(mvGraphics& graphics, mvPipeline& pipeline);

// meshes
mvMesh create_cube         (mvGraphics& graphics, float size = 1.0f);
mvMesh create_textured_cube(mvGraphics& graphics, float size = 1.0f);
mvMesh create_textured_quad(mvGraphics& graphics, float size = 1.0f);
mvMesh create_frustum      (mvGraphics& graphics, float width, float height, float nearZ, float farZ);
mvMesh create_frustum2     (mvGraphics& graphics, float fov, float aspect, float nearZ, float farZ);
mvMesh create_ortho_frustum(mvGraphics& graphics, float width, float height, float nearZ, float farZ);

// buffers
mvBuffer      create_buffer      (mvGraphics& graphics, void* data, unsigned int size, D3D11_BIND_FLAG flags, unsigned int stride = 0u, unsigned int miscFlags = 0u);
mvConstBuffer create_const_buffer(mvGraphics& graphics, void* data, unsigned int size);
void          update_const_buffer(mvGraphics& graphics, mvConstBuffer& buffer, void* data);

// environments
mvEnvironment create_environment(mvGraphics& graphics, const std::string& path, int resolution, int sampleCount, float lodBias, int mipLevels);
void          cleanup_environment(mvEnvironment& environment);

// textures
mvTexture     create_texture     (mvGraphics& graphics, const std::string& path);
mvTexture     create_texture     (mvGraphics& graphics, unsigned char* data, unsigned int dataSize);
mvCubeTexture create_cube_texture(mvGraphics& graphics, const std::string& path);
mvTexture     create_dynamic_texture(mvGraphics& graphics, unsigned int width, unsigned int height, unsigned int arraySize = 1);
mvTexture     create_texture(mvGraphics& graphics, unsigned int width, unsigned int height, unsigned int arraySize = 1, float* data = nullptr);
void          update_dynamic_texture(mvGraphics& graphics, mvTexture& texture, unsigned int width, unsigned int height, float* data);

// pipelines
mvPipeline      finalize_pipeline             (mvGraphics& graphics, mvPipelineInfo& info);
mvVertexLayout  create_vertex_layout          (std::vector<mvVertexElement> elements);
mvVertexElement get_element_from_gltf_semantic(const char* semantic);
mvPixelShader   create_pixel_shader           (mvGraphics& graphics, const std::string& path, std::vector<D3D_SHADER_MACRO>* macros = nullptr);
mvVertexShader  create_vertex_shader          (mvGraphics& graphics, const std::string& path, mvVertexLayout& layout, std::vector<D3D_SHADER_MACRO>* macros = nullptr);
mvComputeShader create_compute_shader         (mvGraphics& graphics, const std::string& path, std::vector<D3D_SHADER_MACRO>* macros = nullptr);


// renderer
mvRendererContext create_renderer_context(mvGraphics& graphics);
void              submit_scene     (mvGraphics& graphics, mvModel& model, mvRendererContext& ctx, mvScene& scene);
void              render_scenes    (mvGraphics& graphics, mvModel& model, mvRendererContext& ctx, sMat4 cam, sMat4 proj);
void              render_skybox    (mvGraphics& graphics, mvRendererContext& rendererCtx, mvModel& model, mvCubeTexture& cubemap, ID3D11SamplerState* sampler, sMat4 cam, sMat4 proj);
void              render_mesh_solid(mvGraphics& graphics, mvRendererContext& rendererCtx, mvModel& model, mvMesh& mesh, sMat4 transform, sMat4 cam, sMat4 proj);

enum mvVertexElement_
{
	Position2D,
	Position3D,
	TexCoord0,
	TexCoord1,
	Color3_0,
	Color3_1,
	Color4_0,
	Color4_1,
	Normal,
	Tangent,
	Joints0,
	Joints1,
	Weights0,
	Weights1,
};

struct mvTransforms
{
	sMat4 model               = sMat4(1.0f);
	sMat4 modelView           = sMat4(1.0f);
	sMat4 modelViewProjection = sMat4(1.0f);
};

struct mvNode
{
    std::string  name;
    mvAssetID    skin   = -1;
    mvAssetID    mesh   = -1;
    mvAssetID    camera = -1;
    mvAssetID    children[256];
    unsigned int childCount = 0u;
    sMat4       matrix               = sMat4(1.0f);
    sVec3       translation          = { 0.0f, 0.0f, 0.0f };
    sVec4       rotation             = { 0.0f, 0.0f, 0.0f, 1.0f };
    sVec3       scale                = { 1.0f, 1.0f, 1.0f };
    sVec3       animationTranslation = { 0.0f, 0.0f, 0.0f };
    sVec4       animationRotation    = { 0.0f, 0.0f, 0.0f, 1.0f };
    sVec3       animationScale       = { 1.0f, 1.0f, 1.0f };
    bool         translationAnimated = false;
    bool         rotationAnimated = false;
    bool         scaleAnimated = false;
    bool         animated = false;

    sMat4      transform = sMat4(1.0f);
    sMat4      worldTransform = sMat4(1.0f);
    sMat4      inverseWorldTransform = sMat4(1.0f);
};

struct mvScene
{
    mvAssetID    nodes[256];
    unsigned int nodeCount = 0u;
    unsigned int meshOffset = 0u;
};

struct mvVertexLayout
{
	unsigned int                          elementCount;
	unsigned int                          size;
	std::vector<unsigned int>             indices;
	std::vector<std::string>              semantics;
	std::vector<DXGI_FORMAT>              formats;
	std::vector<D3D11_INPUT_ELEMENT_DESC> d3dLayout;
};

struct mvShaderMacro
{
    std::string macro;
    std::string value;
};

struct mvShaderOptions
{
    std::vector<D3D_SHADER_MACRO> macros;
    bool hasAlbedoMap = false;
    bool hasNormalMap = false;
    bool hasMetallicRoughnessMap = false;
    bool hasEmmissiveMap = false;
    bool hasOcculusionMap = false;
    bool hasClearcoatMap = false;
    bool hasClearcoatNormalMap = false;
    bool hasClearcoatRoughnessMap = false;
    bool hasSpecularMap = false;
    bool hasSpecularColorMap = false;
    bool hasSpecularGlossinessMap = false;
    bool hasSheenColorMap = false;
    bool hasSheenRoughnessMap = false;
    bool hasTransmissionMap = false;
    bool hasThicknessMap = false;

    bool materialClearCoat = false;
    bool materialMetallicRoughness = false;
    bool materialSpecularGlossiness = false;
    bool materialSheen = false;
    bool materialSpecular = false;
    bool materialTransmission = false;
    bool materialVolume = false;
    bool materialIor = false;

    bool materialUnlit = false;
    bool linearOutput = false;
    bool useIBL = false;
    bool usePunctual = false;

    int alphaMode;

};

struct mvPixelShader
{
    Microsoft::WRL::ComPtr<ID3D11PixelShader> shader;
    Microsoft::WRL::ComPtr<ID3DBlob>          blob;
    std::string                               path;
};

struct mvVertexShader
{
    Microsoft::WRL::ComPtr<ID3D11VertexShader> shader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout>  inputLayout;
    Microsoft::WRL::ComPtr<ID3DBlob>           blob;
    std::string                                path;
};

struct mvComputeShader
{
    Microsoft::WRL::ComPtr<ID3D11ComputeShader> shader;
    Microsoft::WRL::ComPtr<ID3DBlob>            blob;
    std::string                                 path;
};

struct mvPipelineInfo
{
    std::string                pixelShader;
    std::string                vertexShader;
    mvVertexLayout             layout;
    int                        depthBias;
    float                      slopeBias;
    float                      clamp;
    bool                       cull = true;
    std::vector<mvShaderMacro> macros;
};

struct mvPipeline
{
    Microsoft::WRL::ComPtr<ID3D11PixelShader>       pixelShader = nullptr;
    Microsoft::WRL::ComPtr<ID3D11VertexShader>      vertexShader = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob>                pixelBlob = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob>                vertexBlob = nullptr;
    Microsoft::WRL::ComPtr<ID3D11InputLayout>       inputLayout = nullptr;
    Microsoft::WRL::ComPtr<ID3D11BlendState>        blendState = nullptr;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilState = nullptr;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState>   rasterizationState = nullptr;
    D3D11_PRIMITIVE_TOPOLOGY                        topology;
    mvPipelineInfo                                  info;
};

struct mvTexture
{
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureView = nullptr;
    bool                                             alpha       = false;
    Microsoft::WRL::ComPtr<ID3D11SamplerState>       sampler     = nullptr;
};

struct mvCubeTexture
{
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureView = nullptr;
};

struct mvEnvironment
{
    Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler = nullptr;
    Microsoft::WRL::ComPtr<ID3D11SamplerState> brdfSampler = nullptr;
    mvCubeTexture                              skyMap;
    mvCubeTexture                              irradianceMap;
    Microsoft::WRL::ComPtr<ID3D11Texture2D>    specularTextureResource;
    mvCubeTexture                              specularMap;
    mvTexture                                  brdfLUT;
};

struct mvBuffer
{
	unsigned int                                      size;
	Microsoft::WRL::ComPtr<ID3D11Buffer>              buffer;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>  shaderResourceView = nullptr;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> unorderedAccessView = nullptr;
};

struct mvConstBuffer
{
	unsigned int                         size;
	Microsoft::WRL::ComPtr<ID3D11Buffer> buffer;
};

struct mvMeshPrimitive
{
    mvVertexLayout layout;
    mvBuffer       indexBuffer;
    mvBuffer       vertexBuffer;
    mvTexture      normalTexture;
    mvTexture      specularTexture;
    mvTexture      albedoTexture;
    mvTexture      emissiveTexture;
    mvTexture      occlusionTexture;
    mvTexture      metalRoughnessTexture;
    mvTexture      clearcoatTexture;
    mvTexture      clearcoatRoughnessTexture;
    mvTexture      clearcoatNormalTexture;
    mvTexture      morphTexture;
    mvAssetID      materialID = -1;
    float*         morphData = nullptr;
};

struct mvMesh
{
    std::string                  name;
    std::vector<mvMeshPrimitive> primitives;
    std::vector<float>           weights;
    std::vector<float>           weightsAnimated;
    unsigned int                 weightCount;
    mvConstBuffer                morphBuffer;
};

struct GlobalInfo
{

    sVec3 ambientColor = { 0.04f, 0.04f, 0.04f };
    int pcfRange = 0;
    //-------------------------- ( 16 bytes )

    sVec3 camPos;
    char _padding[4];
    //-------------------------- ( 16 bytes )
    
    sMat4 projection;
    sMat4 model;
    sMat4 view;
    //-------------------------- ( 192 bytes )

    //-------------------------- ( 124 bytes )
};

struct mvRenderJob
{
    mvMeshPrimitive*                     meshPrimitive = nullptr;
    sMat4                                accumulatedTransform = sMat4(1.0f);
    mvSkin*                              skin = nullptr;
    Microsoft::WRL::ComPtr<ID3D11Buffer> morphBuffer = nullptr;
};

struct mvRendererContext
{
    GlobalInfo               globalInfo{};
    mvConstBuffer            globalInfoBuffer;
    mvCamera*                camera = nullptr;
    std::vector<mvRenderJob> opaqueJobs;
    std::vector<mvRenderJob> transparentJobs;
    std::vector<mvRenderJob> wireframeJobs;
    ID3D11BlendState*        finalBlendState = nullptr;
    mvPipeline               skyboxPipeline;
    mvPipeline               solidPipeline;
    mvPipeline               solidWireframePipeline;
};

struct mvGraphics
{
    const char*                                    shaderDirectory;
    Microsoft::WRL::ComPtr<ID3D11Buffer>           tranformCBuf;
    Microsoft::WRL::ComPtr<ID3D11Texture2D>        frameBuffer;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> target;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> targetDepth;
    Microsoft::WRL::ComPtr<IDXGISwapChain>         swapChain;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext>    imDeviceContext;  
    Microsoft::WRL::ComPtr<ID3D11Device>           device;
    std::thread::id                                threadID;
    D3D11_VIEWPORT                                 viewport;

    // user options
    bool punctualLighting = true;
    bool imageBasedLighting = true;
    bool clearcoat = true;
};

bool operator==(mvVertexLayout& left, mvVertexLayout& right);
bool operator!=(mvVertexLayout& left, mvVertexLayout& right);