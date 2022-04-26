#include "mvAssetLoader.h"
#include <unordered_map>
#include <assert.h>
#include "sGltf.h"
#include "mvGraphics.h"
#include "mvAnimation.h"
#include "mvCamera.h"

static unsigned char
mvGetAccessorItemCompCount(sGLTFAccessor& accessor)
{
    switch (accessor.type)
    {
    case(S_GLTF_SCALAR): return 1u;
    case(S_GLTF_VEC2): return 2u;
    case(S_GLTF_VEC3): return 3u;
    case(S_GLTF_VEC4): return 4u;
    case(S_GLTF_MAT2): return 4u;
    case(S_GLTF_MAT3): return 9u;
    case(S_GLTF_MAT4): return 16u;
    default:
        assert(false && "Undefined attribute type");
        return 0u;
    }
}

static int
mvGetBufferViewStride(sGLTFModel& model, sGLTFAccessor& accessor)
{
    sGLTFBufferView& bufferview = model.bufferviews[accessor.buffer_view_index];

    unsigned char actualItemCompCount = mvGetAccessorItemCompCount(accessor);

    // calculate stride if not available
    if (bufferview.byte_stride == -1)
    {
        switch (accessor.component_type)
        {
        case S_GLTF_UNSIGNED_BYTE:
        case S_GLTF_BYTE: return 1 * actualItemCompCount;

        case S_GLTF_UNSIGNED_SHORT:
        case S_GLTF_SHORT: return 2 * actualItemCompCount;

        case S_GLTF_FLOAT:
        case S_GLTF_INT:
        case S_GLTF_UNSIGNED_INT: return 4 * actualItemCompCount;

        case S_GLTF_DOUBLE: return 8 * actualItemCompCount;
        }
    }
    return bufferview.byte_stride;
}

template<typename T, typename W>
static void
mvFillBufferAsType(sGLTFModel& model, sGLTFAccessor& accessor, std::vector<W>& outBuffer, unsigned int componentCap)
{
    int bufferviewStride = mvGetBufferViewStride(model, accessor);
    sGLTFBufferView bufferView = model.bufferviews[accessor.buffer_view_index];
    char* bufferRawData = (char*)model.buffers[bufferView.buffer_index].data;
    char* bufferRawSection = &bufferRawData[bufferView.byte_offset + accessor.byteOffset]; // start of buffer section

    unsigned char actualItemCompCount = mvGetAccessorItemCompCount(accessor);

    unsigned char accessorItemCompCount = actualItemCompCount;
    if (componentCap < accessorItemCompCount)
        accessorItemCompCount = componentCap;

    for (int i = 0; i < accessor.count; i++)
    {
        T* values = (T*)&bufferRawSection[i * bufferviewStride];

        for (int j = 0; j < accessorItemCompCount; j++)
            outBuffer.push_back((W)values[j]);
    }
}

template<typename W>
static void
mvFillBuffer(sGLTFModel& model, sGLTFAccessor& accessor, std::vector<W>& outBuffer, unsigned int componentCap = 4)
{
    sGLTFComponentType indexCompType = accessor.component_type;

    switch (indexCompType)
    {
    case S_GLTF_BYTE:
    case S_GLTF_UNSIGNED_BYTE:
        mvFillBufferAsType<unsigned char>(model, accessor, outBuffer, componentCap);
        break;

    case S_GLTF_SHORT:
        mvFillBufferAsType<signed short>(model, accessor, outBuffer, componentCap);
        break;

    case S_GLTF_UNSIGNED_SHORT:
        mvFillBufferAsType<unsigned short>(model, accessor, outBuffer, componentCap);
        break;

    case S_GLTF_INT:
        mvFillBufferAsType<int>(model, accessor, outBuffer, componentCap);
        break;

    case S_GLTF_UNSIGNED_INT:
        mvFillBufferAsType<unsigned int>(model, accessor, outBuffer, componentCap);
        break;

    case S_GLTF_FLOAT:
        mvFillBufferAsType<float>(model, accessor, outBuffer, componentCap);
        break;

    case S_GLTF_DOUBLE:
        mvFillBufferAsType<double>(model, accessor, outBuffer, componentCap);
        break;

    default:
        assert(false && "Unknown index compenent type");
    }
}

static D3D11_TEXTURE_ADDRESS_MODE
get_address_mode(int address)
{
    switch (address)
    {
    case S_GLTF_WRAP_CLAMP_TO_EDGE:   return D3D11_TEXTURE_ADDRESS_CLAMP;
    case S_GLTF_WRAP_MIRRORED_REPEAT: return D3D11_TEXTURE_ADDRESS_MIRROR;
    case S_GLTF_WRAP_REPEAT:          return D3D11_TEXTURE_ADDRESS_WRAP;
    default:                          return D3D11_TEXTURE_ADDRESS_WRAP;
    }
}

