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
            mvVertexElement::Texture2D,
            mvVertexElement::Tangent,
            mvVertexElement::Bitangent
        }
    );

    const float side = size;
    auto vertices = std::vector<f32>{
        -side, -side, -side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,  // 0 near side
         side, -side, -side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,  // 1
        -side,  side, -side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 2
         side,  side, -side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,  // 3

        -side, -side,  side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,  // 4 far side
         side, -side,  side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,  // 5
        -side,  side,  side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,  // 6
         side,  side,  side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 7

        -side, -side, -side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,  // 8 left side
        -side,  side, -side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,  // 9
        -side, -side,  side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,  // 10
        -side,  side,  side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 11

         side, -side, -side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,  // 12 right side
         side,  side, -side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 13
         side, -side,  side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,  // 14
         side,  side,  side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,  // 15

        -side, -side, -side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 16 bottom side
         side, -side, -side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,  // 17
        -side, -side,  side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,  // 18
         side, -side,  side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,  // 19

        -side,  side, -side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,  // 20 top side
         side,  side, -side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,  // 21
        -side,  side,  side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 22
         side,  side,  side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f   // 23
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

        mvVec3 p0 = { vertices[14 * indices[i]], vertices[14 * indices[i] + 1], vertices[14 * indices[i] + 2] };
        mvVec3 p1 = { vertices[14 * indices[i+1]], vertices[14 * indices[i + 1] + 1], vertices[14 * indices[i + 1] + 2] };
        mvVec3 p2 = { vertices[14 * indices[i+2]], vertices[14 * indices[i + 2] + 1], vertices[14 * indices[i + 2] + 2] };

        mvVec3 n = normalize(cross(p1 - p0, p2 - p0));
        vertices[14 * indices[i] + 3] = n[0];
        vertices[14 * indices[i] + 4] = n[1];
        vertices[14 * indices[i] + 5] = n[2];
        vertices[14 * indices[i + 1] + 3] = n[0];
        vertices[14 * indices[i + 1] + 4] = n[1];
        vertices[14 * indices[i + 1] + 5] = n[2];
        vertices[14 * indices[i + 2] + 3] = n[0];
        vertices[14 * indices[i + 2] + 4] = n[1];
        vertices[14 * indices[i + 2] + 5] = n[2];
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
            mvVertexElement::Texture2D,
            mvVertexElement::Tangent,
            mvVertexElement::Bitangent
        }
    );

    const float side = size;
    auto vertices = std::vector<f32>{
        -side,  side, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
         side, -side, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        -side, -side, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
         side,  side, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
    };

    static auto indices = std::vector<u32>{
        1, 0, 2,
        3, 0, 1
    };

    for (size_t i = 0; i < indices.size(); i += 3)
    {
        mvVec3 p0 = { vertices[14 * indices[i]], vertices[14 * indices[i] + 1], vertices[14 * indices[i] + 2] };
        mvVec3 p1 = { vertices[14 * indices[i + 1]], vertices[14 * indices[i + 1] + 1], vertices[14 * indices[i + 1] + 2] };
        mvVec3 p2 = { vertices[14 * indices[i + 2]], vertices[14 * indices[i + 2] + 1], vertices[14 * indices[i + 2] + 2] };

        mvVec3 n = normalize(cross(p1 - p0, p2 - p0));
        vertices[14 * indices[i] + 3] = n[0];
        vertices[14 * indices[i] + 4] = n[1];
        vertices[14 * indices[i] + 5] = n[2];
        vertices[14 * indices[i + 1] + 3] = n[0];
        vertices[14 * indices[i + 1] + 4] = n[1];
        vertices[14 * indices[i + 1] + 5] = n[2];
        vertices[14 * indices[i + 2] + 3] = n[0];
        vertices[14 * indices[i + 2] + 4] = n[1];
        vertices[14 * indices[i + 2] + 5] = n[2];
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

static u8
mvGetAccessorItemCompCount(mvGLTFAccessor& accessor)
{
    switch (accessor.type)
    {
    case(MV_IMP_SCALAR): return 1u;
    case(MV_IMP_VEC2): return 2u;
    case(MV_IMP_VEC3): return 3u;
    case(MV_IMP_VEC4): return 4u;
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
    return D3D11_FILTER_MIN_MAG_MIP_POINT;
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

mvAssetID
load_gltf_assets(mvAssetManager& assetManager, mvGLTFModel& model)
{

    std::vector<mvAssetID> cameraMapping;
    cameraMapping.resize(model.camera_count);

    std::vector<mvAssetID> nodeMapping;
    nodeMapping.resize(model.node_count);

    std::vector<mvAssetID> meshMapping;
    meshMapping.resize(model.mesh_count);

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
            std::vector<f32> bitangentAttributeBuffer;
            std::vector<f32> normalAttributeBuffer;
            std::vector<f32> textureAttributeBuffer;
            std::vector<f32> colorAttributeBuffer;

            std::vector<mvVertexElement> attributes;
            for (u32 i = 0; i < glprimitive.attribute_count; i++)
            {
                auto& attribute = glprimitive.attributes[i];
                switch (attribute.type)
                {
                case(MV_IMP_POSITION):
                    attributes.push_back(mvVertexElement::Position3D);
                    mvFillBuffer<f32>(model, model.accessors[attribute.index], positionAttributeBuffer);
                    break;
                case(MV_IMP_TANGENT):
                    attributes.push_back(mvVertexElement::Tangent);
                    mvFillBuffer<f32>(model, model.accessors[attribute.index], tangentAttributeBuffer, 3);
                    break;
                case(MV_IMP_NORMAL):
                    attributes.push_back(mvVertexElement::Normal);
                    mvFillBuffer<f32>(model, model.accessors[attribute.index], normalAttributeBuffer);
                    break;
                case(MV_IMP_TEXTCOORD):
                    attributes.push_back(mvVertexElement::Texture2D);
                    mvFillBuffer<f32>(model, model.accessors[attribute.index], textureAttributeBuffer, 2);
                    break;
                case(MV_IMP_COLOR_0):
                    attributes.push_back(mvVertexElement::Color);
                    mvFillBuffer<f32>(model, model.accessors[attribute.index], colorAttributeBuffer, 4);
                    break;

                case(MV_IMP_WEIGHTS_0):
                case(MV_IMP_JOINTS_0):
                default:
                    assert(false && "Undefined attribute type");
                    break;
                }
            }

            mvVertexLayout currentlayout = create_vertex_layout(attributes);

            std::vector<u32> indexBuffer;
            std::vector<f32> vertexBuffer;

            u32 triangleCount = origIndexBuffer.size() / 3;

            vertexBuffer.reserve(triangleCount * 16 * 3);
            indexBuffer.reserve(triangleCount * 3);
            if (normalAttributeBuffer.empty()) normalAttributeBuffer.resize(triangleCount * 3 * 3);
            if (textureAttributeBuffer.empty()) textureAttributeBuffer.resize(triangleCount * 3 * 2);

            std::vector<f32> combinedVertexBuffer;

            for (size_t i = 0; i < origIndexBuffer.size(); i++)
            {
                size_t i0 = origIndexBuffer[i];

                f32 x0 = positionAttributeBuffer[i0 * 3];
                f32 y0 = positionAttributeBuffer[i0 * 3 + 1];
                f32 z0 = positionAttributeBuffer[i0 * 3 + 2];

                f32 nx0 = normalAttributeBuffer[i0 * 3];
                f32 ny0 = normalAttributeBuffer[i0 * 3 + 1];
                f32 nz0 = normalAttributeBuffer[i0 * 3 + 2];

                f32 u0 = textureAttributeBuffer[i0 * 2];
                f32 v0 = textureAttributeBuffer[i0 * 2 + 1];

                combinedVertexBuffer.push_back(x0);
                combinedVertexBuffer.push_back(y0);
                combinedVertexBuffer.push_back(z0);
                combinedVertexBuffer.push_back(nx0);  // we will calculate
                combinedVertexBuffer.push_back(ny0);  // we will calculate
                combinedVertexBuffer.push_back(nz0);  // we will calculate
                combinedVertexBuffer.push_back(u0);
                combinedVertexBuffer.push_back(v0);
                combinedVertexBuffer.push_back(0.0f);  // we will calculate
                combinedVertexBuffer.push_back(0.0f);  // we will calculate
                combinedVertexBuffer.push_back(0.0f);  // we will calculate
                combinedVertexBuffer.push_back(0.0f);  // we will calculate
                combinedVertexBuffer.push_back(0.0f);  // we will calculate
                combinedVertexBuffer.push_back(0.0f);  // we will calculate
                combinedVertexBuffer.push_back(0.0f);  // we will calculate
                combinedVertexBuffer.push_back(0.0f);  // we will calculate

                indexBuffer.push_back(indexBuffer.size());
            }

            // calculate normals, tangents, bitangents
            for (size_t i = 0; i < indexBuffer.size() - 2; i += 3)
            {

                size_t i0 = indexBuffer[i];
                size_t i1 = indexBuffer[i + 1];
                size_t i2 = indexBuffer[i + 2];

                mvVec3 p0 = *((mvVec3*)&combinedVertexBuffer[i0 * 16]);
                mvVec3 p1 = *((mvVec3*)&combinedVertexBuffer[i1 * 16]);
                mvVec3 p2 = *((mvVec3*)&combinedVertexBuffer[i2 * 16]);

                mvVec3 n0 = *((mvVec3*)&combinedVertexBuffer[i0 * 16 + 3]);
                mvVec3 n1 = *((mvVec3*)&combinedVertexBuffer[i1 * 16 + 3]);
                mvVec3 n2 = *((mvVec3*)&combinedVertexBuffer[i2 * 16 + 3]);

                mvVec2 tex0 = *((mvVec2*)&combinedVertexBuffer[i0 * 16 + 6]);
                mvVec2 tex1 = *((mvVec2*)&combinedVertexBuffer[i1 * 16 + 6]);
                mvVec2 tex2 = *((mvVec2*)&combinedVertexBuffer[i2 * 16 + 6]);

                // calculate normals
                //mvVec3 n = cNormalize(cCross(p1 - p0, p2 - p0));
                //*((mvVec3*)&combinedVertexBuffer[i0 * 14 + 3]) = n;
                //*((mvVec3*)&combinedVertexBuffer[i1 * 14 + 3]) = n;
                //*((mvVec3*)&combinedVertexBuffer[i2 * 14 + 3]) = n;

                // calculate tangents
                mvVec3 edge1 = p1 - p0;
                mvVec3 edge2 = p2 - p0;

                mvVec2 uv1 = tex1 - tex0;
                mvVec2 uv2 = tex2 - tex0;

                float dirCorrection = (uv1.x * uv2.y - uv1.y * uv2.x) < 0.0f ? -1.0f : 1.0f;

                if (uv1.x * uv2.y == uv1.y * uv2.x)
                {
                    uv1.x = 0.0f;
                    uv1.y = 1.0f;
                    uv2.x = 1.0f;
                    uv2.y = 0.0f;
                }

                mvVec4 tangent = {
                    ((edge1.x * uv2.y) - (edge2.x * uv1.y))*dirCorrection,
                    ((edge1.y * uv2.y) - (edge2.y * uv1.y))*dirCorrection,
                    ((edge1.z * uv2.y) - (edge2.z * uv1.y))*dirCorrection,
                    dirCorrection
                };

                mvVec4 bitangent = {
                    ((edge1.x * uv2.x) - (edge2.x * uv1.x))*dirCorrection,
                    ((edge1.y * uv2.x) - (edge2.y * uv1.x))*dirCorrection,
                    ((edge1.z * uv2.x) - (edge2.z * uv1.x))*dirCorrection,
                    dirCorrection
                };

                // project tangent and bitangent into the plane formed by the vertex' normal
                //mvVec3 newTangent = cNormalize(tangent - n * (tangent * n));
                *((mvVec3*)&combinedVertexBuffer[i0 * 16 + 8]) = normalize(tangent.xyz() - n0 * (tangent.xyz() * n0));
                *((mvVec3*)&combinedVertexBuffer[i1 * 16 + 8]) = normalize(tangent.xyz() - n1 * (tangent.xyz() * n1));
                *((mvVec3*)&combinedVertexBuffer[i2 * 16 + 8]) = normalize(tangent.xyz() - n2 * (tangent.xyz() * n2));

                //mvVec3 newBitangent = cNormalize(bitangent - n * (bitangent * n));
                *((mvVec3*)&combinedVertexBuffer[i0 * 16 + 12]) = normalize(bitangent.xyz() - n0 * (bitangent.xyz() * n0));
                *((mvVec3*)&combinedVertexBuffer[i1 * 16 + 12]) = normalize(bitangent.xyz() - n1 * (bitangent.xyz() * n1));
                *((mvVec3*)&combinedVertexBuffer[i2 * 16 + 12]) = normalize(bitangent.xyz() - n2 * (bitangent.xyz() * n2));

                // vertex 0
                vertexBuffer.push_back(p0.x);
                vertexBuffer.push_back(p0.y);
                vertexBuffer.push_back(p0.z);
                vertexBuffer.push_back(n0.x);
                vertexBuffer.push_back(n0.y);
                vertexBuffer.push_back(n0.z);
                vertexBuffer.push_back(tex0.x);
                vertexBuffer.push_back(tex0.y);
                vertexBuffer.push_back(combinedVertexBuffer[i0 * 16 + 8]);
                vertexBuffer.push_back(combinedVertexBuffer[i0 * 16 + 9]);
                vertexBuffer.push_back(combinedVertexBuffer[i0 * 16 + 10]);
                vertexBuffer.push_back(dirCorrection);
                vertexBuffer.push_back(combinedVertexBuffer[i0 * 16 + 12]);
                vertexBuffer.push_back(combinedVertexBuffer[i0 * 16 + 13]);
                vertexBuffer.push_back(combinedVertexBuffer[i0 * 16 + 14]);
                vertexBuffer.push_back(dirCorrection);

                // vertex 1
                vertexBuffer.push_back(p1.x);
                vertexBuffer.push_back(p1.y);
                vertexBuffer.push_back(p1.z);
                vertexBuffer.push_back(n1.x);
                vertexBuffer.push_back(n1.y);
                vertexBuffer.push_back(n1.z);
                vertexBuffer.push_back(tex1.x);
                vertexBuffer.push_back(tex1.y);
                vertexBuffer.push_back(combinedVertexBuffer[i1 * 16 + 8]);
                vertexBuffer.push_back(combinedVertexBuffer[i1 * 16 + 9]);
                vertexBuffer.push_back(combinedVertexBuffer[i1 * 16 + 10]);
                vertexBuffer.push_back(dirCorrection);
                vertexBuffer.push_back(combinedVertexBuffer[i1 * 16 + 12]);
                vertexBuffer.push_back(combinedVertexBuffer[i1 * 16 + 13]);
                vertexBuffer.push_back(combinedVertexBuffer[i1 * 16 + 14]);
                vertexBuffer.push_back(dirCorrection);

                // vertex 2
                vertexBuffer.push_back(p2.x);
                vertexBuffer.push_back(p2.y);
                vertexBuffer.push_back(p2.z);
                vertexBuffer.push_back(n2.x);
                vertexBuffer.push_back(n2.y);
                vertexBuffer.push_back(n2.z);
                vertexBuffer.push_back(tex2.x);
                vertexBuffer.push_back(tex2.y);
                vertexBuffer.push_back(combinedVertexBuffer[i2 * 16 + 8]);
                vertexBuffer.push_back(combinedVertexBuffer[i2 * 16 + 9]);
                vertexBuffer.push_back(combinedVertexBuffer[i2 * 16 + 10]);
                vertexBuffer.push_back(dirCorrection);
                vertexBuffer.push_back(combinedVertexBuffer[i2 * 16 + 12]);
                vertexBuffer.push_back(combinedVertexBuffer[i2 * 16 + 13]);
                vertexBuffer.push_back(combinedVertexBuffer[i2 * 16 + 14]);
                vertexBuffer.push_back(dirCorrection);

            }

            newMesh.primitives.push_back({});
            newMesh.primitives.back().layout = create_vertex_layout(
                {
                    mvVertexElement::Position3D,
                    mvVertexElement::Normal,
                    mvVertexElement::Texture2D,
                    mvVertexElement::Tangent,
                    mvVertexElement::Bitangent
                }
            );

            // upload index buffer

            if (glprimitive.material_index != -1)
            {

                mvGLTFMaterial& material = model.materials[glprimitive.material_index];

                mvMaterial materialData{};
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
                    


                if (material.base_color_texture != -1)
                {
                    mvGLTFTexture& texture = model.textures[material.base_color_texture];
                    std::string uri = model.images[texture.image_index].uri;
                    if (model.images[texture.image_index].embedded)
                    {
                        newMesh.primitives.back().albedoTexture = mvGetTextureAssetID(&assetManager, model.root + newMesh.name + std::to_string(currentPrimitive) + uri + "_a", model.images[texture.image_index].data);
                    }
                    else
                        newMesh.primitives.back().albedoTexture = mvGetTextureAssetID(&assetManager, model.root + newMesh.name + std::to_string(currentPrimitive) + uri + "_a", model.root + uri);
                    materialData.hasAlbedoMap = true;
                    if (texture.sampler_index > -1)
                    {
                        mvGLTFSampler& sampler = model.samplers[texture.sampler_index];
                        mvTexture& newTexture = assetManager.textures[newMesh.primitives.back().albedoTexture].asset;

                        // Create Sampler State
                        D3D11_SAMPLER_DESC samplerDesc{};
                        samplerDesc.AddressU = get_address_mode(sampler.wrap_s);
                        samplerDesc.AddressV = get_address_mode(sampler.wrap_t);
                        samplerDesc.AddressW = samplerDesc.AddressV;
                        samplerDesc.Filter = get_filter_mode(sampler.min_filter, sampler.mag_filter);
                        samplerDesc.BorderColor[0] = 0.0f;
                        samplerDesc.MaxAnisotropy = D3D11_REQ_MAXANISOTROPY;

                        HRESULT hResult = GContext->graphics.device->CreateSamplerState(&samplerDesc, &newTexture.sampler);
                        assert(SUCCEEDED(hResult));
                    }
                }

                if (material.normal_texture != -1)
                {
                    mvGLTFTexture& texture = model.textures[material.normal_texture];
                    std::string uri = model.images[texture.image_index].uri;
                    if (model.images[texture.image_index].embedded)
                    {
                        newMesh.primitives.back().normalTexture = mvGetTextureAssetID(&assetManager, model.root + newMesh.name + std::to_string(currentPrimitive) + uri + "_n", model.images[texture.image_index].data);
                    }
                    else
                        newMesh.primitives.back().normalTexture = mvGetTextureAssetID(&assetManager, model.root + newMesh.name + std::to_string(currentPrimitive) + uri + "_n", model.root + uri);
                    materialData.hasNormalMap = true;
                    if (texture.sampler_index > -1)
                    {
                        mvGLTFSampler& sampler = model.samplers[texture.sampler_index];
                        mvTexture& newTexture = assetManager.textures[newMesh.primitives.back().normalTexture].asset;

                        // Create Sampler State
                        D3D11_SAMPLER_DESC samplerDesc{};
                        samplerDesc.AddressU = get_address_mode(sampler.wrap_s);
                        samplerDesc.AddressV = get_address_mode(sampler.wrap_t);
                        samplerDesc.AddressW = samplerDesc.AddressU;
                        samplerDesc.Filter = get_filter_mode(sampler.min_filter, sampler.mag_filter);
                        samplerDesc.BorderColor[0] = 0.0f;
                        samplerDesc.MaxAnisotropy = D3D11_REQ_MAXANISOTROPY;

                        HRESULT hResult = GContext->graphics.device->CreateSamplerState(&samplerDesc, &newTexture.sampler);
                        assert(SUCCEEDED(hResult));
                    }
                }

                if (material.metallic_roughness_texture != -1)
                {
                    mvGLTFTexture& texture = model.textures[material.metallic_roughness_texture];
                    std::string uri = model.images[texture.image_index].uri;
                    if (model.images[texture.image_index].embedded)
                    {
                        newMesh.primitives.back().metalRoughnessTexture = mvGetTextureAssetID(&assetManager, model.root + newMesh.name + std::to_string(currentPrimitive) + uri + "_m", model.images[texture.image_index].data);
                    }
                    else
                        newMesh.primitives.back().metalRoughnessTexture = mvGetTextureAssetID(&assetManager, model.root + newMesh.name + std::to_string(currentPrimitive) + uri + "_m", model.root + uri);
                    materialData.hasMetallicRoughnessMap = true;
                    if (texture.sampler_index > -1)
                    {
                        mvGLTFSampler& sampler = model.samplers[texture.sampler_index];
                        mvTexture& newTexture = assetManager.textures[newMesh.primitives.back().metalRoughnessTexture].asset;

                        // Create Sampler State
                        D3D11_SAMPLER_DESC samplerDesc{};
                        samplerDesc.AddressU = get_address_mode(sampler.wrap_s);
                        samplerDesc.AddressV = get_address_mode(sampler.wrap_t);
                        samplerDesc.AddressW = samplerDesc.AddressU;
                        samplerDesc.Filter = get_filter_mode(sampler.min_filter, sampler.mag_filter);
                        samplerDesc.BorderColor[0] = 0.0f;
                        samplerDesc.MaxAnisotropy = D3D11_REQ_MAXANISOTROPY;

                        HRESULT hResult = GContext->graphics.device->CreateSamplerState(&samplerDesc, &newTexture.sampler);
                        assert(SUCCEEDED(hResult));
                    }
                }

                if (material.emissive_texture != -1)
                {
                    mvGLTFTexture& texture = model.textures[material.emissive_texture];
                    std::string uri = model.images[texture.image_index].uri;
                    if (model.images[texture.image_index].embedded)
                    {
                        newMesh.primitives.back().emissiveTexture = mvGetTextureAssetID(&assetManager, model.root + newMesh.name + std::to_string(currentPrimitive) + uri + "_e", model.images[texture.image_index].data);
                    }
                    else
                        newMesh.primitives.back().emissiveTexture = mvGetTextureAssetID(&assetManager, model.root + newMesh.name + std::to_string(currentPrimitive) + uri + "_e", model.root + uri);
                    materialData.hasEmmissiveMap = true;
                    if (texture.sampler_index > -1)
                    {
                        mvGLTFSampler& sampler = model.samplers[texture.sampler_index];
                        mvTexture& newTexture = assetManager.textures[newMesh.primitives.back().emissiveTexture].asset;

                        // Create Sampler State
                        D3D11_SAMPLER_DESC samplerDesc{};
                        samplerDesc.AddressU = get_address_mode(sampler.wrap_s);
                        samplerDesc.AddressV = get_address_mode(sampler.wrap_t);
                        samplerDesc.AddressW = samplerDesc.AddressU;
                        samplerDesc.Filter = get_filter_mode(sampler.min_filter, sampler.mag_filter);
                        samplerDesc.BorderColor[0] = 0.0f;
                        samplerDesc.MaxAnisotropy = D3D11_REQ_MAXANISOTROPY;

                        HRESULT hResult = GContext->graphics.device->CreateSamplerState(&samplerDesc, &newTexture.sampler);
                        assert(SUCCEEDED(hResult));
                    }
                }

                if (material.occlusion_texture != -1)
                {
                    mvGLTFTexture& texture = model.textures[material.occlusion_texture];
                    std::string uri = model.images[texture.image_index].uri;
                    if (model.images[texture.image_index].embedded)
                    {
                        newMesh.primitives.back().occlusionTexture = mvGetTextureAssetID(&assetManager, model.root + newMesh.name + std::to_string(currentPrimitive) + uri + "_o", model.images[texture.image_index].data);
                    }
                    else
                        newMesh.primitives.back().occlusionTexture = mvGetTextureAssetID(&assetManager, model.root + newMesh.name + std::to_string(currentPrimitive) + uri + "_o", model.root + uri);
                    materialData.hasOcculusionMap = true;
                    if (texture.sampler_index > -1)
                    {
                        mvGLTFSampler& sampler = model.samplers[texture.sampler_index];
                        mvTexture& newTexture = assetManager.textures[newMesh.primitives.back().occlusionTexture].asset;

                        // Create Sampler State
                        D3D11_SAMPLER_DESC samplerDesc{};
                        samplerDesc.AddressU = get_address_mode(sampler.wrap_s);
                        samplerDesc.AddressV = get_address_mode(sampler.wrap_t);
                        samplerDesc.AddressW = samplerDesc.AddressU;
                        samplerDesc.Filter = get_filter_mode(sampler.min_filter, sampler.mag_filter);
                        samplerDesc.BorderColor[0] = 0.0f;
                        samplerDesc.MaxAnisotropy = D3D11_REQ_MAXANISOTROPY;

                        HRESULT hResult = GContext->graphics.device->CreateSamplerState(&samplerDesc, &newTexture.sampler);
                        assert(SUCCEEDED(hResult));
                    }
                }

                if (material.clearcoat_texture != -1)
                {
                    mvGLTFTexture& texture = model.textures[material.clearcoat_texture];
                    std::string uri = model.images[texture.image_index].uri;
                    if (model.images[texture.image_index].embedded)
                    {
                        newMesh.primitives.back().clearcoatTexture = mvGetTextureAssetID(&assetManager, model.root + newMesh.name + std::to_string(currentPrimitive) + uri + "_cc", model.images[texture.image_index].data);
                    }
                    else
                        newMesh.primitives.back().clearcoatTexture = mvGetTextureAssetID(&assetManager, model.root + newMesh.name + std::to_string(currentPrimitive) + uri + "_cc", model.root + uri);
                    materialData.hasClearcoatMap = true;
                    if (texture.sampler_index > -1)
                    {
                        mvGLTFSampler& sampler = model.samplers[texture.sampler_index];
                        mvTexture& newTexture = assetManager.textures[newMesh.primitives.back().clearcoatTexture].asset;

                        // Create Sampler State
                        D3D11_SAMPLER_DESC samplerDesc{};
                        samplerDesc.AddressU = get_address_mode(sampler.wrap_s);
                        samplerDesc.AddressV = get_address_mode(sampler.wrap_t);
                        samplerDesc.AddressW = samplerDesc.AddressU;
                        samplerDesc.Filter = get_filter_mode(sampler.min_filter, sampler.mag_filter);
                        samplerDesc.BorderColor[0] = 0.0f;
                        samplerDesc.MaxAnisotropy = D3D11_REQ_MAXANISOTROPY;

                        HRESULT hResult = GContext->graphics.device->CreateSamplerState(&samplerDesc, &newTexture.sampler);
                        assert(SUCCEEDED(hResult));
                    }
                }

                if (material.clearcoat_roughness_texture != -1)
                {
                    mvGLTFTexture& texture = model.textures[material.clearcoat_roughness_texture];
                    std::string uri = model.images[texture.image_index].uri;
                    if (model.images[texture.image_index].embedded)
                    {
                        newMesh.primitives.back().clearcoatRoughnessTexture = mvGetTextureAssetID(&assetManager, model.root + newMesh.name + std::to_string(currentPrimitive) + uri + "_ccr", model.images[texture.image_index].data);
                    }
                    else
                        newMesh.primitives.back().clearcoatRoughnessTexture = mvGetTextureAssetID(&assetManager, model.root + newMesh.name + std::to_string(currentPrimitive) + uri + "_ccr", model.root + uri);
                    materialData.hasClearcoatRoughnessMap = true;
                    if (texture.sampler_index > -1)
                    {
                        mvGLTFSampler& sampler = model.samplers[texture.sampler_index];
                        mvTexture& newTexture = assetManager.textures[newMesh.primitives.back().clearcoatRoughnessTexture].asset;

                        // Create Sampler State
                        D3D11_SAMPLER_DESC samplerDesc{};
                        samplerDesc.AddressU = get_address_mode(sampler.wrap_s);
                        samplerDesc.AddressV = get_address_mode(sampler.wrap_t);
                        samplerDesc.AddressW = samplerDesc.AddressU;
                        samplerDesc.Filter = get_filter_mode(sampler.min_filter, sampler.mag_filter);
                        samplerDesc.BorderColor[0] = 0.0f;
                        samplerDesc.MaxAnisotropy = D3D11_REQ_MAXANISOTROPY;

                        HRESULT hResult = GContext->graphics.device->CreateSamplerState(&samplerDesc, &newTexture.sampler);
                        assert(SUCCEEDED(hResult));
                    }
                }

                if (material.clearcoat_normal_texture != -1)
                {
                    mvGLTFTexture& texture = model.textures[material.clearcoat_normal_texture];
                    std::string uri = model.images[texture.image_index].uri;
                    if (model.images[texture.image_index].embedded)
                    {
                        newMesh.primitives.back().clearcoatNormalTexture = mvGetTextureAssetID(&assetManager, model.root + newMesh.name + std::to_string(currentPrimitive) + uri + "_ccn", model.images[texture.image_index].data);
                    }
                    else
                        newMesh.primitives.back().clearcoatNormalTexture = mvGetTextureAssetID(&assetManager, model.root + newMesh.name + std::to_string(currentPrimitive) + uri + "_ccn", model.root + uri);
                    materialData.hasClearcoatNormalMap = true;
                    if (texture.sampler_index > -1)
                    {
                        mvGLTFSampler& sampler = model.samplers[texture.sampler_index];
                        mvTexture& newTexture = assetManager.textures[newMesh.primitives.back().clearcoatNormalTexture].asset;

                        // Create Sampler State
                        D3D11_SAMPLER_DESC samplerDesc{};
                        samplerDesc.AddressU = get_address_mode(sampler.wrap_s);
                        samplerDesc.AddressV = get_address_mode(sampler.wrap_t);
                        samplerDesc.AddressW = samplerDesc.AddressU;
                        samplerDesc.Filter = get_filter_mode(sampler.min_filter, sampler.mag_filter);
                        samplerDesc.BorderColor[0] = 0.0f;
                        samplerDesc.MaxAnisotropy = D3D11_REQ_MAXANISOTROPY;

                        HRESULT hResult = GContext->graphics.device->CreateSamplerState(&samplerDesc, &newTexture.sampler);
                        assert(SUCCEEDED(hResult));
                    }
                }

                std::string hash = hash_material(materialData, std::string("PBR_PS.hlsl"), std::string("PBR_VS.hlsl"));

                newMesh.primitives.back().materialID = mvGetMaterialAssetID(&assetManager, hash);
                if (newMesh.primitives.back().materialID == -1)
                {
                    newMesh.primitives.back().materialID = register_asset(&assetManager, hash, create_material(assetManager, "PBR_VS.hlsl", "PBR_PS.hlsl", materialData));
                }

            }
            else
            {


                mvMaterial materialData{};
                materialData.data.albedo = { 0.45f, 0.45f, 0.85f, 1.0f };
                materialData.data.metalness = 0.0f;
                materialData.data.roughness = 0.5f;
                materialData.data.alphaCutoff = 0.5f;
                materialData.data.doubleSided = false;
                std::string hash = hash_material(materialData, std::string("PBR_PS.hlsl"), std::string("PBR_VS.hlsl"));

                newMesh.primitives.back().materialID = mvGetMaterialAssetID(&assetManager, hash);
                if (newMesh.primitives.back().materialID == -1)
                {
                    newMesh.primitives.back().materialID = register_asset(&assetManager, hash, create_material(assetManager, "PBR_VS.hlsl", "PBR_PS.hlsl", materialData));
                }
            }

            newMesh.primitives.back().indexBuffer = mvGetBufferAssetID(&assetManager,
                std::string(glmesh.name) + std::to_string(currentPrimitive) + "_indexbuffer",
                indexBuffer.data(),
                indexBuffer.size() * sizeof(u32),
                D3D11_BIND_INDEX_BUFFER);
            newMesh.primitives.back().vertexBuffer = mvGetBufferAssetID(&assetManager,
                std::string(glmesh.name) + std::to_string(currentPrimitive) + "_vertexBuffer",
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