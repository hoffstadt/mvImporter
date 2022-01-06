#include "mvMesh.h"
#include <assert.h>
#include <cmath>
#include "mvSandbox.h"
#include "mvAssetManager.h"

#define MV_IMPORTER_IMPLEMENTATION
#include "mvImporter.h"

mvMesh
create_cube(mvAssetManager& assetManager, f32 size)
{

    mvVertexLayout layout = create_vertex_layout(
        {
            mvVertexElement::Position3D
        }
    );

    const float side = size;
    auto vertices = std::vector<f32>{
        -side, -side, -side,  // 0 near side
         side, -side, -side,  // 1
        -side,  side, -side,  // 2
         side,  side, -side,  // 3
        -side, -side,  side,  // 4 far side
         side, -side,  side,  // 5
        -side,  side,  side,  // 6
         side,  side,  side,  // 7
        -side, -side, -side,  // 8 left side
        -side,  side, -side,  // 9
        -side, -side,  side,  // 10
        -side,  side,  side,  // 11
         side, -side, -side,  // 12 right side
         side,  side, -side,  // 13
         side, -side,  side,  // 14
         side,  side,  side,  // 15
        -side, -side, -side,  // 16 bottom side
         side, -side, -side,  // 17
        -side, -side,  side,  // 18
         side, -side,  side,  // 19
        -side,  side, -side,  // 20 top side
         side,  side, -side,  // 21
        -side,  side,  side,  // 22
         side,  side,  side   // 23
    };

    static auto indices = std::vector<u32>{
        1,  2,  0,  1,  3,  2,
        7,  5,  4,  6,  7,  4,
        9, 10,  8, 9, 11,  10,
        15, 13, 12, 14, 15, 12,
        18, 17, 16, 19, 17, 18,
        21, 23, 20, 23, 22, 20
    };

    mvMesh mesh{};
    mesh.name = "cube";
    mesh.primitives.push_back({});
    mesh.primitives.back().layout = layout;
    mesh.primitives.back().vertexBuffer = mvGetBufferAssetID(&assetManager,
        "cube_vertex" + std::to_string(side),
        vertices.data(),
        vertices.size() * sizeof(f32),
        D3D11_BIND_VERTEX_BUFFER);
    mesh.primitives.back().indexBuffer = mvGetBufferAssetID(&assetManager, 
        "cube_index",
        indices.data(), 
        indices.size() * sizeof(u32), 
        D3D11_BIND_INDEX_BUFFER);

    return mesh;
}

mvMesh
create_textured_cube(mvAssetManager& assetManager, f32 size)
{

    mvVertexLayout layout = create_vertex_layout(
        {
            mvVertexElement::Position3D,
            mvVertexElement::Normal,
            mvVertexElement::TexCoord0,
            mvVertexElement::Tangent
        }
    );

    const float side = size;
    auto vertices = std::vector<f32>{
        -side, -side, -side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 0 near side
         side, -side, -side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 1
        -side,  side, -side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 2
         side,  side, -side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 3

        -side, -side,  side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 4 far side
         side, -side,  side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 5
        -side,  side,  side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 6
         side,  side,  side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 7

        -side, -side, -side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 8 left side
        -side,  side, -side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 9
        -side, -side,  side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 10
        -side,  side,  side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 11

         side, -side, -side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 12 right side
         side,  side, -side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 13
         side, -side,  side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 14
         side,  side,  side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 15

        -side, -side, -side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 16 bottom side
         side, -side, -side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 17
        -side, -side,  side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 18
         side, -side,  side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 19

        -side,  side, -side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 20 top side
         side,  side, -side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 21
        -side,  side,  side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 22
         side,  side,  side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f   // 23
    };

    static auto indices = std::vector<u32>{
        1,  2,  0,  1,  3,  2,
        7,  5,  4,  6,  7,  4,
        9, 10,  8, 9, 11,  10,
        15, 13, 12, 14, 15, 12,
        18, 17, 16, 19, 17, 18,
        21, 23, 20, 23, 22, 20
    };

    for (size_t i = 0; i < indices.size(); i += 3)
    {

        mvVec3 p0 = { vertices[11 * indices[i]], vertices[14 * indices[i] + 1], vertices[14 * indices[i] + 2] };
        mvVec3 p1 = { vertices[11 * indices[i+1]], vertices[14 * indices[i + 1] + 1], vertices[14 * indices[i + 1] + 2] };
        mvVec3 p2 = { vertices[11 * indices[i+2]], vertices[14 * indices[i + 2] + 1], vertices[14 * indices[i + 2] + 2] };

        mvVec3 n = normalize(cross(p1 - p0, p2 - p0));
        vertices[11 * indices[i] + 3] = n[0];
        vertices[11 * indices[i] + 4] = n[1];
        vertices[11 * indices[i] + 5] = n[2];
        vertices[11 * indices[i + 1] + 3] = n[0];
        vertices[11 * indices[i + 1] + 4] = n[1];
        vertices[11 * indices[i + 1] + 5] = n[2];
        vertices[11 * indices[i + 2] + 3] = n[0];
        vertices[11 * indices[i + 2] + 4] = n[1];
        vertices[11 * indices[i + 2] + 5] = n[2];
    }

    mvMesh mesh{};
    mesh.name = "textured cube";
    mesh.primitives.push_back({});
    mesh.primitives.back().layout = layout;
    mesh.primitives.back().vertexBuffer = mvGetBufferAssetID(&assetManager,
        "textured_cube_vertex" + std::to_string(side),
        vertices.data(), 
        vertices.size() * sizeof(f32), 
        D3D11_BIND_VERTEX_BUFFER);
    mesh.primitives.back().indexBuffer = mvGetBufferAssetID(&assetManager, 
        "textured_cube_index",
        indices.data(), 
        indices.size() * sizeof(u32), 
        D3D11_BIND_INDEX_BUFFER);

    return mesh;
}