static D3D11_FILTER
get_filter_mode(int minFilter, int magFilter)
{
    //return D3D11_FILTER_MIN_MAG_MIP_POINT;
    if (magFilter == S_GLTF_FILTER_LINEAR)
    {
        switch (minFilter)
        {

        case S_GLTF_FILTER_LINEAR_MIPMAP_NEAREST:  return D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
        case S_GLTF_FILTER_NEAREST_MIPMAP_LINEAR:  return D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR;

        case S_GLTF_FILTER_LINEAR:
        case S_GLTF_FILTER_LINEAR_MIPMAP_LINEAR:   return D3D11_FILTER_MIN_MAG_MIP_LINEAR;

        case S_GLTF_FILTER_NEAREST:
        case S_GLTF_FILTER_NEAREST_MIPMAP_NEAREST:
        default:                                   return D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
        }
    }
    else // MV_IMP_FILTER_NEAREST
    {
        switch (minFilter)
        {
        case S_GLTF_FILTER_LINEAR_MIPMAP_NEAREST:  return D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
        case S_GLTF_FILTER_NEAREST_MIPMAP_LINEAR:  return D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
        case S_GLTF_FILTER_LINEAR:
        case S_GLTF_FILTER_LINEAR_MIPMAP_LINEAR:   return D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
        case S_GLTF_FILTER_NEAREST_MIPMAP_NEAREST:
        case S_GLTF_FILTER_NEAREST:
        default:                                   return D3D11_FILTER_MIN_MAG_MIP_POINT;
        }
    }
}

static mvTexture
setup_texture(mvGraphics& graphics, sGLTFModel& model, unsigned int currentPrimitive, int textureID, bool& flag, std::string& name, std::string suffix)
{
    if (textureID == -1)
        return {};
    sGLTFTexture& texture = model.textures[textureID];
    std::string uri = model.images[texture.image_index].uri;
    mvTexture result{};
    if (model.images[texture.image_index].embedded)
    {
        result = create_texture(graphics, model.images[texture.image_index].data, model.images[texture.image_index].dataCount);
    }
    else
        result = create_texture(graphics, model.root + uri);
    flag = true;
    if (texture.sampler_index > -1)
    {
        sGLTFSampler& sampler = model.samplers[texture.sampler_index];

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

        HRESULT hResult = graphics.device->CreateSamplerState(&samplerDesc, result.sampler.GetAddressOf());
        assert(SUCCEEDED(hResult));
    }

    return result;
}

static void
load_gltf_skins(mvGraphics& graphics, mvModel& mvmodel, sGLTFModel& model)
{

    for (unsigned int currentSkin = 0u; currentSkin < model.skin_count; currentSkin++)
    {
        sGLTFSkin& glskin = model.skins[currentSkin];
        mvSkin skin{};

        skin.skeleton = glskin.skeleton;
        skin.jointCount = glskin.joints_count;
        for (unsigned int joint = 0; joint < skin.jointCount; joint++)
        {
            skin.joints[joint] = glskin.joints[joint];
        }

        mvFillBuffer(model, model.accessors[glskin.inverseBindMatrices], skin.inverseBindMatrices, 16);

        unsigned int textureWidth = ceil(sqrt(skin.jointCount * 8));

        skin.jointTexture = create_dynamic_texture(graphics, textureWidth, textureWidth);
        skin.textureData.resize(textureWidth * textureWidth * 4);

        mvmodel.skins.push_back(skin);
    }

}

struct RawAttributeBuffers
{
    std::vector<float> positionAttributeBuffer;
    std::vector<float> tangentAttributeBuffer;
    std::vector<float> normalAttributeBuffer;
    std::vector<float> texture0AttributeBuffer;
    std::vector<float> texture1AttributeBuffer;
    std::vector<float> color0AttributeBuffer;
    std::vector<float> color1AttributeBuffer;
    std::vector<float> joints0AttributeBuffer;
    std::vector<float> joints1AttributeBuffer;
    std::vector<float> weights0AttributeBuffer;
    std::vector<float> weights1AttributeBuffer;

    bool hasColor0Vec3 = false;
    bool hasColor0Vec4 = false;
    bool hasColor1Vec3 = false;
    bool hasColor1Vec4 = false;
};

