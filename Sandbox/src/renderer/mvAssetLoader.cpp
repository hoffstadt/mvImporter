#include "mvAssetLoader.h"
#include "mvAssetManager.h"
#include "mvSandbox.h"
#include <unordered_map>

#define MV_IMPORTER_IMPLEMENTATION
#include "mvImporter.h"

static u8
mvGetAccessorItemCompCount(mvGLTFAccessor& accessor)
{
    switch (accessor.type)
    {
    case(MV_IMP_SCALAR): return 1u;
    case(MV_IMP_VEC2): return 2u;
    case(MV_IMP_VEC3): return 3u;
    case(MV_IMP_VEC4): return 4u;
    case(MV_IMP_MAT2): return 4u;
    case(MV_IMP_MAT3): return 9u;
    case(MV_IMP_MAT4): return 16u;
    default:
        assert(false && "Undefined attribute type");
        return 0u;
    }
}

static mvS32
mvGetBufferViewStride(mvGLTFModel& model, mvGLTFAccessor& accessor)
{
    mvGLTFBufferView& bufferview = model.bufferviews[accessor.buffer_view_index];

    u8 actualItemCompCount = mvGetAccessorItemCompCount(accessor);

    // calculate stride if not available
    if (bufferview.byte_stride == -1)
    {
        switch (accessor.component_type)
        {
        case MV_IMP_UNSIGNED_BYTE:
        case MV_IMP_BYTE: return 1 * actualItemCompCount;

        case MV_IMP_UNSIGNED_SHORT:
        case MV_IMP_SHORT: return 2 * actualItemCompCount;

        case MV_IMP_FLOAT:
        case MV_IMP_INT:
        case MV_IMP_UNSIGNED_INT: return 4 * actualItemCompCount;

        case MV_IMP_DOUBLE: return 8 * actualItemCompCount;
        }
    }
    else
        return bufferview.byte_stride;

}

template<typename T>
static T*
mvGetBufferViewStart(mvGLTFModel& model, mvGLTFAccessor& accessor)
{
    mvGLTFBufferView& bufferview = model.bufferviews[accessor.buffer_view_index];
    mvVerifyBufferViewStride(model, accessor);
    return (T*)&model.buffers[bufferview.buffer_index].data[bufferview.byte_offset + accessor.byteOffset];
}

template<typename T, typename W>
static void
mvFillBufferAsType(mvGLTFModel& model, mvGLTFAccessor& accessor, std::vector<W>& outBuffer, u32 componentCap)
{
    mvS32 bufferviewStride = mvGetBufferViewStride(model, accessor);
    mvGLTFBufferView bufferView = model.bufferviews[accessor.buffer_view_index];
    char* bufferRawData = (char*)model.buffers[bufferView.buffer_index].data.data();
    char* bufferRawSection = &bufferRawData[bufferView.byte_offset + accessor.byteOffset]; // start of buffer section

    u8 actualItemCompCount = mvGetAccessorItemCompCount(accessor);

    u8 accessorItemCompCount = actualItemCompCount;
    if (componentCap < accessorItemCompCount)
        accessorItemCompCount = componentCap;

    for (i32 i = 0; i < accessor.count; i++)
    {
        T* values = (T*)&bufferRawSection[i * bufferviewStride];

        for (i32 j = 0; j < accessorItemCompCount; j++)
            outBuffer.push_back((W)values[j]);
    }
}

template<typename W>
static void
mvFillBuffer(mvGLTFModel& model, mvGLTFAccessor& accessor, std::vector<W>& outBuffer, u32 componentCap = 4)
{
    mvGLTFComponentType indexCompType = accessor.component_type;

    switch (indexCompType)
    {
    case MV_IMP_BYTE:
    case MV_IMP_UNSIGNED_BYTE:
        mvFillBufferAsType<u8>(model, accessor, outBuffer, componentCap);
        break;

    case MV_IMP_SHORT:
        mvFillBufferAsType<s16>(model, accessor, outBuffer, componentCap);
        break;

    case MV_IMP_UNSIGNED_SHORT:
        mvFillBufferAsType<u16>(model, accessor, outBuffer, componentCap);
        break;

    case MV_IMP_INT:
        mvFillBufferAsType<i32>(model, accessor, outBuffer, componentCap);
        break;

    case MV_IMP_UNSIGNED_INT:
        mvFillBufferAsType<u32>(model, accessor, outBuffer, componentCap);
        break;

    case MV_IMP_FLOAT:
        mvFillBufferAsType<f32>(model, accessor, outBuffer, componentCap);
        break;

    case MV_IMP_DOUBLE:
        mvFillBufferAsType<f64>(model, accessor, outBuffer, componentCap);
        break;

    default:
        assert(false && "Unknown index compenent type");
    }
}

static D3D11_TEXTURE_ADDRESS_MODE
get_address_mode(mvS32 address)
{
    switch (address)
    {
    case MV_IMP_WRAP_CLAMP_TO_EDGE:   return D3D11_TEXTURE_ADDRESS_CLAMP;
    case MV_IMP_WRAP_MIRRORED_REPEAT: return D3D11_TEXTURE_ADDRESS_MIRROR;
    case MV_IMP_WRAP_REPEAT:          return D3D11_TEXTURE_ADDRESS_WRAP;
    default:                          return D3D11_TEXTURE_ADDRESS_WRAP;
    }
}

static D3D11_FILTER
get_filter_mode(mvS32 minFilter, mvS32 magFilter)
{
    //return D3D11_FILTER_MIN_MAG_MIP_POINT;
    if (magFilter == MV_IMP_FILTER_LINEAR)
    {
        switch (minFilter)
        {

        case MV_IMP_FILTER_LINEAR_MIPMAP_NEAREST:  return D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
        case MV_IMP_FILTER_NEAREST_MIPMAP_LINEAR:  return D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR;

        case MV_IMP_FILTER_LINEAR:
        case MV_IMP_FILTER_LINEAR_MIPMAP_LINEAR:   return D3D11_FILTER_MIN_MAG_MIP_LINEAR;

        case MV_IMP_FILTER_NEAREST:
        case MV_IMP_FILTER_NEAREST_MIPMAP_NEAREST:
        default:                                   return D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
        }
    }
    else // MV_IMP_FILTER_NEAREST
    {
        switch (minFilter)
        {
        case MV_IMP_FILTER_LINEAR_MIPMAP_NEAREST:  return D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
        case MV_IMP_FILTER_NEAREST_MIPMAP_LINEAR:  return D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
        case MV_IMP_FILTER_LINEAR:
        case MV_IMP_FILTER_LINEAR_MIPMAP_LINEAR:   return D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
        case MV_IMP_FILTER_NEAREST_MIPMAP_NEAREST:
        case MV_IMP_FILTER_NEAREST:
        default:                                   return D3D11_FILTER_MIN_MAG_MIP_POINT;
        }
    }
}

static mvAssetID
setup_texture(mvAssetManager& assetManager, mvGLTFModel& model, u32 currentPrimitive, mvS32 textureID, b8& flag, std::string& name, std::string suffix)
{
    if (textureID == -1)
        return -1;
    mvGLTFTexture& texture = model.textures[textureID];
    std::string uri = model.images[texture.image_index].uri;
    mvAssetID resultID = -1;
    if (model.images[texture.image_index].embedded)
    {
        resultID = mvGetTextureAssetID(&assetManager, model.root + name + std::to_string(currentPrimitive) + uri + suffix, model.images[texture.image_index].data);
    }
    else
        resultID = mvGetTextureAssetID(&assetManager, model.root + name + std::to_string(currentPrimitive) + uri + suffix, model.root + uri);
    flag = true;
    if (texture.sampler_index > -1)
    {
        mvGLTFSampler& sampler = model.samplers[texture.sampler_index];
        mvTexture& newTexture = assetManager.textures[resultID].asset;

        // Create Sampler State
        D3D11_SAMPLER_DESC samplerDesc{};
        samplerDesc.AddressU = get_address_mode(sampler.wrap_s);
        samplerDesc.AddressV = get_address_mode(sampler.wrap_t);
        samplerDesc.AddressW = samplerDesc.AddressV;
        samplerDesc.Filter = get_filter_mode(sampler.min_filter, sampler.mag_filter);
        samplerDesc.BorderColor[0] = 0.0f;
        samplerDesc.MaxAnisotropy = D3D11_REQ_MAXANISOTROPY;
        samplerDesc.MinLOD = -FLT_MAX;
        samplerDesc.MaxLOD = FLT_MAX;

        HRESULT hResult = GContext->graphics.device->CreateSamplerState(&samplerDesc, &newTexture.sampler);
        assert(SUCCEEDED(hResult));
    }

    return resultID;
}