mvMesh
create_textured_quad(mvAssetManager& assetManager, f32 size)
{

    mvVertexLayout layout = create_vertex_layout(
        {
            mvVertexElement::Position3D,
            mvVertexElement::Normal,
            mvVertexElement::TexCoord0,
            mvVertexElement::Tangent
        }
    );

    const float side = size;
    auto vertices = std::vector<f32>{
        -side,  side, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
         side, -side, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        -side, -side, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
         side,  side, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    };

    static auto indices = std::vector<u32>{
        1, 0, 2,
        3, 0, 1
    };

    for (size_t i = 0; i < indices.size(); i += 3)
    {
        mvVec3 p0 = { vertices[11 * indices[i]], vertices[11 * indices[i] + 1], vertices[11 * indices[i] + 2] };
        mvVec3 p1 = { vertices[11 * indices[i + 1]], vertices[11 * indices[i + 1] + 1], vertices[11 * indices[i + 1] + 2] };
        mvVec3 p2 = { vertices[11 * indices[i + 2]], vertices[11 * indices[i + 2] + 1], vertices[11 * indices[i + 2] + 2] };

        mvVec3 n = normalize(cross(p1 - p0, p2 - p0));
        vertices[11 * indices[i] + 3] = n[0];
        vertices[11 * indices[i] + 4] = n[1];
        vertices[11 * indices[i] + 5] = n[2];
        vertices[11 * indices[i + 1] + 3] = n[0];
        vertices[11 * indices[i + 1] + 4] = n[1];
        vertices[11 * indices[i + 1] + 5] = n[2];
        vertices[11 * indices[i + 2] + 3] = n[0];
        vertices[11 * indices[i + 2] + 4] = n[1];
        vertices[11 * indices[i + 2] + 5] = n[2];
    }

    mvMesh mesh{};

    mesh.name = "textured quad";
    mesh.primitives.push_back({});
    mesh.primitives.back().layout = layout;
    mesh.primitives.back().vertexBuffer = mvGetBufferAssetID(&assetManager, 
        "textured_quad_vertex",
        vertices.data(), 
        vertices.size() * sizeof(f32),
        D3D11_BIND_VERTEX_BUFFER);
    mesh.primitives.back().indexBuffer = mvGetBufferAssetID(&assetManager,
        "textured_quad_index",
        indices.data(), 
        indices.size()*sizeof(u32), 
        D3D11_BIND_INDEX_BUFFER);

    return mesh;
}

mvMesh
create_frustum(mvAssetManager& assetManager, f32 width, f32 height, f32 nearZ, f32 farZ)
{
    static u32 id = 0;
    id++;

    mvVertexLayout layout = create_vertex_layout(
        {
            mvVertexElement::Position3D
        }
    );

    f32 smallWidth = atan(width / (2.0f * farZ));
    f32 smallHeight = atan(height / (2.0f * farZ));

    auto vertices = std::vector<f32>{
         smallWidth,  smallHeight, nearZ,
        -smallWidth,  smallHeight, nearZ,
        -smallWidth, -smallHeight, nearZ,
         smallWidth, -smallHeight, nearZ,
         width / 2.0f,  height / 2.0f, farZ,
        -width / 2.0f,  height / 2.0f, farZ,
        -width / 2.0f, -height / 2.0f, farZ,
         width / 2.0f, -height / 2.0f, farZ,
    };

    static auto indices = std::vector<u32>{
        0, 1,
        1, 2,
        2, 3,
        3, 0,
        4, 5,
        5, 6,
        6, 7,
        7, 4,
        0, 4,
        3, 7,
        1, 5,
        2, 6
    };

    mvMesh mesh{};

    mesh.name = "frustum1_" + std::to_string(id);
    mesh.primitives.push_back({});
    mesh.primitives.back().layout = layout;
    mesh.primitives.back().vertexBuffer = mvGetBufferAssetID(&assetManager, 
        "frustum_vertex1_" + std::to_string(id),
        vertices.data(), 
        vertices.size() * sizeof(f32), 
        D3D11_BIND_VERTEX_BUFFER);
    mesh.primitives.back().indexBuffer = mvGetBufferAssetID(&assetManager, 
        "frustum_index1",
        indices.data(), 
        indices.size() * sizeof(u32), 
        D3D11_BIND_INDEX_BUFFER);

    return mesh;
}

mvMesh
create_frustum2(mvAssetManager& assetManager, f32 fov, f32 aspect, f32 nearZ, f32 farZ)
{
    static u32 id = 0;
    id++;
    mvVertexLayout layout = create_vertex_layout(
        {
            mvVertexElement::Position3D
        }
    );

    f32 smallWidth = tan(fov) * nearZ;
    f32 smallHeight = smallWidth / aspect;

    f32 bigWidth = tan(fov) * farZ;
    f32 bigHeight = bigWidth / aspect;


    auto vertices = std::vector<f32>{
         smallWidth,  smallHeight, nearZ,
        -smallWidth,  smallHeight, nearZ,
        -smallWidth, -smallHeight, nearZ,
         smallWidth, -smallHeight, nearZ,
         bigWidth,  bigHeight, farZ,
        -bigWidth,  bigHeight, farZ,
        -bigWidth, -bigHeight, farZ,
         bigWidth, -bigHeight, farZ,
    };

    static auto indices = std::vector<u32>{
        0, 1,
        1, 2,
        2, 3,
        3, 0,
        4, 5,
        5, 6,
        6, 7,
        7, 4,
        0, 4,
        3, 7,
        1, 5,
        2, 6
    };

    mvMesh mesh{};

    mesh.name = "frustum2_" + std::to_string(id);
    mesh.primitives.push_back({});
    mesh.primitives.back().layout = layout;
    mesh.primitives.back().vertexBuffer = mvGetBufferAssetID(&assetManager, 
        "frustum_vertex2_" + std::to_string(id),
        vertices.data(), 
        vertices.size() * sizeof(f32), 
        D3D11_BIND_VERTEX_BUFFER);
    mesh.primitives.back().indexBuffer = mvGetBufferAssetID(&assetManager, 
        "frustum_index2",
        indices.data(), 
        indices.size() * sizeof(u32), 
        D3D11_BIND_INDEX_BUFFER);

    return mesh;
}