static std::vector<mvVertexElement>
load_raw_attribute_buffers(sGLTFModel& model, sGLTFMeshPrimitive& glprimitive, RawAttributeBuffers& rawBuffers, float* minBoundary, float* maxBoundary)
{
    std::vector<mvVertexElement> attributes;
    for (unsigned int i = 0; i < glprimitive.attribute_count; i++)
    {
        auto& attribute = glprimitive.attributes[i];
        if (strcmp(attribute.semantic, "POSITION") == 0)
        {
            attributes.push_back(Position3D);
            mvFillBuffer(model, model.accessors[attribute.index], rawBuffers.positionAttributeBuffer);
            if (model.accessors[attribute.index].mins[0] < minBoundary[0]) minBoundary[0] = model.accessors[attribute.index].mins[0];
            if (model.accessors[attribute.index].mins[1] < minBoundary[1]) minBoundary[1] = model.accessors[attribute.index].mins[1];
            if (model.accessors[attribute.index].mins[2] < minBoundary[2]) minBoundary[2] = model.accessors[attribute.index].mins[2];
            if (model.accessors[attribute.index].maxes[0] > maxBoundary[0]) maxBoundary[0] = model.accessors[attribute.index].maxes[0];
            if (model.accessors[attribute.index].maxes[1] > maxBoundary[1]) maxBoundary[1] = model.accessors[attribute.index].maxes[1];
            if (model.accessors[attribute.index].maxes[2] > maxBoundary[2]) maxBoundary[2] = model.accessors[attribute.index].maxes[2];
        }
        else if (strcmp(attribute.semantic, "NORMAL") == 0)
        {
            attributes.push_back(Normal);
            mvFillBuffer(model, model.accessors[attribute.index], rawBuffers.normalAttributeBuffer, 3);
        }
        else if (strcmp(attribute.semantic, "TANGENT") == 0)
        {
            attributes.push_back(Tangent);
            mvFillBuffer(model, model.accessors[attribute.index], rawBuffers.tangentAttributeBuffer, 4);
        }
        else if (strcmp(attribute.semantic, "JOINTS_0") == 0)
        {
            attributes.push_back(Joints0);
            mvFillBuffer(model, model.accessors[attribute.index], rawBuffers.joints0AttributeBuffer, 4);
        }
        else if (strcmp(attribute.semantic, "JOINTS_1") == 0)
        {
            attributes.push_back(Joints1);
            mvFillBuffer(model, model.accessors[attribute.index], rawBuffers.joints1AttributeBuffer, 4);
        }
        else if (strcmp(attribute.semantic, "WEIGHTS_0") == 0)
        {
            attributes.push_back(Weights0);
            mvFillBuffer(model, model.accessors[attribute.index], rawBuffers.weights0AttributeBuffer, 4);
        }
        else if (strcmp(attribute.semantic, "WEIGHTS_1") == 0)
        {
            attributes.push_back(Weights1);
            mvFillBuffer(model, model.accessors[attribute.index], rawBuffers.weights1AttributeBuffer, 4);
        }
        else if (strcmp(attribute.semantic, "TEXCOORD_0") == 0)
        {
            attributes.push_back(TexCoord0);
            mvFillBuffer<float>(model, model.accessors[attribute.index], rawBuffers.texture0AttributeBuffer, 2);
        }
        else if (strcmp(attribute.semantic, "TEXCOORD_1") == 0)
        {
            attributes.push_back(TexCoord1);
            mvFillBuffer<float>(model, model.accessors[attribute.index], rawBuffers.texture1AttributeBuffer, 2);
        }
        else if (strcmp(attribute.semantic, "COLOR_0") == 0)
        {
            sGLTFAccessor& accessor = model.accessors[attribute.index];
            if (accessor.type == S_GLTF_VEC3)
            {
                rawBuffers.hasColor0Vec3 = true;
                rawBuffers.hasColor0Vec4 = false;
                attributes.push_back(Color3_0);
                mvFillBuffer(model, model.accessors[attribute.index], rawBuffers.color0AttributeBuffer, 3);
            }
            else if (accessor.type == S_GLTF_VEC4)
            {
                rawBuffers.hasColor0Vec3 = false;
                rawBuffers.hasColor0Vec4 = true;
                attributes.push_back(Color4_0);
                mvFillBuffer(model, model.accessors[attribute.index], rawBuffers.color0AttributeBuffer, 4);
            }
            else
            {
                assert(false && "Undefined attribute type");
            }

        }
        else if (strcmp(attribute.semantic, "COLOR_1") == 0)
        {
            sGLTFAccessor& accessor = model.accessors[attribute.index];
            if (accessor.type == S_GLTF_VEC3)
            {
                rawBuffers.hasColor1Vec3 = true;
                rawBuffers.hasColor1Vec4 = false;
                attributes.push_back(Color3_1);
                mvFillBuffer(model, model.accessors[attribute.index], rawBuffers.color1AttributeBuffer, 3);
            }
            else if (accessor.type == S_GLTF_VEC4)
            {
                rawBuffers.hasColor1Vec3 = false;
                rawBuffers.hasColor1Vec4 = true;
                attributes.push_back(Color4_1);
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
combine_vertex_buffer(unsigned int triangleCount, mvVertexLayout& modifiedLayout, sGLTFMeshPrimitive& glprimitive, std::vector<unsigned int> origIndexBuffer, RawAttributeBuffers& rawBuffers, std::vector<unsigned int>& indexBuffer, std::vector<float>& combinedVertexBuffer)
{
    for (size_t i = 0; i < triangleCount / 3; i++)
    {
        size_t i0 = glprimitive.indices_index == -1 ? i : origIndexBuffer[i];

        for (size_t j = 0; j < modifiedLayout.semantics.size(); j++)
        {
            std::string semantic = modifiedLayout.semantics[j];

            if (strcmp(semantic.c_str(), "Position") == 0)
            {
                float x0 = rawBuffers.positionAttributeBuffer[i0 * 3];
                float y0 = rawBuffers.positionAttributeBuffer[i0 * 3 + 1];
                float z0 = rawBuffers.positionAttributeBuffer[i0 * 3 + 2];

                // position
                combinedVertexBuffer.push_back(x0);
                combinedVertexBuffer.push_back(y0);
                combinedVertexBuffer.push_back(z0);
            }
            else if (strcmp(semantic.c_str(), "Joints") == 0)
            {
                if (modifiedLayout.indices[j] == 0)
                {
                    float x0 = rawBuffers.joints0AttributeBuffer[i0 * 4];
                    float y0 = rawBuffers.joints0AttributeBuffer[i0 * 4 + 1];
                    float z0 = rawBuffers.joints0AttributeBuffer[i0 * 4 + 2];
                    float w0 = rawBuffers.joints0AttributeBuffer[i0 * 4 + 3];

                    // position
                    combinedVertexBuffer.push_back(x0);
                    combinedVertexBuffer.push_back(y0);
                    combinedVertexBuffer.push_back(z0);
                    combinedVertexBuffer.push_back(w0);
                }
                else
                {
                    float x0 = rawBuffers.joints1AttributeBuffer[i0 * 4];
                    float y0 = rawBuffers.joints1AttributeBuffer[i0 * 4 + 1];
                    float z0 = rawBuffers.joints1AttributeBuffer[i0 * 4 + 2];
                    float w0 = rawBuffers.joints1AttributeBuffer[i0 * 4 + 3];

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
                    float x0 = rawBuffers.weights0AttributeBuffer[i0 * 4];
                    float y0 = rawBuffers.weights0AttributeBuffer[i0 * 4 + 1];
                    float z0 = rawBuffers.weights0AttributeBuffer[i0 * 4 + 2];
                    float w0 = rawBuffers.weights0AttributeBuffer[i0 * 4 + 3];

                    // position
                    combinedVertexBuffer.push_back(x0);
                    combinedVertexBuffer.push_back(y0);
                    combinedVertexBuffer.push_back(z0);
                    combinedVertexBuffer.push_back(w0);
                }
                else
                {
                    float x0 = rawBuffers.weights1AttributeBuffer[i0 * 4];
                    float y0 = rawBuffers.weights1AttributeBuffer[i0 * 4 + 1];
                    float z0 = rawBuffers.weights1AttributeBuffer[i0 * 4 + 2];
                    float w0 = rawBuffers.weights1AttributeBuffer[i0 * 4 + 3];

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
                    float nx0 = rawBuffers.normalAttributeBuffer[i0 * 3];
                    float ny0 = rawBuffers.normalAttributeBuffer[i0 * 3 + 1];
                    float nz0 = rawBuffers.normalAttributeBuffer[i0 * 3 + 2];

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
                    float tx0 = rawBuffers.tangentAttributeBuffer[i0 * 4];
                    float ty0 = rawBuffers.tangentAttributeBuffer[i0 * 4 + 1];
                    float tz0 = rawBuffers.tangentAttributeBuffer[i0 * 4 + 2];
                    float tw0 = rawBuffers.tangentAttributeBuffer[i0 * 4 + 3];

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
                    float u0 = rawBuffers.texture0AttributeBuffer[i0 * 2];
                    float v0 = rawBuffers.texture0AttributeBuffer[i0 * 2 + 1];
                    combinedVertexBuffer.push_back(u0);
                    combinedVertexBuffer.push_back(v0);
                }
                else
                {
                    float u1 = rawBuffers.texture1AttributeBuffer[i0 * 2];
                    float v1 = rawBuffers.texture1AttributeBuffer[i0 * 2 + 1];
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
                        float r0 = rawBuffers.color0AttributeBuffer[i0 * 4];
                        float g0 = rawBuffers.color0AttributeBuffer[i0 * 4 + 1];
                        float b0 = rawBuffers.color0AttributeBuffer[i0 * 4 + 2];
                        float a0 = rawBuffers.color0AttributeBuffer[i0 * 4 + 3];
                        combinedVertexBuffer.push_back(r0);  // we will calculate
                        combinedVertexBuffer.push_back(g0);  // we will calculate
                        combinedVertexBuffer.push_back(b0);  // we will calculate
                        combinedVertexBuffer.push_back(a0);  // we will calculate
                    }
                    else
                    {
                        float r0 = rawBuffers.color0AttributeBuffer[i0 * 3];
                        float g0 = rawBuffers.color0AttributeBuffer[i0 * 3 + 1];
                        float b0 = rawBuffers.color0AttributeBuffer[i0 * 3 + 2];

                        combinedVertexBuffer.push_back(r0);  // we will calculate
                        combinedVertexBuffer.push_back(g0);  // we will calculate
                        combinedVertexBuffer.push_back(b0);  // we will calculate
                    }
                }
                else
                {
                    if (modifiedLayout.formats[j] == DXGI_FORMAT_R32G32B32A32_FLOAT)
                    {
                        float r0 = rawBuffers.color1AttributeBuffer[i0 * 4];
                        float g0 = rawBuffers.color1AttributeBuffer[i0 * 4 + 1];
                        float b0 = rawBuffers.color1AttributeBuffer[i0 * 4 + 2];
                        float a0 = rawBuffers.color1AttributeBuffer[i0 * 4 + 3];
                        combinedVertexBuffer.push_back(r0);  // we will calculate
                        combinedVertexBuffer.push_back(g0);  // we will calculate
                        combinedVertexBuffer.push_back(b0);  // we will calculate
                        combinedVertexBuffer.push_back(a0);  // we will calculate
                    }
                    else
                    {
                        float r0 = rawBuffers.color1AttributeBuffer[i0 * 3];
                        float g0 = rawBuffers.color1AttributeBuffer[i0 * 3 + 1];
                        float b0 = rawBuffers.color1AttributeBuffer[i0 * 3 + 2];

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
finalize_vertex_buffers(RawAttributeBuffers& rawBuffers, mvVertexLayout& modifiedLayout, std::vector<float>& combinedVertexBuffer, std::vector<unsigned int>& indexBuffer, std::vector<float>& vertexBuffer)
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

        unsigned int currentLocation = 0u;
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
gather_target_attributes(sGLTFModel& model, sGLTFMeshPrimitive& glprimitive)
{
    std::vector<mvVertexElement> targetAttributes;

    bool hasPosition = false;
    bool hasNormal = false;
    bool hasTangent = false;
    bool hasTex0 = false;
    bool hasTex1 = false;
    bool hasColor0 = false;
    bool hasColor1 = false;

    for (int i = 0; i < glprimitive.target_count; i++)
    {
        sGLTFMorphTarget target = glprimitive.targets[i];
        for (int j = 0; j < target.attribute_count; j++)
        {
            sGLTFAttribute attribute = target.attributes[j];
            if (strcmp(attribute.semantic, "POSITION") == 0 && !hasPosition)
            {
                hasPosition = true;
                targetAttributes.push_back(Position3D);
            }
            else if (strcmp(attribute.semantic, "NORMAL") == 0 && !hasNormal)
            {
                hasNormal = true;
                targetAttributes.push_back(Normal);
            }
            else if (strcmp(attribute.semantic, "TANGENT") == 0 && !hasTangent)
            {
                hasTangent = true;
                targetAttributes.push_back(Tangent);
            }
            else if (strcmp(attribute.semantic, "TEXCOORD_0") == 0 && !hasTex0)
            {
                hasTex0 = true;
                targetAttributes.push_back(TexCoord0);
            }
            else if (strcmp(attribute.semantic, "TEXCOORD_1") == 0 && !hasTex1)
            {
                hasTex1 = true;
                targetAttributes.push_back(TexCoord1);
            }
            else if (strcmp(attribute.semantic, "COLOR_0") == 0 && !hasColor1)
            {
                hasColor0 = true;
                sGLTFAccessor& accessor = model.accessors[attribute.index];
                if (accessor.type == S_GLTF_VEC3)
                {
                    targetAttributes.push_back(Color3_0);
                }
                else if (accessor.type == S_GLTF_VEC4)
                {
                    targetAttributes.push_back(Color4_0);
                }
                else
                {
                    assert(false && "Undefined attribute type");
                }

            }

            else if (strcmp(attribute.semantic, "COLOR_1") == 0 && !hasColor1)
            {
                hasColor1 = true;
                sGLTFAccessor& accessor = model.accessors[attribute.index];
                if (accessor.type == S_GLTF_VEC3)
                {
                    targetAttributes.push_back(Color3_1);
                }
                else if (accessor.type == S_GLTF_VEC4)
                {
                    targetAttributes.push_back(Color4_1);
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

static void
load_gltf_meshes(mvGraphics& graphics, mvModel& mvmodel, sGLTFModel& model, float* minBoundary, float* maxBoundary)
{

    for (unsigned int currentMesh = 0u; currentMesh < model.mesh_count; currentMesh++)
    {
        sGLTFMesh& glmesh = model.meshes[currentMesh];

        mvMesh newMesh{};
        newMesh.name = glmesh.name;
        bool useMorphing = false;
        newMesh.weightCount = glmesh.weights_count;
        for (unsigned int currentWeight = 0; currentWeight < glmesh.weights_count; currentWeight++)
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

            // create transform constant buffer
            D3D11_BUFFER_DESC cbd;
            cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            cbd.Usage = D3D11_USAGE_DYNAMIC;
            cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            cbd.MiscFlags = 0u;
            cbd.ByteWidth = sizeof(float)*newMesh.weights.size();
            cbd.StructureByteStride = 0u;

            newMesh.morphBuffer.size = cbd.ByteWidth;
            graphics.device->CreateBuffer(&cbd, nullptr, &newMesh.morphBuffer.buffer);
            update_const_buffer(graphics, newMesh.morphBuffer, newMesh.weights.data());
        }

        for (unsigned int currentPrimitive = 0u; currentPrimitive < glmesh.primitives_count; currentPrimitive++)
        {

            sGLTFMeshPrimitive& glprimitive = glmesh.primitives[currentPrimitive];

            std::vector<unsigned int> origIndexBuffer;
            if (glprimitive.indices_index > -1)
            {
                unsigned char indexCompCount = mvGetAccessorItemCompCount(model.accessors[glprimitive.indices_index]);

                sGLTFComponentType indexCompType = model.accessors[glprimitive.indices_index].component_type;
                mvFillBuffer(model, model.accessors[glprimitive.indices_index], origIndexBuffer);
            }

            RawAttributeBuffers rawBuffers{};
            std::vector<mvVertexElement> attributes = load_raw_attribute_buffers(model, glprimitive, rawBuffers, minBoundary, maxBoundary);

            std::vector<unsigned int> indexBuffer;
            std::vector<float> vertexBuffer;

            unsigned int triangleCount = origIndexBuffer.size() * 3;

            if (glprimitive.indices_index == -1)
                triangleCount = rawBuffers.positionAttributeBuffer.size();

            if (rawBuffers.normalAttributeBuffer.empty())
            {
                attributes.push_back(Normal);
            }
            if (rawBuffers.tangentAttributeBuffer.empty())
            {
                attributes.push_back(Tangent);
            }

            mvVertexLayout modifiedLayout = create_vertex_layout(attributes);

            vertexBuffer.reserve(triangleCount * modifiedLayout.size * 3);
            indexBuffer.reserve(triangleCount * 3);

            std::vector<float> combinedVertexBuffer;

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

                std::unordered_map<mvVertexElement, int> attributeOffsets;
                int attributeOffset = 0;

                for (int i = 0; i < targetAttributes.size(); i++)
                {
                    switch (targetAttributes[i])
                    {
                    case Position3D:
                        materialData.extramacros.push_back({ "HAS_MORPH_TARGET_POSITION", "1" });
                        materialData.extramacros.push_back({ "MORPH_TARGET_POSITION_OFFSET", std::to_string(attributeOffset) });
                        break;
                    case Normal:
                        materialData.extramacros.push_back({ "HAS_MORPH_TARGET_NORMAL", "1" });
                        materialData.extramacros.push_back({ "MORPH_TARGET_NORMAL_OFFSET", std::to_string(attributeOffset) });
                        break;
                    case Tangent:
                        materialData.extramacros.push_back({ "HAS_MORPH_TARGET_TANGENT", "1" });
                        materialData.extramacros.push_back({ "MORPH_TARGET_TANGENT_OFFSET", std::to_string(attributeOffset) });
                        break;
                    case TexCoord0:
                        materialData.extramacros.push_back({ "HAS_MORPH_TARGET_TEXCOORD_0", "1" });
                        materialData.extramacros.push_back({ "MORPH_TARGET_TEXCOORD_0_OFFSET", std::to_string(attributeOffset) });
                        break;
                    case TexCoord1:
                        materialData.extramacros.push_back({ "HAS_MORPH_TARGET_TEXCOORD_1", "1" });
                        materialData.extramacros.push_back({ "MORPH_TARGET_TEXCOORD_1_OFFSET", std::to_string(attributeOffset) });
                        break;
                    }
                    
                    attributeOffsets[targetAttributes[i]] = attributeOffset;
                    attributeOffset += glprimitive.target_count;
                }

                int vertexCount = triangleCount * 3;
                float textureWidth = ceil(sqrt(vertexCount));
                float singleTextureSize = pow(textureWidth, 2) * 4;
                newMesh.primitives.back().morphData = new float[singleTextureSize * glprimitive.target_count * targetAttributes.size()];
                ZeroMemory(newMesh.primitives.back().morphData, sizeof(float)*singleTextureSize* glprimitive.target_count* targetAttributes.size());

                for (int i = 0; i < glprimitive.target_count; i++)
                {
                    sGLTFMorphTarget& target = glprimitive.targets[i];

                    for (const auto& item : attributeOffsets)
                    {
                        for (int j = 0; j < target.attribute_count; j++)
                        {
                            // todo: handle colors
                            mvVertexElement semantic = get_element_from_gltf_semantic(target.attributes[j].semantic);

                            if (item.first == semantic)
                            {
                                std::vector<float> data;
                                
                                sGLTFAccessor& accessor = model.accessors[target.attributes[j].index];
                                int offset = item.second * singleTextureSize;
                                mvFillBuffer(model, accessor, data);

                                std::vector<float> rdata;
                                if (accessor.type == S_GLTF_VEC2)
                                {
                                    rdata.resize(origIndexBuffer.size() * 2);
                                    for (int k = 0; k < origIndexBuffer.size(); k++)
                                    {
                                        unsigned int i0 = origIndexBuffer[k];
                                        rdata[k * 3] = data[i0 * 3];
                                        rdata[k * 3 + 1] = data[i0 * 3 + 1];
                                    }
                                }
                                else if (accessor.type == S_GLTF_VEC3)
                                {
                                    rdata.resize(origIndexBuffer.size() * 3);
                                    for (int k = 0; k < origIndexBuffer.size(); k++)
                                    {
                                        unsigned int i0 = origIndexBuffer[k];
                                        rdata[k * 3] = data[i0 * 3];
                                        rdata[k * 3 + 1] = data[i0 * 3 + 1];
                                        rdata[k * 3 + 2] = data[i0 * 3 + 2];
                                    }
                                }
                                else if (accessor.type == S_GLTF_VEC4)
                                {
                                    rdata.resize(origIndexBuffer.size() * 4);
                                    for (int k = 0; k < origIndexBuffer.size(); k++)
                                    {
                                        unsigned int i0 = origIndexBuffer[k];
                                        rdata[k * 3] = data[i0 * 3];
                                        rdata[k * 3 + 1] = data[i0 * 3 + 1];
                                        rdata[k * 3 + 2] = data[i0 * 3 + 2];
                                        rdata[k * 3 + 3] = data[i0 * 3 + 3];
                                    }
                                }


                                switch (accessor.type)
                                {
                                case S_GLTF_VEC2:
                                case S_GLTF_VEC3:
                                {
                                    int paddingOffset = 0;
                                    int accessorOffset = 0;
                                    int componentCount = accessor.type == S_GLTF_VEC2 ? 2 : 3;
                                    for (int k = 0; k < origIndexBuffer.size(); k++)
                                    {
                                        memcpy(&newMesh.primitives.back().morphData[offset + paddingOffset], &rdata[accessorOffset], sizeof(float)*componentCount);
                                        paddingOffset += 4;
                                        accessorOffset += componentCount;
                                    }
                                    break;
                                }
                                case S_GLTF_VEC4:
                                {
                                    memcpy(&newMesh.primitives.back().morphData[offset], rdata.data(), rdata.size() * sizeof(float));
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
                newMesh.primitives.back().morphTexture = create_texture(graphics, textureWidth, textureWidth, glprimitive.target_count* targetAttributes.size(), newMesh.primitives.back().morphData);
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

                sGLTFMaterial& material = model.materials[glprimitive.material_index];

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

                newMesh.primitives.back().albedoTexture = setup_texture(graphics, model, currentPrimitive, material.base_color_texture, materialData.hasAlbedoMap, newMesh.name, "_a");
                newMesh.primitives.back().normalTexture = setup_texture(graphics, model, currentPrimitive, material.normal_texture, materialData.hasNormalMap, newMesh.name, "_n");
                newMesh.primitives.back().metalRoughnessTexture = setup_texture(graphics, model, currentPrimitive, material.metallic_roughness_texture, materialData.hasMetallicRoughnessMap, newMesh.name, "_m");
                newMesh.primitives.back().emissiveTexture = setup_texture(graphics, model, currentPrimitive, material.emissive_texture, materialData.hasEmmissiveMap, newMesh.name, "_e");
                newMesh.primitives.back().occlusionTexture = setup_texture(graphics, model, currentPrimitive, material.occlusion_texture, materialData.hasOcculusionMap, newMesh.name, "_o");
                newMesh.primitives.back().clearcoatTexture = setup_texture(graphics, model, currentPrimitive, material.clearcoat_texture, materialData.hasClearcoatMap, newMesh.name, "_cc");
                newMesh.primitives.back().clearcoatRoughnessTexture = setup_texture(graphics, model, currentPrimitive, material.clearcoat_roughness_texture, materialData.hasClearcoatRoughnessMap, newMesh.name, "_ccr");
                newMesh.primitives.back().clearcoatNormalTexture = setup_texture(graphics, model, currentPrimitive, material.clearcoat_normal_texture, materialData.hasClearcoatNormalMap, newMesh.name, "_ccn");
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

            newMesh.primitives.back().materialID = mvGetMaterialAssetID(&mvmodel.materialManager, hash);
            if (newMesh.primitives.back().materialID == -1)
            {
                newMesh.primitives.back().materialID = register_asset(&mvmodel.materialManager, hash, create_material(graphics, "PBR_VS.hlsl", "PBR_PS.hlsl", materialData));
            }

            newMesh.primitives.back().vertexBuffer = create_buffer(graphics, vertexBuffer.data(), vertexBuffer.size() * sizeof(float), D3D11_BIND_VERTEX_BUFFER);
            newMesh.primitives.back().indexBuffer = create_buffer(graphics, indexBuffer.data(), indexBuffer.size() * sizeof(unsigned int), D3D11_BIND_INDEX_BUFFER);

        }

        mvmodel.meshes.push_back(newMesh);

    }

}

static void
load_gltf_nodes(mvModel& mvmodel, sGLTFModel& model)
{

    for (unsigned int currentNode = 0u; currentNode < model.node_count; currentNode++)
    {
        sGLTFNode& glnode = model.nodes[currentNode];

        mvNode newNode{};
        newNode.name = glnode.name;
        if (glnode.mesh_index > -1)
            newNode.mesh = glnode.mesh_index;
        if (glnode.skin_index > -1)
            newNode.skin = glnode.skin_index;
        if (glnode.camera_index > -1)
            newNode.camera = glnode.camera_index;

        newNode.childCount = glnode.child_count;

        for (int i = 0; i < glnode.child_count; i++)
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

        mvmodel.nodes.push_back(newNode);
    }

}

static void
load_gltf_animations(mvModel& mvmodel, sGLTFModel& model)
{

    for (unsigned int currentAnimation = 0u; currentAnimation < model.animation_count; currentAnimation++)
    {
        sGLTFAnimation& glanimation = model.animations[currentAnimation];

        mvAnimation animation{};
        animation.channels = new mvAnimationChannel[glanimation.channel_count];
        animation.channelCount = glanimation.channel_count;

        for (unsigned int channel_index = 0u; channel_index < glanimation.channel_count; channel_index++)
        {
            sGLTFAnimationChannel& glchannel = glanimation.channels[channel_index];
            sGLTFAnimationSampler& glsampler = glanimation.samplers[glchannel.sampler];
            mvAnimationChannel& channel = animation.channels[channel_index];
            channel.node = glchannel.target.node;
            channel.path = glchannel.target.path;
            channel.interpolation = glsampler.interpolation;

            sGLTFAccessor& inputaccessor = model.accessors[glsampler.input];
            sGLTFAccessor& outputaccessor = model.accessors[glsampler.output];
            mvFillBuffer(model, inputaccessor, channel.inputdata, 1);

            sGLTFComponentType indexCompType = outputaccessor.component_type;

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

        mvmodel.animations.push_back(animation);
    }

}

mvModel
load_gltf_assets(mvGraphics& graphics, sGLTFModel& model)
{
    mvModel mvmodel{};
    float maxBoundary[3] = { -FLT_MAX , -FLT_MAX , -FLT_MAX };
    float minBoundary[3] = { FLT_MAX , FLT_MAX , FLT_MAX };
    mvmodel.loaded = true;
    load_gltf_skins(graphics, mvmodel, model);
    load_gltf_meshes(graphics, mvmodel, model, minBoundary, maxBoundary);
    load_gltf_nodes(mvmodel, model);
    load_gltf_animations(mvmodel, model);

    for (unsigned int currentCamera = 0u; currentCamera < model.camera_count; currentCamera++)
    {
        sGLTFCamera& glcamera = model.cameras[currentCamera];
        mvCamera camera{};

        if (glcamera.type == S_GLTF_PERSPECTIVE)
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

        mvmodel.cameras.push_back(camera);
    }


    for (int i = 0; i < 3; i++)
    {
        mvmodel.minBoundary[i] = minBoundary[i];
        mvmodel.maxBoundary[i] = maxBoundary[i];
    }

    // updates based on correct offset mapping
    for (unsigned int currentAnimation = 0u; currentAnimation < model.animation_count; currentAnimation++)
    {
        mvAnimation& animation = mvmodel.animations[currentAnimation];

        for (unsigned int channel_index = 0u; channel_index < animation.channelCount; channel_index++)
        {
            mvAnimationChannel& channel = animation.channels[channel_index];
            mvmodel.nodes[channel.node].animated = true;
        }
    }

    for (unsigned int currentNode = 0u; currentNode < model.node_count; currentNode++)
    {

        mvNode& node = mvmodel.nodes[currentNode];

        if (node.camera > -1)
        {

            mvCamera& camera = mvmodel.cameras[node.camera];

            if (camera.type == MV_CAMERA_PERSPECTIVE)
            {
                mvMesh frustum1 = create_frustum2(graphics, camera.fieldOfView * 180.0f / PI, camera.aspectRatio, camera.nearZ, camera.farZ);
                mvmodel.meshes.push_back(frustum1);
                node.mesh = mvmodel.meshes.size() - 1;
            }
            else
            {
                mvMesh frustum1 = create_ortho_frustum(graphics, camera.width, camera.height, camera.nearZ, camera.farZ);
                mvmodel.meshes.push_back(frustum1);
                node.mesh = mvmodel.meshes.size() - 1;
            }
        }
    }

    mvAssetID defaultScene = -1;

    for (unsigned int currentScene = 0u; currentScene < model.scene_count; currentScene++)
    {
        sGLTFScene& glscene = model.scenes[currentScene];

        mvScene newScene{};
        newScene.nodeCount = glscene.node_count;

        for (int i = 0; i < glscene.node_count; i++)
            newScene.nodes[i] = glscene.nodes[i];


        mvmodel.scenes.push_back(newScene);

        if (currentScene == model.scene)
            defaultScene = mvmodel.scenes.size()-1;
    }

    mvmodel.defaultScene = defaultScene;
    return mvmodel;
}

void
unload_gltf_assets(mvModel& model)
{
    model.loaded = false;
    model.defaultScene = -1;
    model.skins.clear();
    model.cameras.clear();
    model.meshes.clear();
    model.nodes.clear();
    model.animations.clear();
    model.scenes.clear();
}