static std::vector<mvAssetID>
load_gltf_skins(mvAssetManager& assetManager, mvGLTFModel& model)
{
    std::vector<mvAssetID> skinMapping;
    skinMapping.resize(model.skin_count);

    for (u32 currentSkin = 0u; currentSkin < model.skin_count; currentSkin++)
    {
        mvGLTFSkin& glskin = model.skins[currentSkin];
        mvSkin skin{};

        skin.skeleton = glskin.skeleton;
        skin.jointCount = glskin.joints_count;
        for (u32 joint = 0; joint < skin.jointCount; joint++)
        {
            skin.joints[joint] = glskin.joints[joint];
        }

        mvFillBuffer(model, model.accessors[glskin.inverseBindMatrices], skin.inverseBindMatrices, 16);

        u32 textureWidth = ceil(sqrt(skin.jointCount * 8));

        skin.jointTexture = create_dynamic_texture(textureWidth, textureWidth);
        skin.textureData = new f32[textureWidth * textureWidth * 4];

        skinMapping[currentSkin] = register_asset(&assetManager, glskin.name, skin);
    }

    return skinMapping;
}

static std::vector<mvAssetID>
load_gltf_cameras(mvAssetManager& assetManager, mvGLTFModel& model)
{
    std::vector<mvAssetID> cameraMapping;
    cameraMapping.resize(model.camera_count);

    for (u32 currentCamera = 0u; currentCamera < model.camera_count; currentCamera++)
    {
        mvGLTFCamera& glcamera = model.cameras[currentCamera];
        mvCamera camera{};

        if (glcamera.type == MV_IMP_PERSPECTIVE)
        {
            camera.type = MV_CAMERA_PERSPECTIVE;
            camera.aspectRatio = glcamera.perspective.aspectRatio;
            camera.farZ = glcamera.perspective.zfar;
            camera.nearZ = glcamera.perspective.znear;
            camera.fieldOfView = glcamera.perspective.yfov;
        }
        else
        {
            camera.type = MV_CAMERA_ORTHOGRAPHIC;
            camera.farZ = glcamera.orthographic.zfar;
            camera.nearZ = glcamera.orthographic.znear;
            camera.width = glcamera.orthographic.xmag * 2.0f;
            camera.height = glcamera.orthographic.ymag * 2.0f;
        }

        cameraMapping[currentCamera] = register_asset(&assetManager, glcamera.name, camera);
    }

    return cameraMapping;
}

struct RawAttributeBuffers
{
    std::vector<f32> positionAttributeBuffer;
    std::vector<f32> tangentAttributeBuffer;
    std::vector<f32> normalAttributeBuffer;
    std::vector<f32> texture0AttributeBuffer;
    std::vector<f32> texture1AttributeBuffer;
    std::vector<f32> color0AttributeBuffer;
    std::vector<f32> color1AttributeBuffer;
    std::vector<f32> joints0AttributeBuffer;
    std::vector<f32> joints1AttributeBuffer;
    std::vector<f32> weights0AttributeBuffer;
    std::vector<f32> weights1AttributeBuffer;

    b8 hasColor0Vec3 = false;
    b8 hasColor0Vec4 = false;
    b8 hasColor1Vec3 = false;
    b8 hasColor1Vec4 = false;
};

static std::vector<mvVertexElement>
load_raw_attribute_buffers(mvGLTFModel& model, mvGLTFMeshPrimitive& glprimitive, RawAttributeBuffers& rawBuffers)
{
    std::vector<mvVertexElement> attributes;
    for (u32 i = 0; i < glprimitive.attribute_count; i++)
    {
        auto& attribute = glprimitive.attributes[i];
        if (strcmp(attribute.semantic.c_str(), "POSITION") == 0)
        {
            attributes.push_back(mvVertexElement::Position3D);
            mvFillBuffer(model, model.accessors[attribute.index], rawBuffers.positionAttributeBuffer);
        }
        else if (strcmp(attribute.semantic.c_str(), "NORMAL") == 0)
        {
            attributes.push_back(mvVertexElement::Normal);
            mvFillBuffer(model, model.accessors[attribute.index], rawBuffers.normalAttributeBuffer, 3);
        }
        else if (strcmp(attribute.semantic.c_str(), "TANGENT") == 0)
        {
            attributes.push_back(mvVertexElement::Tangent);
            mvFillBuffer(model, model.accessors[attribute.index], rawBuffers.tangentAttributeBuffer, 4);
        }
        else if (strcmp(attribute.semantic.c_str(), "JOINTS_0") == 0)
        {
            attributes.push_back(mvVertexElement::Joints0);
            mvFillBuffer(model, model.accessors[attribute.index], rawBuffers.joints0AttributeBuffer, 4);
        }
        else if (strcmp(attribute.semantic.c_str(), "JOINTS_1") == 0)
        {
            attributes.push_back(mvVertexElement::Joints1);
            mvFillBuffer(model, model.accessors[attribute.index], rawBuffers.joints1AttributeBuffer, 4);
        }
        else if (strcmp(attribute.semantic.c_str(), "WEIGHTS_0") == 0)
        {
            attributes.push_back(mvVertexElement::Weights0);
            mvFillBuffer(model, model.accessors[attribute.index], rawBuffers.weights0AttributeBuffer, 4);
        }
        else if (strcmp(attribute.semantic.c_str(), "WEIGHTS_1") == 0)
        {
            attributes.push_back(mvVertexElement::Weights1);
            mvFillBuffer(model, model.accessors[attribute.index], rawBuffers.weights1AttributeBuffer, 4);
        }
        else if (strcmp(attribute.semantic.c_str(), "TEXCOORD_0") == 0)
        {
            attributes.push_back(mvVertexElement::TexCoord0);
            mvFillBuffer<f32>(model, model.accessors[attribute.index], rawBuffers.texture0AttributeBuffer, 2);
        }
        else if (strcmp(attribute.semantic.c_str(), "TEXCOORD_1") == 0)
        {
            attributes.push_back(mvVertexElement::TexCoord1);
            mvFillBuffer<f32>(model, model.accessors[attribute.index], rawBuffers.texture1AttributeBuffer, 2);
        }
        else if (strcmp(attribute.semantic.c_str(), "COLOR_0") == 0)
        {
            mvGLTFAccessor& accessor = model.accessors[attribute.index];
            if (accessor.type == MV_IMP_VEC3)
            {
                rawBuffers.hasColor0Vec3 = true;
                rawBuffers.hasColor0Vec4 = false;
                attributes.push_back(mvVertexElement::Color3_0);
                mvFillBuffer(model, model.accessors[attribute.index], rawBuffers.color0AttributeBuffer, 3);
            }
            else if (accessor.type == MV_IMP_VEC4)
            {
                rawBuffers.hasColor0Vec3 = false;
                rawBuffers.hasColor0Vec4 = true;
                attributes.push_back(mvVertexElement::Color4_0);
                mvFillBuffer(model, model.accessors[attribute.index], rawBuffers.color0AttributeBuffer, 4);
            }
            else
            {
                assert(false && "Undefined attribute type");
            }

        }
        else if (strcmp(attribute.semantic.c_str(), "COLOR_1") == 0)
        {
            mvGLTFAccessor& accessor = model.accessors[attribute.index];
            if (accessor.type == MV_IMP_VEC3)
            {
                rawBuffers.hasColor1Vec3 = true;
                rawBuffers.hasColor1Vec4 = false;
                attributes.push_back(mvVertexElement::Color3_1);
                mvFillBuffer(model, model.accessors[attribute.index], rawBuffers.color1AttributeBuffer, 3);
            }
            else if (accessor.type == MV_IMP_VEC4)
            {
                rawBuffers.hasColor1Vec3 = false;
                rawBuffers.hasColor1Vec4 = true;
                attributes.push_back(mvVertexElement::Color4_1);
                mvFillBuffer(model, model.accessors[attribute.index], rawBuffers.color1AttributeBuffer, 4);
            }
            else
            {
                assert(false && "Undefined attribute type");
            }

        }
        else
        {
            assert(false && "Undefined attribute type");
        }
    }

    return attributes;
}