mvMesh
create_ortho_frustum(mvAssetManager& assetManager, f32 width, f32 height, f32 nearZ, f32 farZ)
{
    static u32 id = 0;
    id++;

    mvVertexLayout layout = create_vertex_layout(
        {
            mvVertexElement::Position3D
        }
    );

    auto vertices = std::vector<f32>{
         width,  height, nearZ,
        -width,  height, nearZ,
        -width, -height, nearZ,
         width, -height, nearZ,
         width / 2.0f,  height / 2.0f, farZ,
        -width / 2.0f,  height / 2.0f, farZ,
        -width / 2.0f, -height / 2.0f, farZ,
         width / 2.0f, -height / 2.0f, farZ,
    };

    static auto indices = std::vector<u32>{
        0, 1,
        1, 2,
        2, 3,
        3, 0,
        4, 5,
        5, 6,
        6, 7,
        7, 4,
        0, 4,
        3, 7,
        1, 5,
        2, 6
    };

    mvMesh mesh{};

    mesh.name = "frustum3_" + std::to_string(id);
    mesh.primitives.push_back({});
    mesh.primitives.back().layout = layout;
    mesh.primitives.back().vertexBuffer = mvGetBufferAssetID(&assetManager,
        "frustum_vertex3_" + std::to_string(id),
        vertices.data(),
        vertices.size() * sizeof(f32),
        D3D11_BIND_VERTEX_BUFFER);
    mesh.primitives.back().indexBuffer = mvGetBufferAssetID(&assetManager,
        "frustum_index3",
        indices.data(),
        indices.size() * sizeof(u32),
        D3D11_BIND_INDEX_BUFFER);

    return mesh;
}

void
compute_joints(mvAssetManager& am, mvNode& parentNode, mvSkin& skin)
{
    u32 textureWidth = ceil(sqrt(skin.jointCount * 8));

    for (u32 i = 0; i < skin.jointCount; i++)
    {
        s32 joint = skin.joints[i];
        mvNode& node = am.nodes[joint].asset;

        //mvMat4 jointMatrix = (* (mvMat4*)&skin.inverseBindMatrices[i * 16]) * node.worldTransform * parentNode.inverseWorldTransform;
        //mvMat4 jointMatrix = (*(mvMat4*)&skin.inverseBindMatrices[i * 16]) * parentNode.inverseWorldTransform * node.worldTransform;
        mvMat4 jointMatrix = node.worldTransform * parentNode.inverseWorldTransform * (*(mvMat4*)&skin.inverseBindMatrices[i * 16]);

        //mvMat4 jointMatrix = node.worldTransform * (*(mvMat4*)&skin.inverseBindMatrices[i * 16]);
        //jointMatrix = parentNode.inverseWorldTransform * jointMatrix;

        mvMat4 normalMatrix = transpose(invert(jointMatrix));

        *(mvMat4*)&skin.textureData[i * 32] = jointMatrix;
        *(mvMat4*)&skin.textureData[i * 32+16] = normalMatrix;
    }

    update_dynamic_texture(skin.jointTexture, textureWidth, textureWidth, skin.textureData);
}

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

