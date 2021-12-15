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
    if (bufferview.byte_stride == -1)
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

void
load_gltf_assets(mvAssetManager& assetManager, mvGLTFModel& model)
{

    u32 nodeOffset = assetManager.nodeCount;
    u32 meshOffset = assetManager.meshCount;
    u32 materialOffset = assetManager.materialCount;
    u32 cameraOffset = assetManager.cameraCount;

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

        register_asset(&assetManager, glcamera.name, camera);
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
                default:
                    assert(false && "Undefined attribute type");
                    break;
                }
            }

            mvVertexLayout currentlayout = create_vertex_layout(attributes);

            std::vector<u32> indexBuffer;
            std::vector<f32> vertexBuffer;

            u32 triangleCount = origIndexBuffer.size() / 3;

            vertexBuffer.reserve(triangleCount * 14 * 3);
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

                indexBuffer.push_back(indexBuffer.size());
            }

            // calculate normals, tangents, bitangents
            for (size_t i = 0; i < indexBuffer.size() - 2; i += 3)
            {

                size_t i0 = indexBuffer[i];
                size_t i1 = indexBuffer[i + 1];
                size_t i2 = indexBuffer[i + 2];

                mvVec3 p0 = *((mvVec3*)&combinedVertexBuffer[i0 * 14]);
                mvVec3 p1 = *((mvVec3*)&combinedVertexBuffer[i1 * 14]);
                mvVec3 p2 = *((mvVec3*)&combinedVertexBuffer[i2 * 14]);

                mvVec3 n0 = *((mvVec3*)&combinedVertexBuffer[i0 * 14 + 3]);
                mvVec3 n1 = *((mvVec3*)&combinedVertexBuffer[i1 * 14 + 3]);
                mvVec3 n2 = *((mvVec3*)&combinedVertexBuffer[i2 * 14 + 3]);

                mvVec2 tex0 = *((mvVec2*)&combinedVertexBuffer[i0 * 14 + 6]);
                mvVec2 tex1 = *((mvVec2*)&combinedVertexBuffer[i1 * 14 + 6]);
                mvVec2 tex2 = *((mvVec2*)&combinedVertexBuffer[i2 * 14 + 6]);

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

                mvVec3 tangent = {
                    ((edge1.x * uv2.y) - (edge2.x * uv1.y)) * dirCorrection,
                    ((edge1.y * uv2.y) - (edge2.y * uv1.y)) * dirCorrection,
                    ((edge1.z * uv2.y) - (edge2.z * uv1.y)) * dirCorrection
                };

                mvVec3 bitangent = {
				    ((edge1.x * uv2.x) - (edge2.x * uv1.x)) * dirCorrection,
				    ((edge1.y * uv2.x) - (edge2.y * uv1.x)) * dirCorrection,
				    ((edge1.z * uv2.x) - (edge2.z * uv1.x)) * dirCorrection
			    };

                // project tangent and bitangent into the plane formed by the vertex' normal
                //mvVec3 newTangent = cNormalize(tangent - n * (tangent * n));
                *((mvVec3*)&combinedVertexBuffer[i0 * 14 + 8]) = normalize(tangent - n0 * (tangent * n0));
                *((mvVec3*)&combinedVertexBuffer[i1 * 14 + 8]) = normalize(tangent - n1 * (tangent * n1));
                *((mvVec3*)&combinedVertexBuffer[i2 * 14 + 8]) = normalize(tangent - n2 * (tangent * n2));

                //mvVec3 newBitangent = cNormalize(bitangent - n * (bitangent * n));
                *((mvVec3*)&combinedVertexBuffer[i0 * 14 + 11]) = normalize(bitangent - n0 * (bitangent * n0));
                *((mvVec3*)&combinedVertexBuffer[i1 * 14 + 11]) = normalize(bitangent - n1 * (bitangent * n1));
                *((mvVec3*)&combinedVertexBuffer[i2 * 14 + 11]) = normalize(bitangent - n2 * (bitangent * n2));

                // vertex 0
                vertexBuffer.push_back(p0.x);
                vertexBuffer.push_back(p0.y);
                vertexBuffer.push_back(p0.z);
                vertexBuffer.push_back(n0.x);
                vertexBuffer.push_back(n0.y);
                vertexBuffer.push_back(n0.z);
                vertexBuffer.push_back(tex0.x);
                vertexBuffer.push_back(tex0.y);
                vertexBuffer.push_back(combinedVertexBuffer[i0 * 14 + 8]);
                vertexBuffer.push_back(combinedVertexBuffer[i0 * 14 + 9]);
                vertexBuffer.push_back(combinedVertexBuffer[i0 * 14 + 10]);
                vertexBuffer.push_back(combinedVertexBuffer[i0 * 14 + 11]);
                vertexBuffer.push_back(combinedVertexBuffer[i0 * 14 + 12]);
                vertexBuffer.push_back(combinedVertexBuffer[i0 * 14 + 13]);

                // vertex 1
                vertexBuffer.push_back(p1.x);
                vertexBuffer.push_back(p1.y);
                vertexBuffer.push_back(p1.z);
                vertexBuffer.push_back(n1.x);
                vertexBuffer.push_back(n1.y);
                vertexBuffer.push_back(n1.z);
                vertexBuffer.push_back(tex1.x);
                vertexBuffer.push_back(tex1.y);
                vertexBuffer.push_back(combinedVertexBuffer[i1 * 14 + 8]);
                vertexBuffer.push_back(combinedVertexBuffer[i1 * 14 + 9]);
                vertexBuffer.push_back(combinedVertexBuffer[i1 * 14 + 10]);
                vertexBuffer.push_back(combinedVertexBuffer[i1 * 14 + 11]);
                vertexBuffer.push_back(combinedVertexBuffer[i1 * 14 + 12]);
                vertexBuffer.push_back(combinedVertexBuffer[i1 * 14 + 13]);

                // vertex 2
                vertexBuffer.push_back(p2.x);
                vertexBuffer.push_back(p2.y);
                vertexBuffer.push_back(p2.z);
                vertexBuffer.push_back(n2.x);
                vertexBuffer.push_back(n2.y);
                vertexBuffer.push_back(n2.z);
                vertexBuffer.push_back(tex2.x);
                vertexBuffer.push_back(tex2.y);
                vertexBuffer.push_back(combinedVertexBuffer[i2 * 14 + 8]);
                vertexBuffer.push_back(combinedVertexBuffer[i2 * 14 + 9]);
                vertexBuffer.push_back(combinedVertexBuffer[i2 * 14 + 10]);
                vertexBuffer.push_back(combinedVertexBuffer[i2 * 14 + 11]);
                vertexBuffer.push_back(combinedVertexBuffer[i2 * 14 + 12]);
                vertexBuffer.push_back(combinedVertexBuffer[i2 * 14 + 13]);

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

                mvMaterialData materialData{};
                materialData.albedo = *(mvVec4*)material.base_color_factor;
                materialData.metalness = material.metallic_factor;
                materialData.roughness = material.roughness_factor;
                materialData.hasAlpha = material.double_sided;
                materialData.emisiveFactor = *(mvVec3*)material.emissive_factor;
                materialData.occlusionStrength = material.occlusion_texture_strength;

                if (material.base_color_texture != -1)
                {
                    mvGLTFTexture& texture = model.textures[material.base_color_texture];
                    std::string uri = model.images[texture.image_index].uri;
                    if (model.images[texture.image_index].embedded)
                    {
                        newMesh.primitives.back().albedoTexture = mvGetTextureAssetID(&assetManager, model.root + newMesh.name + std::to_string(currentPrimitive) + uri + "_a", model.images[texture.image_index].data);
                    }
                    else
                        newMesh.primitives.back().albedoTexture = mvGetTextureAssetID(&assetManager, model.root + uri);
                    materialData.useAlbedoMap = true;
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
                        newMesh.primitives.back().normalTexture = mvGetTextureAssetID(&assetManager, model.root + uri);
                    materialData.useNormalMap = true;
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
                        newMesh.primitives.back().metalRoughnessTexture = mvGetTextureAssetID(&assetManager, model.root + uri);
                    materialData.useRoughnessMap = true;
                    materialData.useMetalMap = true;
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
                        newMesh.primitives.back().emissiveTexture = mvGetTextureAssetID(&assetManager, model.root + uri);
                    materialData.useEmissiveMap = true;
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
                        newMesh.primitives.back().occlusionTexture = mvGetTextureAssetID(&assetManager, model.root + uri);
                    materialData.useOcclusionMap = true;
                }

                std::string hash = std::string("PBR_PS.hlsl") +
                    std::string("PBR_VS.hlsl") +
                    std::to_string(materialData.albedo.x) +
                    std::to_string(materialData.albedo.y) +
                    std::to_string(materialData.albedo.z) +
                    std::to_string(materialData.albedo.w) +
                    std::to_string(materialData.metalness) +
                    std::to_string(materialData.roughness) +
                    std::to_string(materialData.emisiveFactor.x) +
                    std::to_string(materialData.emisiveFactor.y) +
                    std::to_string(materialData.emisiveFactor.z) +
                    std::to_string(materialData.radiance) +
                    std::to_string(materialData.fresnel) +
                    std::string(materialData.hasAlpha ? "T" : "F") +
                    std::string(materialData.useAlbedoMap ? "T" : "F") +
                    std::string(materialData.useNormalMap ? "T" : "F") +
                    std::string(materialData.useRoughnessMap ? "T" : "F") +
                    std::string(materialData.useOcclusionMap ? "T" : "F") +
                    std::string(materialData.useEmissiveMap ? "T" : "F") +
                    std::string(materialData.useMetalMap ? "T" : "F");

                newMesh.primitives.back().materialID = mvGetMaterialAssetID(&assetManager, hash);
                if (newMesh.primitives.back().materialID == -1)
                {
                    newMesh.primitives.back().materialID = register_asset(&assetManager, hash, create_material(assetManager, "PBR_VS.hlsl", "PBR_PS.hlsl", materialData));
                }

            }
            else
            {
                mvMaterialData materialData{};
                materialData.albedo = { 0.45f, 0.45f, 0.85f, 1.0f };
                materialData.metalness = 0.0f;
                materialData.roughness = 0.5f;
                materialData.hasAlpha = true;
                std::string hash = std::string("PBR_PS.hlsl") +
                    std::string("PBR_VS.hlsl") +
                    std::to_string(materialData.albedo.x) +
                    std::to_string(materialData.albedo.y) +
                    std::to_string(materialData.albedo.z) +
                    std::to_string(materialData.albedo.w) +
                    std::to_string(materialData.metalness) +
                    std::to_string(materialData.roughness) +
                    std::to_string(materialData.emisiveFactor.x) +
                    std::to_string(materialData.emisiveFactor.y) +
                    std::to_string(materialData.emisiveFactor.z) +
                    std::to_string(materialData.radiance) +
                    std::to_string(materialData.fresnel) +
                    std::string(materialData.hasAlpha ? "T" : "F") +
                    std::string(materialData.useAlbedoMap ? "T" : "F") +
                    std::string(materialData.useNormalMap ? "T" : "F") +
                    std::string(materialData.useRoughnessMap ? "T" : "F") +
                    std::string(materialData.useOcclusionMap ? "T" : "F") +
                    std::string(materialData.useEmissiveMap ? "T" : "F") +
                    std::string(materialData.useMetalMap ? "T" : "F");

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

        register_asset(&assetManager, newMesh.name + std::to_string(currentMesh), newMesh);

    }

    for (u32 currentNode = 0u; currentNode < model.node_count; currentNode++)
    {
        mvGLTFNode& glnode = model.nodes[currentNode];

        mvNode newNode{};
        newNode.name = glnode.name;
        if(glnode.mesh_index > -1)
            newNode.mesh = glnode.mesh_index + meshOffset;
        if (glnode.camera_index > -1)
        {
            
            newNode.camera = glnode.camera_index + cameraOffset;
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
            newNode.children[i] = (mvAssetID)glnode.children[i] + nodeOffset;

        newNode.rotation = *(mvVec4*)(glnode.rotation);
        newNode.scale = *(mvVec3*)(glnode.scale);
        newNode.translation = *(mvVec3*)(glnode.translation);

        if (glnode.hadMatrix)
        {
            newNode.matrix = *(mvMat4*)(glnode.matrix);
        }
        else
        {

            float x2 = newNode.rotation.x * newNode.rotation.x;
            float xy = newNode.rotation.x * newNode.rotation.y;
            float xz = newNode.rotation.x * newNode.rotation.z;
            float xw = newNode.rotation.x * newNode.rotation.w;
            float y2 = newNode.rotation.y * newNode.rotation.y;
            float yz = newNode.rotation.y * newNode.rotation.z;
            float yw = newNode.rotation.y * newNode.rotation.w;
            float z2 = newNode.rotation.z * newNode.rotation.z;
            float zw = newNode.rotation.z * newNode.rotation.w;
            float w2 = newNode.rotation.w * newNode.rotation.w;
            float m00 = x2 - y2 - z2 + w2;
            float m01 = 2.0f * (xy - zw);
            float m02 = 2.0f * (xz + yw);
            float m10 = 2.0f * (xy + zw);
            float m11 = -x2 + y2 - z2 + w2;
            float m12 = 2.0f * (yz - xw);
            float m20 = 2.0f * (xz - yw);
            float m21 = 2.0f * (yz + xw);
            float m22 = -x2 - y2 + z2 + w2;

            mvMat4 rotationMat = create_matrix(
                m00, m01, m02, 0.0f,
                m10, m11, m12, 0.0f,
                m20, m21, m22, 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f);

            newNode.matrix = translate(identity_mat4(), newNode.translation) * rotationMat * scale(identity_mat4(), newNode.scale);
        }

        register_asset(&assetManager, "node_" + std::to_string(currentNode), newNode);
    }

    for (u32 currentScene = 0u; currentScene < model.scene_count; currentScene++)
    {
        mvGLTFScene& glscene = model.scenes[currentScene];

        mvScene newScene{};
        newScene.nodeCount = glscene.node_count;

        for (i32 i = 0; i < glscene.node_count; i++)
            newScene.nodes[i] = glscene.nodes[i] + nodeOffset;

        register_asset(&assetManager, "scene_" + std::to_string(currentScene), newScene);
    }
}