static void
combine_vertex_buffer(u32 triangleCount, mvVertexLayout& modifiedLayout, mvGLTFMeshPrimitive& glprimitive, std::vector<u32> origIndexBuffer, RawAttributeBuffers& rawBuffers, std::vector<u32>& indexBuffer, std::vector<f32>& combinedVertexBuffer)
{
    for (size_t i = 0; i < triangleCount / 3; i++)
    {
        size_t i0 = glprimitive.indices_index == -1 ? i : origIndexBuffer[i];

        for (size_t j = 0; j < modifiedLayout.semantics.size(); j++)
        {
            std::string semantic = modifiedLayout.semantics[j];

            if (strcmp(semantic.c_str(), "Position") == 0)
            {
                f32 x0 = rawBuffers.positionAttributeBuffer[i0 * 3];
                f32 y0 = rawBuffers.positionAttributeBuffer[i0 * 3 + 1];
                f32 z0 = rawBuffers.positionAttributeBuffer[i0 * 3 + 2];

                // position
                combinedVertexBuffer.push_back(x0);
                combinedVertexBuffer.push_back(y0);
                combinedVertexBuffer.push_back(z0);
            }
            else if (strcmp(semantic.c_str(), "Joints") == 0)
            {
                if (modifiedLayout.indices[j] == 0)
                {
                    f32 x0 = rawBuffers.joints0AttributeBuffer[i0 * 4];
                    f32 y0 = rawBuffers.joints0AttributeBuffer[i0 * 4 + 1];
                    f32 z0 = rawBuffers.joints0AttributeBuffer[i0 * 4 + 2];
                    f32 w0 = rawBuffers.joints0AttributeBuffer[i0 * 4 + 3];

                    // position
                    combinedVertexBuffer.push_back(x0);
                    combinedVertexBuffer.push_back(y0);
                    combinedVertexBuffer.push_back(z0);
                    combinedVertexBuffer.push_back(w0);
                }
                else
                {
                    f32 x0 = rawBuffers.joints1AttributeBuffer[i0 * 4];
                    f32 y0 = rawBuffers.joints1AttributeBuffer[i0 * 4 + 1];
                    f32 z0 = rawBuffers.joints1AttributeBuffer[i0 * 4 + 2];
                    f32 w0 = rawBuffers.joints1AttributeBuffer[i0 * 4 + 3];

                    // position
                    combinedVertexBuffer.push_back(x0);
                    combinedVertexBuffer.push_back(y0);
                    combinedVertexBuffer.push_back(z0);
                    combinedVertexBuffer.push_back(w0);
                }
            }
            else if (strcmp(semantic.c_str(), "Weights") == 0)
            {
                if (modifiedLayout.indices[j] == 0)
                {
                    f32 x0 = rawBuffers.weights0AttributeBuffer[i0 * 4];
                    f32 y0 = rawBuffers.weights0AttributeBuffer[i0 * 4 + 1];
                    f32 z0 = rawBuffers.weights0AttributeBuffer[i0 * 4 + 2];
                    f32 w0 = rawBuffers.weights0AttributeBuffer[i0 * 4 + 3];

                    // position
                    combinedVertexBuffer.push_back(x0);
                    combinedVertexBuffer.push_back(y0);
                    combinedVertexBuffer.push_back(z0);
                    combinedVertexBuffer.push_back(w0);
                }
                else
                {
                    f32 x0 = rawBuffers.weights1AttributeBuffer[i0 * 4];
                    f32 y0 = rawBuffers.weights1AttributeBuffer[i0 * 4 + 1];
                    f32 z0 = rawBuffers.weights1AttributeBuffer[i0 * 4 + 2];
                    f32 w0 = rawBuffers.weights1AttributeBuffer[i0 * 4 + 3];

                    // position
                    combinedVertexBuffer.push_back(x0);
                    combinedVertexBuffer.push_back(y0);
                    combinedVertexBuffer.push_back(z0);
                    combinedVertexBuffer.push_back(w0);
                }
            }
            else if (strcmp(semantic.c_str(), "Normal") == 0)
            {
                if (rawBuffers.normalAttributeBuffer.empty())
                {
                    combinedVertexBuffer.push_back(0.0f);  // we will calculate
                    combinedVertexBuffer.push_back(0.0f);  // we will calculate
                    combinedVertexBuffer.push_back(0.0f);  // we will calculate
                }
                else
                {
                    f32 nx0 = rawBuffers.normalAttributeBuffer[i0 * 3];
                    f32 ny0 = rawBuffers.normalAttributeBuffer[i0 * 3 + 1];
                    f32 nz0 = rawBuffers.normalAttributeBuffer[i0 * 3 + 2];

                    combinedVertexBuffer.push_back(nx0);
                    combinedVertexBuffer.push_back(ny0);
                    combinedVertexBuffer.push_back(nz0);
                }
            }
            else if (strcmp(semantic.c_str(), "Tangent") == 0)
            {
                if (rawBuffers.tangentAttributeBuffer.empty())
                {
                    combinedVertexBuffer.push_back(0.0f);  // we will calculate
                    combinedVertexBuffer.push_back(0.0f);  // we will calculate
                    combinedVertexBuffer.push_back(0.0f);  // we will calculate
                    combinedVertexBuffer.push_back(0.0f);  // we will calculate
                }
                else
                {
                    f32 tx0 = rawBuffers.tangentAttributeBuffer[i0 * 4];
                    f32 ty0 = rawBuffers.tangentAttributeBuffer[i0 * 4 + 1];
                    f32 tz0 = rawBuffers.tangentAttributeBuffer[i0 * 4 + 2];
                    f32 tw0 = rawBuffers.tangentAttributeBuffer[i0 * 4 + 3];

                    combinedVertexBuffer.push_back(tx0);
                    combinedVertexBuffer.push_back(ty0);
                    combinedVertexBuffer.push_back(tz0);
                    combinedVertexBuffer.push_back(-tw0);
                }
            }
            else if (strcmp(semantic.c_str(), "TexCoord") == 0)
            {
                if (modifiedLayout.indices[j] == 0)
                {
                    f32 u0 = rawBuffers.texture0AttributeBuffer[i0 * 2];
                    f32 v0 = rawBuffers.texture0AttributeBuffer[i0 * 2 + 1];
                    combinedVertexBuffer.push_back(u0);
                    combinedVertexBuffer.push_back(v0);
                }
                else
                {
                    f32 u1 = rawBuffers.texture1AttributeBuffer[i0 * 2];
                    f32 v1 = rawBuffers.texture1AttributeBuffer[i0 * 2 + 1];
                    combinedVertexBuffer.push_back(u1);
                    combinedVertexBuffer.push_back(v1);
                }
            }
            else if (strcmp(semantic.c_str(), "Color") == 0)
            {
                if (modifiedLayout.indices[j] == 0)
                {
                    if (modifiedLayout.formats[j] == DXGI_FORMAT_R32G32B32A32_FLOAT)
                    {
                        f32 r0 = rawBuffers.color0AttributeBuffer[i0 * 4];
                        f32 g0 = rawBuffers.color0AttributeBuffer[i0 * 4 + 1];
                        f32 b0 = rawBuffers.color0AttributeBuffer[i0 * 4 + 2];
                        f32 a0 = rawBuffers.color0AttributeBuffer[i0 * 4 + 3];
                        combinedVertexBuffer.push_back(r0);  // we will calculate
                        combinedVertexBuffer.push_back(g0);  // we will calculate
                        combinedVertexBuffer.push_back(b0);  // we will calculate
                        combinedVertexBuffer.push_back(a0);  // we will calculate
                    }
                    else
                    {
                        f32 r0 = rawBuffers.color0AttributeBuffer[i0 * 3];
                        f32 g0 = rawBuffers.color0AttributeBuffer[i0 * 3 + 1];
                        f32 b0 = rawBuffers.color0AttributeBuffer[i0 * 3 + 2];

                        combinedVertexBuffer.push_back(r0);  // we will calculate
                        combinedVertexBuffer.push_back(g0);  // we will calculate
                        combinedVertexBuffer.push_back(b0);  // we will calculate
                    }
                }
                else
                {
                    if (modifiedLayout.formats[j] == DXGI_FORMAT_R32G32B32A32_FLOAT)
                    {
                        f32 r0 = rawBuffers.color1AttributeBuffer[i0 * 4];
                        f32 g0 = rawBuffers.color1AttributeBuffer[i0 * 4 + 1];
                        f32 b0 = rawBuffers.color1AttributeBuffer[i0 * 4 + 2];
                        f32 a0 = rawBuffers.color1AttributeBuffer[i0 * 4 + 3];
                        combinedVertexBuffer.push_back(r0);  // we will calculate
                        combinedVertexBuffer.push_back(g0);  // we will calculate
                        combinedVertexBuffer.push_back(b0);  // we will calculate
                        combinedVertexBuffer.push_back(a0);  // we will calculate
                    }
                    else
                    {
                        f32 r0 = rawBuffers.color1AttributeBuffer[i0 * 3];
                        f32 g0 = rawBuffers.color1AttributeBuffer[i0 * 3 + 1];
                        f32 b0 = rawBuffers.color1AttributeBuffer[i0 * 3 + 2];

                        combinedVertexBuffer.push_back(r0);  // we will calculate
                        combinedVertexBuffer.push_back(g0);  // we will calculate
                        combinedVertexBuffer.push_back(b0);  // we will calculate
                    }
                }
            }
        }

        indexBuffer.push_back(indexBuffer.size());
    }
}