static void
mvVerifyBufferViewStride(mvGLTFModel& model, mvGLTFAccessor& accessor)
{
    mvGLTFBufferView& bufferview = model.bufferviews[accessor.buffer_view_index];

    u8 actualItemCompCount = mvGetAccessorItemCompCount(accessor);

    // calculate stride if not available
    //if (bufferview.byte_stride == -1)
    {
        switch (accessor.component_type)
        {
        case MV_IMP_UNSIGNED_BYTE:
        case MV_IMP_BYTE:
            bufferview.byte_stride = 1 * actualItemCompCount;
            break;

        case MV_IMP_UNSIGNED_SHORT:
        case MV_IMP_SHORT:
            bufferview.byte_stride = 2 * actualItemCompCount;
            break;

        case MV_IMP_FLOAT:
        case MV_IMP_INT:
        case MV_IMP_UNSIGNED_INT:
            bufferview.byte_stride = 4 * actualItemCompCount;
            break;

        case MV_IMP_DOUBLE:
            bufferview.byte_stride = 8 * actualItemCompCount;
            break;
        }
    }

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
mvFillBuffer(mvGLTFModel& model, mvGLTFAccessor& accessor, std::vector<W>& outBuffer, u32 componentCap = 4)
{
    mvVerifyBufferViewStride(model, accessor);
    mvGLTFBufferView bufferView = model.bufferviews[accessor.buffer_view_index];
    char* bufferRawData = (char*)model.buffers[bufferView.buffer_index].data.data();
    char* bufferRawSection = &bufferRawData[bufferView.byte_offset + accessor.byteOffset]; // start of buffer section

    u8 actualItemCompCount = mvGetAccessorItemCompCount(accessor);

    u8 accessorItemCompCount = actualItemCompCount;
    if (componentCap < accessorItemCompCount)
        accessorItemCompCount = componentCap;

    for (i32 i = 0; i < accessor.count; i++)
    {
        T* values = (T*)&bufferRawSection[i*bufferView.byte_stride];

        for (i32 j = 0; j < accessorItemCompCount; j++)
            outBuffer.push_back((W)values[j]);
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

mvAssetID
load_gltf_assets(mvAssetManager& assetManager, mvGLTFModel& model)
{

    std::vector<mvAssetID> cameraMapping;
    cameraMapping.resize(model.camera_count);

    std::vector<mvAssetID> nodeMapping;
    nodeMapping.resize(model.node_count);

    std::vector<mvAssetID> meshMapping;
    meshMapping.resize(model.mesh_count);

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

        mvFillBuffer<f32>(model, model.accessors[glskin.inverseBindMatrices], skin.inverseBindMatrices, 16);

        u32 textureWidth = ceil(sqrt(skin.jointCount * 8));

        skin.jointTexture = create_dynamic_texture(textureWidth, textureWidth);
        skin.textureData = new f32[textureWidth * textureWidth * 4];

        skinMapping[currentSkin] = register_asset(&assetManager, glskin.name, skin);
    }

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

    for (u32 currentMesh = 0u; currentMesh < model.mesh_count; currentMesh++)
    {
        mvGLTFMesh& glmesh = model.meshes[currentMesh];

        mvMesh newMesh{};
        newMesh.name = glmesh.name;

        for (u32 currentPrimitive = 0u; currentPrimitive < glmesh.primitives_count; currentPrimitive++)
        {

            mvGLTFMeshPrimitive& glprimitive = glmesh.primitives[currentPrimitive];

            std::vector<u32> origIndexBuffer;
            u8 indexCompCount = mvGetAccessorItemCompCount(model.accessors[glprimitive.indices_index]);

            mvGLTFComponentType indexCompType = model.accessors[glprimitive.indices_index].component_type;

            switch (indexCompType)
            {
            case MV_IMP_BYTE:
            case MV_IMP_UNSIGNED_BYTE:
                mvFillBuffer<u8>(model, model.accessors[glprimitive.indices_index], origIndexBuffer);
                break;

            case MV_IMP_SHORT:
            case MV_IMP_UNSIGNED_SHORT:
                mvFillBuffer<u16>(model, model.accessors[glprimitive.indices_index], origIndexBuffer);
                break;

            case MV_IMP_INT:
            case MV_IMP_UNSIGNED_INT:
                mvFillBuffer<u32>(model, model.accessors[glprimitive.indices_index], origIndexBuffer);
                break;

            default:
                assert(false && "Unknown index compenent type");
            }

            std::vector<f32> positionAttributeBuffer;
            std::vector<f32> tangentAttributeBuffer;
            std::vector<f32> normalAttributeBuffer;
            std::vector<f32> texture0AttributeBuffer;
            std::vector<f32> texture1AttributeBuffer;
            std::vector<f32> colorAttributeBuffer;
            std::vector<f32> joints0AttributeBuffer;
            std::vector<f32> joints1AttributeBuffer;
            std::vector<f32> weights0AttributeBuffer;
            std::vector<f32> weights1AttributeBuffer;

            b8 calculateNormals = true;
            b8 calculateTangents = true;
            b8 hasTexCoord0 = false;
            b8 hasTexCoord1 = false;
            b8 hasColorVec3 = false;
            b8 hasColorVec4 = false;
            b8 hasJoints0 = false;
            b8 hasJoints1 = false;
            b8 hasWeights0 = false;
            b8 hasWeights1 = false;
            b8 useSkinning = false;

            std::vector<mvVertexElement> attributes;
            for (u32 i = 0; i < glprimitive.attribute_count; i++)
            {
                auto& attribute = glprimitive.attributes[i];
                if (strcmp(attribute.semantic.c_str(), "POSITION") == 0)
                {
                    attributes.push_back(mvVertexElement::Position3D);
                    mvFillBuffer<f32>(model, model.accessors[attribute.index], positionAttributeBuffer);
                }
                else if (strcmp(attribute.semantic.c_str(), "NORMAL") == 0)
                {
                    calculateNormals = false;
                    attributes.push_back(mvVertexElement::Normal);
                    mvFillBuffer<f32>(model, model.accessors[attribute.index], normalAttributeBuffer);
                }
                else if (strcmp(attribute.semantic.c_str(), "TANGENT") == 0)
                {
                    calculateTangents = false;
                    attributes.push_back(mvVertexElement::Tangent);
                    mvFillBuffer<f32>(model, model.accessors[attribute.index], tangentAttributeBuffer, 4);
                }
                else if (strcmp(attribute.semantic.c_str(), "JOINTS_0") == 0)
                {
                    hasJoints0 = true;
                    useSkinning = true;
                    attributes.push_back(mvVertexElement::Joints0);
                    mvFillBuffer<f32>(model, model.accessors[attribute.index], joints0AttributeBuffer, 4);
                }
                else if (strcmp(attribute.semantic.c_str(), "JOINTS_1") == 0)
                {
                    hasJoints1 = true;
                    useSkinning = true;
                    attributes.push_back(mvVertexElement::Joints1);
                    mvFillBuffer<f32>(model, model.accessors[attribute.index], joints1AttributeBuffer, 4);
                }
                else if (strcmp(attribute.semantic.c_str(), "WEIGHTS_0") == 0)
                {
                    hasWeights0 = true;
                    useSkinning = true;
                    attributes.push_back(mvVertexElement::Weights0);
                    mvFillBuffer<f32>(model, model.accessors[attribute.index], weights0AttributeBuffer, 4);
                }
                else if (strcmp(attribute.semantic.c_str(), "WEIGHTS_1") == 0)
                {
                    hasWeights1 = true;
                    useSkinning = true;
                    attributes.push_back(mvVertexElement::Weights1);
                    mvFillBuffer<f32>(model, model.accessors[attribute.index], weights1AttributeBuffer, 4);
                }
                else if (strcmp(attribute.semantic.c_str(), "TEXCOORD_0") == 0)
                {
                    hasTexCoord0 = true;
                    attributes.push_back(mvVertexElement::TexCoord0);
                    mvFillBuffer<f32>(model, model.accessors[attribute.index], texture0AttributeBuffer, 2);
                }
                else if (strcmp(attribute.semantic.c_str(), "TEXCOORD_1") == 0)
                {
                    hasTexCoord1 = true;
                    attributes.push_back(mvVertexElement::TexCoord1);
                    mvFillBuffer<f32>(model, model.accessors[attribute.index], texture1AttributeBuffer, 2);
                }
                else if (strcmp(attribute.semantic.c_str(), "COLOR_0") == 0)
                {
                    mvGLTFAccessor& accessor = model.accessors[attribute.index];
                    if (accessor.type == MV_IMP_VEC3)
                    {
                        hasColorVec3 = true;
                        attributes.push_back(mvVertexElement::Color3);

                        switch (accessor.component_type)
                        {
                        case MV_IMP_UNSIGNED_BYTE:
                            mvFillBuffer<u8>(model, model.accessors[attribute.index], colorAttributeBuffer, 3);
                            break;
                        case MV_IMP_UNSIGNED_SHORT:
                            mvFillBuffer<u16>(model, model.accessors[attribute.index], colorAttributeBuffer, 3);
                            break;
                        case MV_IMP_FLOAT:
                            mvFillBuffer<f32>(model, model.accessors[attribute.index], colorAttributeBuffer, 3);
                            break;

                        default:
                        {
                            assert(false && "Undefined attribute type");
                        }
                                
                        }
                        
                    }
                    else if (accessor.type == MV_IMP_VEC4)
                    {
                        hasColorVec4 = true;
                        attributes.push_back(mvVertexElement::Color4);
                        switch (accessor.component_type)
                        {
                        case MV_IMP_UNSIGNED_BYTE:
                            mvFillBuffer<u8>(model, model.accessors[attribute.index], colorAttributeBuffer, 4);
                            break;
                        case MV_IMP_UNSIGNED_SHORT:
                            mvFillBuffer<u16>(model, model.accessors[attribute.index], colorAttributeBuffer, 4);
                            break;
                        case MV_IMP_FLOAT:
                            mvFillBuffer<f32>(model, model.accessors[attribute.index], colorAttributeBuffer, 4);
                            break;

                        default:
                        {
                            assert(false && "Undefined attribute type");
                        }
                        }
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

            std::vector<u32> indexBuffer;
            std::vector<f32> vertexBuffer;

            u32 triangleCount = origIndexBuffer.size() / 3;

            //vertexBuffer.reserve(triangleCount * 12 * 3);

            if (calculateNormals)
            {
                attributes.push_back(mvVertexElement::Normal);
            }
            if (calculateTangents)
            {
                attributes.push_back(mvVertexElement::Tangent);
            }

            mvVertexLayout modifiedLayout = create_vertex_layout(attributes);

            vertexBuffer.reserve(triangleCount * modifiedLayout.size * 3);
            indexBuffer.reserve(triangleCount * 3);

            std::vector<f32> combinedVertexBuffer;

            for (size_t i = 0; i < origIndexBuffer.size(); i++)
            {
                size_t i0 = origIndexBuffer[i];

                for (size_t j = 0; j < modifiedLayout.semantics.size(); j++)
                {
                    std::string semantic = modifiedLayout.semantics[j];

                    if (strcmp(semantic.c_str(), "Position") == 0)
                    {
                        f32 x0 = positionAttributeBuffer[i0 * 3];
                        f32 y0 = positionAttributeBuffer[i0 * 3 + 1];
                        f32 z0 = positionAttributeBuffer[i0 * 3 + 2];

                        // position
                        combinedVertexBuffer.push_back(x0);
                        combinedVertexBuffer.push_back(y0);
                        combinedVertexBuffer.push_back(z0);
                    }
                    else if (strcmp(semantic.c_str(), "Joints") == 0)
                    {
                        if (modifiedLayout.indices[j] == 0)
                        {
                            f32 x0 = joints0AttributeBuffer[i0 * 3];
                            f32 y0 = joints0AttributeBuffer[i0 * 3 + 1];
                            f32 z0 = joints0AttributeBuffer[i0 * 3 + 2];
                            f32 w0 = joints0AttributeBuffer[i0 * 3 + 3];

                            // position
                            combinedVertexBuffer.push_back(x0);
                            combinedVertexBuffer.push_back(y0);
                            combinedVertexBuffer.push_back(z0);
                            combinedVertexBuffer.push_back(w0);
                        }
                        else
                        {
                            f32 x0 = joints1AttributeBuffer[i0 * 3];
                            f32 y0 = joints1AttributeBuffer[i0 * 3 + 1];
                            f32 z0 = joints1AttributeBuffer[i0 * 3 + 2];
                            f32 w0 = joints1AttributeBuffer[i0 * 3 + 3];

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
                            f32 x0 = weights0AttributeBuffer[i0 * 3];
                            f32 y0 = weights0AttributeBuffer[i0 * 3 + 1];
                            f32 z0 = weights0AttributeBuffer[i0 * 3 + 2];
                            f32 w0 = weights0AttributeBuffer[i0 * 3 + 3];

                            // position
                            combinedVertexBuffer.push_back(x0);
                            combinedVertexBuffer.push_back(y0);
                            combinedVertexBuffer.push_back(z0);
                            combinedVertexBuffer.push_back(w0);
                        }
                        else
                        {
                            f32 x0 = weights1AttributeBuffer[i0 * 3];
                            f32 y0 = weights1AttributeBuffer[i0 * 3 + 1];
                            f32 z0 = weights1AttributeBuffer[i0 * 3 + 2];
                            f32 w0 = weights1AttributeBuffer[i0 * 3 + 3];

                            // position
                            combinedVertexBuffer.push_back(x0);
                            combinedVertexBuffer.push_back(y0);
                            combinedVertexBuffer.push_back(z0);
                            combinedVertexBuffer.push_back(w0);
                        }
                    }
                    else if (strcmp(semantic.c_str(), "Normal") == 0)
                    {
                        if (calculateNormals)
                        {
                            combinedVertexBuffer.push_back(0.0f);  // we will calculate
                            combinedVertexBuffer.push_back(0.0f);  // we will calculate
                            combinedVertexBuffer.push_back(0.0f);  // we will calculate
                        }
                        else
                        {
                            f32 nx0 = normalAttributeBuffer[i0 * 3];
                            f32 ny0 = normalAttributeBuffer[i0 * 3 + 1];
                            f32 nz0 = normalAttributeBuffer[i0 * 3 + 2];

                            combinedVertexBuffer.push_back(nx0);
                            combinedVertexBuffer.push_back(ny0);
                            combinedVertexBuffer.push_back(nz0);
                        }
                    }
                    else if (strcmp(semantic.c_str(), "Tangent") == 0)
                    {
                        if (calculateTangents)
                        {
                            combinedVertexBuffer.push_back(0.0f);  // we will calculate
                            combinedVertexBuffer.push_back(0.0f);  // we will calculate
                            combinedVertexBuffer.push_back(0.0f);  // we will calculate
                            combinedVertexBuffer.push_back(0.0f);  // we will calculate
                        }
                        else
                        {
                            f32 tx0 = tangentAttributeBuffer[i0 * 3];
                            f32 ty0 = tangentAttributeBuffer[i0 * 3 + 1];
                            f32 tz0 = tangentAttributeBuffer[i0 * 3 + 2];
                            //f32 tw0 = round(tangentAttributeBuffer[i0 * 3 + 3]);
                            f32 tw0 = tangentAttributeBuffer[i0 * 3 + 3];
                            //f32 tw1 = tw0 == 0.0 ? 1.0 : tw0;

                            combinedVertexBuffer.push_back(tx0);
                            combinedVertexBuffer.push_back(ty0);
                            combinedVertexBuffer.push_back(tz0);
                            combinedVertexBuffer.push_back(tw0);
                        }
                    }
                    else if (strcmp(semantic.c_str(), "Tex0Coord") == 0)
                    {
                        f32 u0 = texture0AttributeBuffer[i0 * 2];
                        f32 v0 = texture0AttributeBuffer[i0 * 2 + 1];
                        combinedVertexBuffer.push_back(u0);
                        combinedVertexBuffer.push_back(v0);
                    }
                    else if (strcmp(semantic.c_str(), "Tex1Coord") == 0)
                    {
                        f32 u1 = texture1AttributeBuffer[i0 * 2];
                        f32 v1 = texture1AttributeBuffer[i0 * 2 + 1];
                        combinedVertexBuffer.push_back(u1);
                        combinedVertexBuffer.push_back(v1);
                    }
                    else if (strcmp(semantic.c_str(), "Color") == 0)
                    {

                        f32 r0 = colorAttributeBuffer[i0 * 3];
                        f32 g0 = colorAttributeBuffer[i0 * 3 + 1];
                        f32 b0 = colorAttributeBuffer[i0 * 3 + 2];
                        
                        combinedVertexBuffer.push_back(r0);  // we will calculate
                        combinedVertexBuffer.push_back(g0);  // we will calculate
                        combinedVertexBuffer.push_back(b0);  // we will calculate

                        if (modifiedLayout.formats[j] == DXGI_FORMAT_R32G32B32A32_FLOAT)
                        {
                            f32 a0 = colorAttributeBuffer[i0 * 3 + 3];
                            combinedVertexBuffer.push_back(a0);  // we will calculate
                        }
                    }
                }

                indexBuffer.push_back(indexBuffer.size());
            }

            // calculate normals, tangents
            for (size_t i = 0; i < indexBuffer.size() - 2; i += 3)
            {

                size_t indices[3];
                indices[0] = indexBuffer[i];
                indices[1] = indexBuffer[i+1];
                indices[2] = indexBuffer[i+2];

                mvVec3 p[3];
                mvVec3 n[3];
                mvVec4 tan[3];
                mvVec2 tex0[3];
                mvVec2 tex1[3];
                mvVec4 color[3];
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
                    else if (strcmp(semantic.c_str(), "Tex0Coord") == 0)
                    {

                        for (size_t k = 0; k < 3; k++)
                        {
                            tex0[k].x = combinedVertexBuffer[indices[k] * modifiedLayout.elementCount + currentLocation];
                            tex0[k].y = combinedVertexBuffer[indices[k] * modifiedLayout.elementCount + currentLocation + 1];
                        }
                        currentLocation += 2u;

                    }
                    else if (strcmp(semantic.c_str(), "Tex1Coord") == 0)
                    {

                        for (size_t k = 0; k < 3; k++)
                        {
                            tex1[k].x = combinedVertexBuffer[indices[k] * modifiedLayout.elementCount + currentLocation];
                            tex1[k].y = combinedVertexBuffer[indices[k] * modifiedLayout.elementCount + currentLocation + 1];
                        }
                        currentLocation += 2u;

                    }
                    else if (strcmp(semantic.c_str(), "Color") == 0)
                    {

                        for (size_t k = 0; k < 3; k++)
                        {
                            color[k].r = combinedVertexBuffer[indices[k] * modifiedLayout.elementCount + currentLocation];
                            color[k].g = combinedVertexBuffer[indices[k] * modifiedLayout.elementCount + currentLocation + 1];
                            color[k].b = combinedVertexBuffer[indices[k] * modifiedLayout.elementCount + currentLocation + 2];

                            if (modifiedLayout.formats[j] == DXGI_FORMAT_R32G32B32A32_FLOAT)
                            {
                                color[k].a = combinedVertexBuffer[indices[k] * modifiedLayout.elementCount + currentLocation + 3];
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
                    ((edge1.x * uv2.y) - (edge2.x * uv1.y))*dirCorrection,
                    ((edge1.y * uv2.y) - (edge2.y * uv1.y))*dirCorrection,
                    ((edge1.z * uv2.y) - (edge2.z * uv1.y))*dirCorrection,
                    dirCorrection
                };

                if (calculateNormals)
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
                    if (calculateTangents)
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
                        else if (strcmp(semantic.c_str(), "Tex0Coord") == 0)
                        {
                            vertexBuffer.push_back(tex0[k].x);
                            vertexBuffer.push_back(tex0[k].y);
                        }
                        else if (strcmp(semantic.c_str(), "Tex1Coord") == 0)
                        {
                            vertexBuffer.push_back(tex1[k].x);
                            vertexBuffer.push_back(tex1[k].y);
                        }
                        else if (strcmp(semantic.c_str(), "Color") == 0)
                        {
                            vertexBuffer.push_back(color[k].x);
                            vertexBuffer.push_back(color[k].y);
                            vertexBuffer.push_back(color[k].z);

                            if (modifiedLayout.formats[j] == DXGI_FORMAT_R32G32B32A32_FLOAT)
                            {
                                vertexBuffer.push_back(color[k].w);
                            }
                        }
                    }
                }

            }

            newMesh.primitives.push_back({});
            newMesh.primitives.back().layout = modifiedLayout;

            // upload index buffer

            if (glprimitive.material_index != -1)
            {

                mvGLTFMaterial& material = model.materials[glprimitive.material_index];

                mvMaterial materialData{};
                materialData.extramacros.push_back({ "HAS_NORMALS", "0" });
                materialData.extramacros.push_back({ "HAS_TANGENTS", "0" });
                if(hasTexCoord0) materialData.extramacros.push_back({ "HAS_TEXCOORD_0_VEC2", "0" });
                if(hasTexCoord1) materialData.extramacros.push_back({ "HAS_TEXCOORD_1_VEC2", "0" });
                if(hasColorVec3) materialData.extramacros.push_back({ "HAS_VERTEX_COLOR_VEC3", "0" });
                if(hasColorVec4) materialData.extramacros.push_back({ "HAS_VERTEX_COLOR_VEC4", "0" });
                if(hasJoints0) materialData.extramacros.push_back({ "HAS_JOINTS_0_VEC4", "0" });
                if(hasJoints1) materialData.extramacros.push_back({ "HAS_JOINTS_1_VEC4", "0" });
                if(hasWeights0) materialData.extramacros.push_back({ "HAS_WEIGHTS_0_VEC4", "0" });
                if(hasWeights1) materialData.extramacros.push_back({ "HAS_WEIGHTS_1_VEC4", "0" });
                if(useSkinning) materialData.extramacros.push_back({ "USE_SKINNING", "0" });
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
                materialData.layout = modifiedLayout;
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
               
                std::string hash = hash_material(materialData, modifiedLayout, std::string("PBR_PS.hlsl"), std::string("PBR_VS.hlsl"));

                newMesh.primitives.back().materialID = mvGetMaterialAssetID(&assetManager, hash);
                if (newMesh.primitives.back().materialID == -1)
                {
                    newMesh.primitives.back().materialID = register_asset(&assetManager, hash, create_material(assetManager, "PBR_VS.hlsl", "PBR_PS.hlsl", materialData));
                }

            }
            else
            {


                mvMaterial materialData{};
                materialData.extramacros.push_back({ "HAS_NORMALS", "0" });
                materialData.extramacros.push_back({ "HAS_TANGENTS", "0" });
                if (hasTexCoord0) materialData.extramacros.push_back({ "HAS_TEXCOORD_0_VEC2", "0" });
                if (hasTexCoord1) materialData.extramacros.push_back({ "HAS_TEXCOORD_1_VEC2", "0" });
                if (hasColorVec3) materialData.extramacros.push_back({ "HAS_VERTEX_COLOR_VEC3", "0" });
                if (hasColorVec4) materialData.extramacros.push_back({ "HAS_VERTEX_COLOR_VEC4", "0" });
                if (hasJoints0) materialData.extramacros.push_back({ "HAS_JOINTS_0_VEC4", "0" });
                if (hasJoints1) materialData.extramacros.push_back({ "HAS_JOINTS_1_VEC4", "0" });
                if (hasWeights0) materialData.extramacros.push_back({ "HAS_WEIGHTS_0_VEC4", "0" });
                if (hasWeights1) materialData.extramacros.push_back({ "HAS_WEIGHTS_1_VEC4", "0" });
                if (useSkinning) materialData.extramacros.push_back({ "USE_SKINNING", "0" });
                materialData.data.albedo = { 0.45f, 0.45f, 0.85f, 1.0f };
                materialData.data.metalness = 0.0f;
                materialData.data.roughness = 0.5f;
                materialData.data.alphaCutoff = 0.5f;
                materialData.data.doubleSided = false;
                materialData.layout = modifiedLayout;
                std::string hash = hash_material(materialData, modifiedLayout, std::string("PBR_PS.hlsl"), std::string("PBR_VS.hlsl"));

                newMesh.primitives.back().materialID = mvGetMaterialAssetID(&assetManager, hash);
                if (newMesh.primitives.back().materialID == -1)
                {
                    newMesh.primitives.back().materialID = register_asset(&assetManager, hash, create_material(assetManager, "PBR_VS.hlsl", "PBR_PS.hlsl", materialData));
                }
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

    for (u32 currentNode = 0u; currentNode < model.node_count; currentNode++)
    {
        mvGLTFNode& glnode = model.nodes[currentNode];

        mvNode newNode{};
        newNode.name = glnode.name;
        if (glnode.mesh_index > -1)
            newNode.mesh = meshMapping[glnode.mesh_index];
        if (glnode.skin_index > -1)
            newNode.skin = skinMapping[glnode.skin_index];
        if (glnode.camera_index > -1)
        {
            
            newNode.camera = cameraMapping[glnode.camera_index];
            mvCamera& camera = assetManager.cameras[newNode.camera].asset;

            if (camera.type == MV_CAMERA_PERSPECTIVE)
            {
                mvMesh frustum1 = create_frustum2(assetManager, camera.fieldOfView*180.0f/M_PI, camera.aspectRatio, camera.nearZ, camera.farZ);
                newNode.mesh = register_asset(&assetManager, "node_camera_" + std::to_string(currentNode), frustum1);
            }
            else
            {
                mvMesh frustum1 = create_ortho_frustum(assetManager, camera.width, camera.height, camera.nearZ, camera.farZ);
                newNode.mesh = register_asset(&assetManager, "bad_node_camera_" + std::to_string(currentNode), frustum1);
            }
        }
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

        nodeMapping[currentNode] = register_asset(&assetManager, "node_" + std::to_string(currentNode), newNode);
    }

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
            channel.node = nodeMapping[glchannel.target.node];
            assetManager.nodes[channel.node].asset.animated = true;
            channel.path = glchannel.target.path;
            channel.interpolation = glsampler.interpolation;
            
            mvGLTFAccessor& inputaccessor = model.accessors[glsampler.input];
            mvGLTFAccessor& outputaccessor = model.accessors[glsampler.output];
            mvFillBuffer<f32>(model, inputaccessor, channel.inputdata, 1);
            //mvFillBuffer<u32>(model, outputaccessor, channel.outputdata, 4);

            mvGLTFComponentType indexCompType = outputaccessor.component_type;

            if (channel.path == "translation" || channel.path == "scale")
            {
                mvFillBuffer<u32>(model, outputaccessor, channel.outputdata, 3);
            }

            if (channel.path == "rotation")
            {
                switch (indexCompType)
                {
                case MV_IMP_BYTE:
                case MV_IMP_UNSIGNED_BYTE:
                    mvFillBuffer<u8>(model, outputaccessor, channel.outputdata, 4);
                    break;

                case MV_IMP_SHORT:
                case MV_IMP_UNSIGNED_SHORT:
                    mvFillBuffer<u16>(model, outputaccessor, channel.outputdata, 4);
                    break;

                case MV_IMP_FLOAT:
                    mvFillBuffer<u32>(model, outputaccessor, channel.outputdata, 4);
                    break;

                default:
                    assert(false && "Unknown index compenent type");
                }
            }

            if (channel.path == "weights")
            {
                switch (indexCompType)
                {
                case MV_IMP_BYTE:
                case MV_IMP_UNSIGNED_BYTE:
                    mvFillBuffer<u8>(model, outputaccessor, channel.outputdata, 1);
                    break;

                case MV_IMP_SHORT:
                case MV_IMP_UNSIGNED_SHORT:
                    mvFillBuffer<u16>(model, outputaccessor, channel.outputdata, 1);
                    break;

                case MV_IMP_FLOAT:
                    mvFillBuffer<u32>(model, outputaccessor, channel.outputdata, 1);
                    break;

                default:
                    assert(false && "Unknown index compenent type");
                }
            }
        }

        register_asset(&assetManager, model.name + "_animation_" + std::to_string(currentAnimation), animation);
    }

    // update children for actual offset
    for (u32 currentNode = 0u; currentNode < model.node_count; currentNode++)
    {

        mvNode& node = assetManager.nodes[nodeMapping[currentNode]].asset;
        for (i32 i = 0; i < node.childCount; i++)
            node.children[i] = nodeMapping[node.children[i]];
    }

    // update joints for actual offset
    for (u32 currentSkin = 0u; currentSkin < model.skin_count; currentSkin++)
    {

        mvSkin& skin = assetManager.skins[skinMapping[currentSkin]].asset;
        for (i32 i = 0; i < skin.jointCount; i++)
            skin.joints[i] = nodeMapping[skin.joints[i]];
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