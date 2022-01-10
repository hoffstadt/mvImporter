#include "mvMesh.h"
#include <assert.h>
#include <cmath>
#include "mvSandbox.h"
#include "mvAssetManager.h"

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