static void
finalize_vertex_buffers(RawAttributeBuffers& rawBuffers, mvVertexLayout& modifiedLayout, std::vector<f32>& combinedVertexBuffer, std::vector<u32>& indexBuffer, std::vector<f32>& vertexBuffer)
{
    // calculate normals, tangents
    for (size_t i = 0; i < indexBuffer.size() - 2; i += 3)
    {

        size_t indices[3];
        indices[0] = indexBuffer[i];
        indices[1] = indexBuffer[i + 1];
        indices[2] = indexBuffer[i + 2];

        mvVec3 p[3];
        mvVec3 n[3];
        mvVec4 tan[3];
        mvVec2 tex0[3];
        mvVec2 tex1[3];
        mvVec4 color0[3];
        mvVec4 color1[3];
        mvVec4 joints0[3];
        mvVec4 joints1[3];
        mvVec4 weights0[3];
        mvVec4 weights1[3];

        u32 currentLocation = 0u;
        for (size_t j = 0; j < modifiedLayout.semantics.size(); j++)
        {
            std::string semantic = modifiedLayout.semantics[j];

            if (strcmp(semantic.c_str(), "Position") == 0)
            {

                for (size_t k = 0; k < 3; k++)
                {
                    p[k].x = combinedVertexBuffer[indices[k] * modifiedLayout.elementCount + currentLocation];
                    p[k].y = combinedVertexBuffer[indices[k] * modifiedLayout.elementCount + currentLocation + 1];
                    p[k].z = combinedVertexBuffer[indices[k] * modifiedLayout.elementCount + currentLocation + 2];
                }
                currentLocation += 3u;
            }
            else if (strcmp(semantic.c_str(), "Normal") == 0)
            {

                for (size_t k = 0; k < 3; k++)
                {
                    n[k].x = combinedVertexBuffer[indices[k] * modifiedLayout.elementCount + currentLocation];
                    n[k].y = combinedVertexBuffer[indices[k] * modifiedLayout.elementCount + currentLocation + 1];
                    n[k].z = combinedVertexBuffer[indices[k] * modifiedLayout.elementCount + currentLocation + 2];
                }
                currentLocation += 3u;

            }
            else if (strcmp(semantic.c_str(), "Tangent") == 0)
            {

                for (size_t k = 0; k < 3; k++)
                {
                    tan[k].x = combinedVertexBuffer[indices[k] * modifiedLayout.elementCount + currentLocation];
                    tan[k].y = combinedVertexBuffer[indices[k] * modifiedLayout.elementCount + currentLocation + 1];
                    tan[k].z = combinedVertexBuffer[indices[k] * modifiedLayout.elementCount + currentLocation + 2];
                    tan[k].w = combinedVertexBuffer[indices[k] * modifiedLayout.elementCount + currentLocation + 3];
                }
                currentLocation += 4u;

            }
            else if (strcmp(semantic.c_str(), "Joints") == 0)
            {
                if (modifiedLayout.indices[j] == 0)
                {
                    for (size_t k = 0; k < 3; k++)
                    {
                        joints0[k].x = combinedVertexBuffer[indices[k] * modifiedLayout.elementCount + currentLocation];
                        joints0[k].y = combinedVertexBuffer[indices[k] * modifiedLayout.elementCount + currentLocation + 1];
                        joints0[k].z = combinedVertexBuffer[indices[k] * modifiedLayout.elementCount + currentLocation + 2];
                        joints0[k].w = combinedVertexBuffer[indices[k] * modifiedLayout.elementCount + currentLocation + 3];
                    }
                }
                else
                {
                    for (size_t k = 0; k < 3; k++)
                    {
                        joints1[k].x = combinedVertexBuffer[indices[k] * modifiedLayout.elementCount + currentLocation];
                        joints1[k].y = combinedVertexBuffer[indices[k] * modifiedLayout.elementCount + currentLocation + 1];
                        joints1[k].z = combinedVertexBuffer[indices[k] * modifiedLayout.elementCount + currentLocation + 2];
                        joints1[k].w = combinedVertexBuffer[indices[k] * modifiedLayout.elementCount + currentLocation + 3];
                    }
                }
                currentLocation += 4u;

            }
            else if (strcmp(semantic.c_str(), "Weights") == 0)
            {
                if (modifiedLayout.indices[j] == 0)
                {
                    for (size_t k = 0; k < 3; k++)
                    {
                        weights0[k].x = combinedVertexBuffer[indices[k] * modifiedLayout.elementCount + currentLocation];
                        weights0[k].y = combinedVertexBuffer[indices[k] * modifiedLayout.elementCount + currentLocation + 1];
                        weights0[k].z = combinedVertexBuffer[indices[k] * modifiedLayout.elementCount + currentLocation + 2];
                        weights0[k].w = combinedVertexBuffer[indices[k] * modifiedLayout.elementCount + currentLocation + 3];
                    }
                }
                else
                {
                    for (size_t k = 0; k < 3; k++)
                    {
                        weights1[k].x = combinedVertexBuffer[indices[k] * modifiedLayout.elementCount + currentLocation];
                        weights1[k].y = combinedVertexBuffer[indices[k] * modifiedLayout.elementCount + currentLocation + 1];
                        weights1[k].z = combinedVertexBuffer[indices[k] * modifiedLayout.elementCount + currentLocation + 2];
                        weights1[k].w = combinedVertexBuffer[indices[k] * modifiedLayout.elementCount + currentLocation + 3];
                    }
                }
                currentLocation += 4u;

            }
            else if (strcmp(semantic.c_str(), "TexCoord") == 0)
            {

                if (modifiedLayout.indices[j] == 0)
                {
                    for (size_t k = 0; k < 3; k++)
                    {
                        tex0[k].x = combinedVertexBuffer[indices[k] * modifiedLayout.elementCount + currentLocation];
                        tex0[k].y = combinedVertexBuffer[indices[k] * modifiedLayout.elementCount + currentLocation + 1];
                    }
                }
                else
                {
                    for (size_t k = 0; k < 3; k++)
                    {
                        tex1[k].x = combinedVertexBuffer[indices[k] * modifiedLayout.elementCount + currentLocation];
                        tex1[k].y = combinedVertexBuffer[indices[k] * modifiedLayout.elementCount + currentLocation + 1];
                    }
                }
                currentLocation += 2u;

            }

            else if (strcmp(semantic.c_str(), "Color") == 0)
            {
                if (modifiedLayout.indices[j] == 0)
                {
                    for (size_t k = 0; k < 3; k++)
                    {
                        color0[k].r = combinedVertexBuffer[indices[k] * modifiedLayout.elementCount + currentLocation];
                        color0[k].g = combinedVertexBuffer[indices[k] * modifiedLayout.elementCount + currentLocation + 1];
                        color0[k].b = combinedVertexBuffer[indices[k] * modifiedLayout.elementCount + currentLocation + 2];

                        if (modifiedLayout.formats[j] == DXGI_FORMAT_R32G32B32A32_FLOAT)
                        {
                            color0[k].a = combinedVertexBuffer[indices[k] * modifiedLayout.elementCount + currentLocation + 3];
                        }
                    }
                }
                else
                {
                    for (size_t k = 0; k < 3; k++)
                    {
                        color1[k].r = combinedVertexBuffer[indices[k] * modifiedLayout.elementCount + currentLocation];
                        color1[k].g = combinedVertexBuffer[indices[k] * modifiedLayout.elementCount + currentLocation + 1];
                        color1[k].b = combinedVertexBuffer[indices[k] * modifiedLayout.elementCount + currentLocation + 2];

                        if (modifiedLayout.formats[j] == DXGI_FORMAT_R32G32B32A32_FLOAT)
                        {
                            color1[k].a = combinedVertexBuffer[indices[k] * modifiedLayout.elementCount + currentLocation + 3];
                        }
                    }
                }
                if (modifiedLayout.formats[j] == DXGI_FORMAT_R32G32B32A32_FLOAT)
                    currentLocation += 4u;
                else
                    currentLocation += 3u;

            }
        }

        // calculate normals


         //calculate tangents
        mvVec3 edge1 = p[1] - p[0];
        mvVec3 edge2 = p[2] - p[0];

        mvVec2 uv1 = tex0[1] - tex0[0];
        mvVec2 uv2 = tex0[2] - tex0[0];

        if (uv1.x * uv2.y == uv1.y * uv2.x)
        {
            uv1.x = 0.0f;
            uv1.y = 1.0f;
            uv2.x = 1.0f;
            uv2.y = 0.0f;
        }

        float dirCorrection = (uv1.x * uv2.y - uv1.y * uv2.x) < 0.0f ? -1.0f : 1.0f;

        mvVec4 tangent = {
            ((edge1.x * uv2.y) - (edge2.x * uv1.y)) * dirCorrection,
            ((edge1.y * uv2.y) - (edge2.y * uv1.y)) * dirCorrection,
            ((edge1.z * uv2.y) - (edge2.z * uv1.y)) * dirCorrection,
            dirCorrection
        };

        if (rawBuffers.normalAttributeBuffer.empty())
        {
            //mvVec3 nn = {
            //    ((edge1.x * uv2.y) - (edge2.x * uv1.y)),
            //    ((edge1.y * uv2.y) - (edge2.y * uv1.y)),
            //    ((edge1.z * uv2.y) - (edge2.z * uv1.y))
            //};
            mvVec3 nn = normalize(cross(edge1, edge2));
            n[0] = nn;
            n[1] = nn;
            n[2] = nn;
        }

        // project tangent and bitangent into the plane formed by the vertex' normal
        //mvVec3 newTangent = cNormalize(tangent - n * (tangent * n));
        mvVec4 tanf[3];
        for (size_t k = 0; k < 3; k++)
        {
            if (rawBuffers.tangentAttributeBuffer.empty())
            {
                mvVec3 interTan = normalize(tangent.xyz() - n[k] * (tangent.xyz() * n[k]));
                tanf[k].x = interTan.x;
                tanf[k].y = interTan.y;
                tanf[k].z = interTan.z;
                tanf[k].w = dirCorrection;
            }
            else
            {
                tanf[k] = tan[k];
            }
        }

        for (size_t k = 0; k < 3; k++)
        {
            for (size_t j = 0; j < modifiedLayout.semantics.size(); j++)
            {
                std::string semantic = modifiedLayout.semantics[j];

                if (strcmp(semantic.c_str(), "Position") == 0)
                {
                    vertexBuffer.push_back(p[k].x);
                    vertexBuffer.push_back(p[k].y);
                    vertexBuffer.push_back(p[k].z);
                }
                else if (strcmp(semantic.c_str(), "Normal") == 0)
                {
                    vertexBuffer.push_back(n[k].x);
                    vertexBuffer.push_back(n[k].y);
                    vertexBuffer.push_back(n[k].z);
                }
                else if (strcmp(semantic.c_str(), "Tangent") == 0)
                {
                    vertexBuffer.push_back(tanf[k].x);
                    vertexBuffer.push_back(tanf[k].y);
                    vertexBuffer.push_back(tanf[k].z);
                    vertexBuffer.push_back(tanf[k].w);
                }
                else if (strcmp(semantic.c_str(), "Joints") == 0)
                {
                    if (modifiedLayout.indices[j] == 0)
                    {
                        vertexBuffer.push_back(joints0[k].x);
                        vertexBuffer.push_back(joints0[k].y);
                        vertexBuffer.push_back(joints0[k].z);
                        vertexBuffer.push_back(joints0[k].w);
                    }
                    else
                    {
                        vertexBuffer.push_back(joints1[k].x);
                        vertexBuffer.push_back(joints1[k].y);
                        vertexBuffer.push_back(joints1[k].z);
                        vertexBuffer.push_back(joints1[k].w);
                    }
                }
                else if (strcmp(semantic.c_str(), "Weights") == 0)
                {
                    if (modifiedLayout.indices[j] == 0)
                    {
                        vertexBuffer.push_back(weights0[k].x);
                        vertexBuffer.push_back(weights0[k].y);
                        vertexBuffer.push_back(weights0[k].z);
                        vertexBuffer.push_back(weights0[k].w);
                    }
                    else
                    {
                        vertexBuffer.push_back(weights1[k].x);
                        vertexBuffer.push_back(weights1[k].y);
                        vertexBuffer.push_back(weights1[k].z);
                        vertexBuffer.push_back(weights1[k].w);
                    }
                }
                else if (strcmp(semantic.c_str(), "TexCoord") == 0)
                {


                    if (modifiedLayout.indices[j] == 0)
                    {
                        vertexBuffer.push_back(tex0[k].x);
                        vertexBuffer.push_back(tex0[k].y);
                    }
                    else
                    {
                        vertexBuffer.push_back(tex1[k].x);
                        vertexBuffer.push_back(tex1[k].y);
                    }
                }

                else if (strcmp(semantic.c_str(), "Color") == 0)
                {

                    if (modifiedLayout.indices[j] == 0)
                    {
                        vertexBuffer.push_back(color0[k].x);
                        vertexBuffer.push_back(color0[k].y);
                        vertexBuffer.push_back(color0[k].z);

                        if (modifiedLayout.formats[j] == DXGI_FORMAT_R32G32B32A32_FLOAT)
                        {
                            vertexBuffer.push_back(color0[k].w);
                        }
                    }
                    else
                    {
                        vertexBuffer.push_back(color1[k].x);
                        vertexBuffer.push_back(color1[k].y);
                        vertexBuffer.push_back(color1[k].z);

                        if (modifiedLayout.formats[j] == DXGI_FORMAT_R32G32B32A32_FLOAT)
                        {
                            vertexBuffer.push_back(color1[k].w);
                        }
                    }
                }
            }
        }

    }

}

static std::vector<mvVertexElement>
gather_target_attributes(mvGLTFModel& model, mvGLTFMeshPrimitive& glprimitive)
{
    std::vector<mvVertexElement> targetAttributes;

    b8 hasPosition = false;
    b8 hasNormal = false;
    b8 hasTangent = false;
    b8 hasTex0 = false;
    b8 hasTex1 = false;
    b8 hasColor0 = false;
    b8 hasColor1 = false;

    for (int i = 0; i < glprimitive.target_count; i++)
    {
        mvGLTFMorphTarget target = glprimitive.targets[i];
        for (int j = 0; j < target.attribute_count; j++)
        {
            mvGLTFAttribute attribute = target.attributes[j];
            if (strcmp(attribute.semantic.c_str(), "POSITION") == 0 && !hasPosition)
            {
                hasPosition = true;
                targetAttributes.push_back(mvVertexElement::Position3D);
            }
            else if (strcmp(attribute.semantic.c_str(), "NORMAL") == 0 && !hasNormal)
            {
                hasNormal = true;
                targetAttributes.push_back(mvVertexElement::Normal);
            }
            else if (strcmp(attribute.semantic.c_str(), "TANGENT") == 0 && !hasTangent)
            {
                hasTangent = true;
                targetAttributes.push_back(mvVertexElement::Tangent);
            }
            else if (strcmp(attribute.semantic.c_str(), "TEXCOORD_0") == 0 && !hasTex0)
            {
                hasTex0 = true;
                targetAttributes.push_back(mvVertexElement::TexCoord0);
            }
            else if (strcmp(attribute.semantic.c_str(), "TEXCOORD_1") == 0 && !hasTex1)
            {
                hasTex1 = true;
                targetAttributes.push_back(mvVertexElement::TexCoord1);
            }
            else if (strcmp(attribute.semantic.c_str(), "COLOR_0") == 0 && !hasColor1)
            {
                hasColor0 = true;
                mvGLTFAccessor& accessor = model.accessors[attribute.index];
                if (accessor.type == MV_IMP_VEC3)
                {
                    targetAttributes.push_back(mvVertexElement::Color3_0);
                }
                else if (accessor.type == MV_IMP_VEC4)
                {
                    targetAttributes.push_back(mvVertexElement::Color4_0);
                }
                else
                {
                    assert(false && "Undefined attribute type");
                }

            }

            else if (strcmp(attribute.semantic.c_str(), "COLOR_1") == 0 && !hasColor1)
            {
                hasColor1 = true;
                mvGLTFAccessor& accessor = model.accessors[attribute.index];
                if (accessor.type == MV_IMP_VEC3)
                {
                    targetAttributes.push_back(mvVertexElement::Color3_1);
                }
                else if (accessor.type == MV_IMP_VEC4)
                {
                    targetAttributes.push_back(mvVertexElement::Color4_1);
                }
                else
                {
                    assert(false && "Undefined attribute type");
                }

            }
        }
    }
    return targetAttributes;
}

static std::vector<mvAssetID>
load_gltf_meshes(mvAssetManager& assetManager, mvGLTFModel& model)
{
    std::vector<mvAssetID> meshMapping;
    meshMapping.resize(model.mesh_count);

    for (u32 currentMesh = 0u; currentMesh < model.mesh_count; currentMesh++)
    {
        mvGLTFMesh& glmesh = model.meshes[currentMesh];

        mvMesh newMesh{};
        newMesh.name = glmesh.name;
        b8 useMorphing = false;
        newMesh.weightCount = glmesh.weights_count;
        for (u32 currentWeight = 0; currentWeight < glmesh.weights_count; currentWeight++)
        {
            newMesh.weights.push_back(glmesh.weights[currentWeight]);
            newMesh.weights.push_back(0.0f);
            newMesh.weights.push_back(0.0f);
            newMesh.weights.push_back(0.0f);
            newMesh.weightsAnimated.push_back(glmesh.weights[currentWeight]);
            newMesh.weightsAnimated.push_back(0.0f);
            newMesh.weightsAnimated.push_back(0.0f);
            newMesh.weightsAnimated.push_back(0.0f);
        }

        if (!newMesh.weights.empty())
        {
            useMorphing = true;
            //while (newMesh.weights.size() * sizeof(f32) % 16)
            //{
            //    newMesh.weights.push_back(0.0f);
            //    newMesh.weightsAnimated.push_back(0.0f);
            //}

            // create transform constant buffer
            D3D11_BUFFER_DESC cbd;
            cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            cbd.Usage = D3D11_USAGE_DYNAMIC;
            cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            cbd.MiscFlags = 0u;
            cbd.ByteWidth = sizeof(float)*newMesh.weights.size();
            cbd.StructureByteStride = 0u;

            newMesh.morphBuffer.size = cbd.ByteWidth;
            GContext->graphics.device->CreateBuffer(&cbd, nullptr, &newMesh.morphBuffer.buffer);
            update_const_buffer(newMesh.morphBuffer, newMesh.weights.data());
        }

        for (u32 currentPrimitive = 0u; currentPrimitive < glmesh.primitives_count; currentPrimitive++)
        {

            mvGLTFMeshPrimitive& glprimitive = glmesh.primitives[currentPrimitive];

            std::vector<u32> origIndexBuffer;
            if (glprimitive.indices_index > -1)
            {
                u8 indexCompCount = mvGetAccessorItemCompCount(model.accessors[glprimitive.indices_index]);

                mvGLTFComponentType indexCompType = model.accessors[glprimitive.indices_index].component_type;
                mvFillBuffer(model, model.accessors[glprimitive.indices_index], origIndexBuffer);
            }

            RawAttributeBuffers rawBuffers{};
            std::vector<mvVertexElement> attributes = load_raw_attribute_buffers(model, glprimitive, rawBuffers);

            std::vector<u32> indexBuffer;
            std::vector<f32> vertexBuffer;

            u32 triangleCount = origIndexBuffer.size() * 3;

            if (glprimitive.indices_index == -1)
                triangleCount = rawBuffers.positionAttributeBuffer.size();

            if (rawBuffers.normalAttributeBuffer.empty())
            {
                attributes.push_back(mvVertexElement::Normal);
            }
            if (rawBuffers.tangentAttributeBuffer.empty())
            {
                attributes.push_back(mvVertexElement::Tangent);
            }

            mvVertexLayout modifiedLayout = create_vertex_layout(attributes);

            vertexBuffer.reserve(triangleCount * modifiedLayout.size * 3);
            indexBuffer.reserve(triangleCount * 3);

            std::vector<f32> combinedVertexBuffer;

            combine_vertex_buffer(triangleCount, modifiedLayout, glprimitive, origIndexBuffer, rawBuffers, indexBuffer, combinedVertexBuffer);
            finalize_vertex_buffers(rawBuffers, modifiedLayout, combinedVertexBuffer, indexBuffer, vertexBuffer);
            mvMaterial materialData{};
            newMesh.primitives.push_back({});

            std::vector<mvVertexElement> targetAttributes = gather_target_attributes(model, glprimitive);

            if (!targetAttributes.empty())
            {
                
                materialData.extramacros.push_back({ "HAS_MORPH_TARGETS", "1" });
                const auto max2DTextureSize = pow(D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION, 2);
                const auto maxTextureArraySize = D3D11_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION;

                std::unordered_map<mvVertexElement, i32> attributeOffsets;
                i32 attributeOffset = 0;

                for (i32 i = 0; i < targetAttributes.size(); i++)
                {
                    switch (targetAttributes[i])
                    {
                    case mvVertexElement::Position3D:
                        materialData.extramacros.push_back({ "HAS_MORPH_TARGET_POSITION", "1" });
                        materialData.extramacros.push_back({ "MORPH_TARGET_POSITION_OFFSET", std::to_string(attributeOffset) });
                        break;
                    case mvVertexElement::Normal:
                        materialData.extramacros.push_back({ "HAS_MORPH_TARGET_NORMAL", "1" });
                        materialData.extramacros.push_back({ "MORPH_TARGET_NORMAL_OFFSET", std::to_string(attributeOffset) });
                        break;
                    case mvVertexElement::Tangent:
                        materialData.extramacros.push_back({ "HAS_MORPH_TARGET_TANGENT", "1" });
                        materialData.extramacros.push_back({ "MORPH_TARGET_TANGENT_OFFSET", std::to_string(attributeOffset) });
                        break;
                    case mvVertexElement::TexCoord0:
                        materialData.extramacros.push_back({ "HAS_MORPH_TARGET_TEXCOORD_0", "1" });
                        materialData.extramacros.push_back({ "MORPH_TARGET_TEXCOORD_0_OFFSET", std::to_string(attributeOffset) });
                        break;
                    case mvVertexElement::TexCoord1:
                        materialData.extramacros.push_back({ "HAS_MORPH_TARGET_TEXCOORD_1", "1" });
                        materialData.extramacros.push_back({ "MORPH_TARGET_TEXCOORD_1_OFFSET", std::to_string(attributeOffset) });
                        break;
                    }
                    
                    attributeOffsets[targetAttributes[i]] = attributeOffset;
                    attributeOffset += glprimitive.target_count;
                }

                //int vertexCount = model.accessors[glprimitive.targets[0].attributes[0].index].count;
                int vertexCount = triangleCount * 3;
                f32 textureWidth = ceil(sqrt(vertexCount));
                f32 singleTextureSize = pow(textureWidth, 2) * 4;
                newMesh.primitives.back().morphData = new f32[singleTextureSize * glprimitive.target_count * targetAttributes.size()];
                ZeroMemory(newMesh.primitives.back().morphData, sizeof(f32)*singleTextureSize* glprimitive.target_count* targetAttributes.size());

                for (i32 i = 0; i < glprimitive.target_count; i++)
                {
                    mvGLTFMorphTarget& target = glprimitive.targets[i];

                    for (const auto& item : attributeOffsets)
                    {
                        for (i32 j = 0; j < target.attribute_count; j++)
                        {
                            // todo: handle colors
                            mvVertexElement semantic = get_element_from_gltf_semantic(target.attributes[j].semantic.c_str());

                            if (item.first == semantic)
                            {
                                std::vector<f32> data;
                                
                                mvGLTFAccessor& accessor = model.accessors[target.attributes[j].index];
                                i32 offset = item.second * singleTextureSize;
                                mvFillBuffer(model, accessor, data);

                                std::vector<f32> rdata;
                                if (accessor.type == MV_IMP_VEC2)
                                {
                                    rdata.resize(origIndexBuffer.size() * 2);
                                    for (i32 k = 0; k < origIndexBuffer.size(); k++)
                                    {
                                        u32 i0 = origIndexBuffer[k];
                                        rdata[k * 3] = data[i0 * 3];
                                        rdata[k * 3 + 1] = data[i0 * 3 + 1];
                                    }
                                }
                                else if (accessor.type == MV_IMP_VEC3)
                                {
                                    rdata.resize(origIndexBuffer.size() * 3);
                                    for (i32 k = 0; k < origIndexBuffer.size(); k++)
                                    {
                                        u32 i0 = origIndexBuffer[k];
                                        rdata[k * 3] = data[i0 * 3];
                                        rdata[k * 3 + 1] = data[i0 * 3 + 1];
                                        rdata[k * 3 + 2] = data[i0 * 3 + 2];
                                    }
                                }
                                else if (accessor.type == MV_IMP_VEC4)
                                {
                                    rdata.resize(origIndexBuffer.size() * 4);
                                    for (i32 k = 0; k < origIndexBuffer.size(); k++)
                                    {
                                        u32 i0 = origIndexBuffer[k];
                                        rdata[k * 3] = data[i0 * 3];
                                        rdata[k * 3 + 1] = data[i0 * 3 + 1];
                                        rdata[k * 3 + 2] = data[i0 * 3 + 2];
                                        rdata[k * 3 + 3] = data[i0 * 3 + 3];
                                    }
                                }


                                switch (accessor.type)
                                {
                                case MV_IMP_VEC2:
                                case MV_IMP_VEC3:
                                {
                                    i32 paddingOffset = 0;
                                    i32 accessorOffset = 0;
                                    i32 componentCount = accessor.type == MV_IMP_VEC2 ? 2 : 3;
                                    for (i32 k = 0; k < origIndexBuffer.size(); k++)
                                    {
                                        memcpy(&newMesh.primitives.back().morphData[offset + paddingOffset], &rdata[accessorOffset], sizeof(f32)*componentCount);
                                        paddingOffset += 4;
                                        accessorOffset += componentCount;
                                    }
                                    break;
                                }
                                case MV_IMP_VEC4:
                                {
                                    memcpy(&newMesh.primitives.back().morphData[offset], rdata.data(), rdata.size() * sizeof(f32));
                                    break;
                                }

                                default:
                                    assert(false);
                                    
                                }

                            }
                        }
                        attributeOffsets[item.first] = item.second + 1;
                    }
                }
                newMesh.primitives.back().morphTexture = create_texture(textureWidth, textureWidth, glprimitive.target_count* targetAttributes.size(), newMesh.primitives.back().morphData);
            }

            newMesh.primitives.back().layout = modifiedLayout;
            materialData.extramacros.push_back({ "HAS_NORMALS", "0" });
            materialData.extramacros.push_back({ "HAS_TANGENTS", "0" });
            std::string weightCount = std::to_string(glmesh.weights_count);
            if (!rawBuffers.texture0AttributeBuffer.empty()) materialData.extramacros.push_back({ "HAS_TEXCOORD_0_VEC2", "0" });
            if (useMorphing) materialData.extramacros.push_back({ "USE_MORPHING", "0" });
            if (useMorphing) materialData.extramacros.push_back({ "WEIGHT_COUNT", weightCount.c_str() });
            if (!rawBuffers.texture1AttributeBuffer.empty()) materialData.extramacros.push_back({ "HAS_TEXCOORD_1_VEC2", "0" });
            if (!rawBuffers.color0AttributeBuffer.empty() && rawBuffers.hasColor0Vec3) materialData.extramacros.push_back({ "HAS_VERTEX_COLOR_0_VEC3", "0" });
            if (!rawBuffers.color0AttributeBuffer.empty() && rawBuffers.hasColor0Vec4) materialData.extramacros.push_back({ "HAS_VERTEX_COLOR_0_VEC4", "0" });
            if (!rawBuffers.color1AttributeBuffer.empty() && rawBuffers.hasColor1Vec3) materialData.extramacros.push_back({ "HAS_VERTEX_COLOR_1_VEC3", "0" });
            if (!rawBuffers.color1AttributeBuffer.empty() && rawBuffers.hasColor1Vec4) materialData.extramacros.push_back({ "HAS_VERTEX_COLOR_1_VEC4", "0" });
            if (!rawBuffers.joints0AttributeBuffer.empty()) materialData.extramacros.push_back({ "HAS_JOINTS_0_VEC4", "0" });
            if (!rawBuffers.joints1AttributeBuffer.empty()) materialData.extramacros.push_back({ "HAS_JOINTS_1_VEC4", "0" });
            if (!rawBuffers.weights0AttributeBuffer.empty()) materialData.extramacros.push_back({ "HAS_WEIGHTS_0_VEC4", "0" });
            if (!rawBuffers.weights1AttributeBuffer.empty()) materialData.extramacros.push_back({ "HAS_WEIGHTS_1_VEC4", "0" });
            if (!rawBuffers.joints0AttributeBuffer.empty()) materialData.extramacros.push_back({ "USE_SKINNING", "0" });

            // upload index buffer

            if (glprimitive.material_index != -1)
            {

                mvGLTFMaterial& material = model.materials[glprimitive.material_index];

                materialData.data.albedo = *(mvVec4*)material.base_color_factor;
                materialData.data.metalness = material.metallic_factor;
                materialData.data.roughness = material.roughness_factor;
                materialData.data.emisiveFactor = *(mvVec3*)material.emissive_factor;
                materialData.data.occlusionStrength = material.occlusion_texture_strength;
                materialData.data.alphaCutoff = 0.5f;
                materialData.data.doubleSided = material.double_sided;
                materialData.data.clearcoatFactor = material.clearcoat_factor;
                materialData.data.clearcoatRoughnessFactor = material.clearcoat_roughness_factor;
                materialData.data.clearcoatNormalScale = material.clearcoat_normal_texture_scale;
                materialData.extensionClearcoat = material.clearcoat_extension;
                materialData.pbrMetallicRoughness = material.pbrMetallicRoughness;
                materialData.alphaMode = material.alphaMode;
                if (materialData.alphaMode == 1)
                    materialData.data.alphaCutoff = material.alphaCutoff;

                newMesh.primitives.back().albedoTexture = setup_texture(assetManager, model, currentPrimitive, material.base_color_texture, materialData.hasAlbedoMap, newMesh.name, "_a");
                newMesh.primitives.back().normalTexture = setup_texture(assetManager, model, currentPrimitive, material.normal_texture, materialData.hasNormalMap, newMesh.name, "_n");
                newMesh.primitives.back().metalRoughnessTexture = setup_texture(assetManager, model, currentPrimitive, material.metallic_roughness_texture, materialData.hasMetallicRoughnessMap, newMesh.name, "_m");
                newMesh.primitives.back().emissiveTexture = setup_texture(assetManager, model, currentPrimitive, material.emissive_texture, materialData.hasEmmissiveMap, newMesh.name, "_e");
                newMesh.primitives.back().occlusionTexture = setup_texture(assetManager, model, currentPrimitive, material.occlusion_texture, materialData.hasOcculusionMap, newMesh.name, "_o");
                newMesh.primitives.back().clearcoatTexture = setup_texture(assetManager, model, currentPrimitive, material.clearcoat_texture, materialData.hasClearcoatMap, newMesh.name, "_cc");
                newMesh.primitives.back().clearcoatRoughnessTexture = setup_texture(assetManager, model, currentPrimitive, material.clearcoat_roughness_texture, materialData.hasClearcoatRoughnessMap, newMesh.name, "_ccr");
                newMesh.primitives.back().clearcoatNormalTexture = setup_texture(assetManager, model, currentPrimitive, material.clearcoat_normal_texture, materialData.hasClearcoatNormalMap, newMesh.name, "_ccn");
            }
            else
            {
                materialData.data.albedo = { 0.45f, 0.45f, 0.85f, 1.0f };
                materialData.data.metalness = 0.0f;
                materialData.data.roughness = 0.5f;
                materialData.data.alphaCutoff = 0.5f;
                materialData.data.doubleSided = false;
            }

            materialData.layout = modifiedLayout;
            std::string hash = hash_material(materialData, modifiedLayout, std::string("PBR_PS.hlsl"), std::string("PBR_VS.hlsl"));

            newMesh.primitives.back().materialID = mvGetMaterialAssetID(&assetManager, hash);
            if (newMesh.primitives.back().materialID == -1)
            {
                newMesh.primitives.back().materialID = register_asset(&assetManager, hash, create_material(assetManager, "PBR_VS.hlsl", "PBR_PS.hlsl", materialData));
            }

            newMesh.primitives.back().indexBuffer = mvGetBufferAssetID(&assetManager,
                model.name + std::string(glmesh.name) + std::to_string(currentMesh) + std::to_string(currentPrimitive) + "_indexbuffer",
                indexBuffer.data(),
                indexBuffer.size() * sizeof(u32),
                D3D11_BIND_INDEX_BUFFER);
            newMesh.primitives.back().vertexBuffer = mvGetBufferAssetID(&assetManager,
                model.name + std::string(glmesh.name) + std::to_string(currentMesh) + std::to_string(currentPrimitive) + "_vertexBuffer",
                vertexBuffer.data(),
                vertexBuffer.size() * sizeof(f32),
                D3D11_BIND_VERTEX_BUFFER);
        }

        meshMapping[currentMesh] = register_asset(&assetManager, newMesh.name + std::to_string(currentMesh), newMesh);

    }


    return meshMapping;
}

static std::vector<mvAssetID>
load_gltf_nodes(mvAssetManager& assetManager, mvGLTFModel& model)
{
    std::vector<mvAssetID> nodeMapping;
    nodeMapping.resize(model.node_count);

    for (u32 currentNode = 0u; currentNode < model.node_count; currentNode++)
    {
        mvGLTFNode& glnode = model.nodes[currentNode];

        mvNode newNode{};
        newNode.name = glnode.name;
        if (glnode.mesh_index > -1)
            newNode.mesh = glnode.mesh_index;
        if (glnode.skin_index > -1)
            newNode.skin = glnode.skin_index;
        if (glnode.camera_index > -1)
            newNode.camera = glnode.camera_index;

        newNode.childCount = glnode.child_count;

        for (i32 i = 0; i < glnode.child_count; i++)
            newNode.children[i] = (mvAssetID)glnode.children[i];

        newNode.rotation = *(mvVec4*)(glnode.rotation);
        newNode.scale = *(mvVec3*)(glnode.scale);
        newNode.translation = *(mvVec3*)(glnode.translation);

        if (glnode.hadMatrix)
        {
            newNode.matrix = *(mvMat4*)(glnode.matrix);
        }
        else
        {
            newNode.matrix = rotation_translation_scale(newNode.rotation, newNode.translation, newNode.scale);
        }

        newNode.transform = newNode.matrix;

        nodeMapping[currentNode] = register_asset(&assetManager, "node_" + std::to_string(currentNode), newNode);
    }

    return nodeMapping;
}

static std::vector<mvAssetID>
load_gltf_animations(mvAssetManager& assetManager, mvGLTFModel& model)
{
    std::vector<mvAssetID> animationMapping;
    animationMapping.resize(model.animation_count);

    for (u32 currentAnimation = 0u; currentAnimation < model.animation_count; currentAnimation++)
    {
        mvGLTFAnimation& glanimation = model.animations[currentAnimation];

        mvAnimation animation{};
        animation.channels = new mvAnimationChannel[glanimation.channel_count];
        animation.channelCount = glanimation.channel_count;

        for (u32 channel_index = 0u; channel_index < glanimation.channel_count; channel_index++)
        {
            mvGLTFAnimationChannel& glchannel = glanimation.channels[channel_index];
            mvGLTFAnimationSampler& glsampler = glanimation.samplers[glchannel.sampler];
            mvAnimationChannel& channel = animation.channels[channel_index];
            channel.node = glchannel.target.node;
            channel.path = glchannel.target.path;
            channel.interpolation = glsampler.interpolation;

            mvGLTFAccessor& inputaccessor = model.accessors[glsampler.input];
            mvGLTFAccessor& outputaccessor = model.accessors[glsampler.output];
            mvFillBuffer(model, inputaccessor, channel.inputdata, 1);

            mvGLTFComponentType indexCompType = outputaccessor.component_type;

            if (channel.path == "translation" || channel.path == "scale")
            {
                mvFillBuffer(model, outputaccessor, channel.outputdata, 3);
            }

            if (channel.path == "rotation")
            {
                mvFillBuffer(model, outputaccessor, channel.outputdata, 4);
            }

            if (channel.path == "weights")
            {
                mvFillBuffer(model, outputaccessor, channel.outputdata, 1);
            }
        }

        animationMapping[currentAnimation] = register_asset(&assetManager, model.name + "_animation_" + std::to_string(currentAnimation), animation);
    }

    return animationMapping;
}

mvAssetID
load_gltf_assets(mvAssetManager& assetManager, mvGLTFModel& model)
{

    std::vector<mvAssetID> skinMapping = load_gltf_skins(assetManager, model);
    std::vector<mvAssetID> cameraMapping = load_gltf_cameras(assetManager, model);
    std::vector<mvAssetID> meshMapping = load_gltf_meshes(assetManager, model);
    std::vector<mvAssetID> nodeMapping = load_gltf_nodes(assetManager, model);
    std::vector<mvAssetID> animationMapping = load_gltf_animations(assetManager, model);

    // updates based on correct offset mapping
    for (u32 currentAnimation = 0u; currentAnimation < model.animation_count; currentAnimation++)
    {
        mvAnimation& animation = assetManager.animations[animationMapping[currentAnimation]].asset;

        for (u32 channel_index = 0u; channel_index < animation.channelCount; channel_index++)
        {
            mvAnimationChannel& channel = animation.channels[channel_index];
            channel.node = nodeMapping[channel.node];
            assetManager.nodes[channel.node].asset.animated = true;
        }
    }

    for (u32 currentNode = 0u; currentNode < model.node_count; currentNode++)
    {

        mvNode& node = assetManager.nodes[nodeMapping[currentNode]].asset;
        if(node.mesh > -1)
            node.mesh = meshMapping[node.mesh];
        if (node.skin > -1)
            node.skin = skinMapping[node.skin];
        for (i32 i = 0; i < node.childCount; i++)
            node.children[i] = nodeMapping[node.children[i]];

        if (node.camera > -1)
        {

            node.camera = cameraMapping[node.camera];
            mvCamera& camera = assetManager.cameras[node.camera].asset;

            if (camera.type == MV_CAMERA_PERSPECTIVE)
            {
                mvMesh frustum1 = create_frustum2(assetManager, camera.fieldOfView * 180.0f / M_PI, camera.aspectRatio, camera.nearZ, camera.farZ);
                node.mesh = register_asset(&assetManager, "node_camera_" + std::to_string(currentNode), frustum1);
            }
            else
            {
                mvMesh frustum1 = create_ortho_frustum(assetManager, camera.width, camera.height, camera.nearZ, camera.farZ);
                node.mesh = register_asset(&assetManager, "bad_node_camera_" + std::to_string(currentNode), frustum1);
            }
        }
    }

    // update joints for actual offset
    for (u32 currentSkin = 0u; currentSkin < model.skin_count; currentSkin++)
    {

        mvSkin& skin = assetManager.skins[skinMapping[currentSkin]].asset;
        if (skin.skeleton != -1)
            skin.skeleton = nodeMapping[skin.skeleton];
        for (i32 i = 0; i < skin.jointCount; i++)
        {
            skin.joints[i] = nodeMapping[skin.joints[i]];
        }
    }

    mvAssetID defaultScene = -1;

    for (u32 currentScene = 0u; currentScene < model.scene_count; currentScene++)
    {
        mvGLTFScene& glscene = model.scenes[currentScene];

        mvScene newScene{};
        newScene.nodeCount = glscene.node_count;

        for (i32 i = 0; i < glscene.node_count; i++)
            newScene.nodes[i] = nodeMapping[glscene.nodes[i]];


        mvAssetID sceneId = register_asset(&assetManager, model.name + "_scene_" + std::to_string(currentScene), newScene);

        if (currentScene == model.scene)
            defaultScene = sceneId;
    }

    return defaultScene;
}