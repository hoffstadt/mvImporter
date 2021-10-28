#include "mvMesh.h"
#include <assert.h>
#include <cmath>
#include "mv3D_internal.h"
#include "mvObjLoader.h"
#include "mvAssetManager.h"

#define MV_IMPORTER_IMPLEMENTATION
#include "mvImporter.h"

void
mvLoadOBJAssets(mvAssetManager& assetManager, const std::string& root, const std::string& file)
{
    std::vector<mvAssetID> diffuseTextureMaps;
    std::vector<mvAssetID> normalTextureMaps;
    std::vector<mvAssetID> specularTextureMaps;
    std::vector<mvObjMaterial> objMaterials = mvLoadObjMaterials(root + file + ".mtl");
    mvObjModel objModel = mvLoadObjModel(root + file + ".obj");

    for (size_t i = 0; i < objModel.meshes.size(); i++)
    {
        for (size_t j = 0; j < objMaterials.size(); j++)
        {
            if (objMaterials[j].name == objModel.meshes[i]->material)
            {
                mvMesh newMesh{};
                newMesh.pbr = false;
                newMesh.name = objModel.meshes[i]->name;
                newMesh.layout = mvCreateVertexLayout(
                    {
                        mvVertexElement::Position3D,
                        mvVertexElement::Normal,
                        mvVertexElement::Texture2D,
                        mvVertexElement::Tangent,
                        mvVertexElement::Bitangent
                    }
                );

                if (!objMaterials[j].diffuseMap.empty())
                    newMesh.diffuseTexture = mvGetTextureAsset(&assetManager, root + objMaterials[j].diffuseMap);
                if (!objMaterials[j].normalMap.empty())
                    newMesh.normalTexture = mvGetTextureAsset(&assetManager, root + objMaterials[j].normalMap);
                if (!objMaterials[j].specularMap.empty())
                    newMesh.specularTexture = mvGetTextureAsset(&assetManager, root + objMaterials[j].specularMap);

                newMesh.vertexBuffer = mvGetBufferAsset(&assetManager, objModel.meshes[i]->averticies.data(), objModel.meshes[i]->averticies.size() * sizeof(f32) * 14, D3D11_BIND_VERTEX_BUFFER, newMesh.name + "_vertex");
                newMesh.indexBuffer = mvGetBufferAsset(&assetManager, objModel.meshes[i]->indicies.data(), objModel.meshes[i]->indicies.size() * sizeof(u32), D3D11_BIND_INDEX_BUFFER, newMesh.name + "_index");
                
                mvRegistryMeshAsset(&assetManager, newMesh);
            }
        }

    }

    
}

mvMesh
mvCreateTexturedCube(mvAssetManager& assetManager, f32 size)
{

    mvVertexLayout layout = mvCreateVertexLayout(
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
        -side, -side, -side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 0 near side
         side, -side, -side, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 1
        -side,  side, -side, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 2
         side,  side, -side, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 3
        -side, -side,  side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 4 far side
         side, -side,  side, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 5
        -side,  side,  side, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 6
         side,  side,  side, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 7
        -side, -side, -side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 8 left side
        -side,  side, -side, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 9
        -side, -side,  side, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 10
        -side,  side,  side, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 11
         side, -side, -side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 12 right side
         side,  side, -side, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 13
         side, -side,  side, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 14
         side,  side,  side, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 15
        -side, -side, -side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 16 bottom side
         side, -side, -side, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 17
        -side, -side,  side, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 18
         side, -side,  side, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 19
        -side,  side, -side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 20 top side
         side,  side, -side, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 21
        -side,  side,  side, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 22
         side,  side,  side, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f   // 23
    };

    static auto indices = std::vector<u32>{
        0,  2,  1,  2,  3,  1,
        4,  5,  7,  4,  7,  6,
        8, 10,  9, 10, 11,  9,
        12, 13, 15, 12, 15, 14,
        16, 17, 18, 18, 17, 19,
        20, 23, 21, 20, 22, 23
    };

    for (size_t i = 0; i < indices.size(); i += 3)
    {

        mvVec3 p0 = { vertices[14 * indices[i]], vertices[14 * indices[i] + 1], vertices[14 * indices[i] + 2] };
        mvVec3 p1 = { vertices[14 * indices[i+1]], vertices[14 * indices[i + 1] + 1], vertices[14 * indices[i + 1] + 2] };
        mvVec3 p2 = { vertices[14 * indices[i+2]], vertices[14 * indices[i + 2] + 1], vertices[14 * indices[i + 2] + 2] };

        mvVec3 n = mvNormalize(mvCross(p1 - p0, p2 - p0));
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
    mesh.pbr = false;
    mesh.name = "textured cube";
    mesh.layout = layout;
    mesh.vertexBuffer = mvGetBufferAsset(&assetManager, 
        vertices.data(), 
        vertices.size() * sizeof(f32), 
        D3D11_BIND_VERTEX_BUFFER, 
        "textured_cube_vertex" + std::to_string(side));
    mesh.indexBuffer = mvGetBufferAsset(&assetManager, indices.data(), indices.size() * sizeof(u32), D3D11_BIND_INDEX_BUFFER, "textured_cube_index");

    return mesh;
}

mvMesh
mvCreateTexturedQuad(mvAssetManager& assetManager, f32 size)
{

    mvVertexLayout layout = mvCreateVertexLayout(
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
         side, -side, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        -side, -side, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
         side,  side, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    };

    static auto indices = std::vector<u32>{
        1, 2, 0,
        3, 1, 0
    };

    for (size_t i = 0; i < indices.size(); i += 3)
    {
        mvVec3 p0 = { vertices[14 * indices[i]], vertices[14 * indices[i] + 1], vertices[14 * indices[i] + 2] };
        mvVec3 p1 = { vertices[14 * indices[i + 1]], vertices[14 * indices[i + 1] + 1], vertices[14 * indices[i + 1] + 2] };
        mvVec3 p2 = { vertices[14 * indices[i + 2]], vertices[14 * indices[i + 2] + 1], vertices[14 * indices[i + 2] + 2] };

        mvVec3 n = mvNormalize(mvCross(p1 - p0, p2 - p0));
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
    mesh.layout = layout;
    mesh.pbr = false;
    mesh.vertexBuffer = mvGetBufferAsset(&assetManager, vertices.data(), vertices.size() * sizeof(f32), D3D11_BIND_VERTEX_BUFFER, "textured_quad_vertex");
    mesh.indexBuffer = mvGetBufferAsset(&assetManager, indices.data(), indices.size()*sizeof(u32), D3D11_BIND_INDEX_BUFFER, "textured_quad_index");

    return mesh;
}

mv_internal u8
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

mv_internal void
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
mv_internal T*
mvGetBufferViewStart(mvGLTFModel& model, mvGLTFAccessor& accessor)
{
    mvGLTFBufferView& bufferview = model.bufferviews[accessor.buffer_view_index];
    mvVerifyBufferViewStride(model, accessor);
    return (T*)&model.buffers[bufferview.buffer_index].data[bufferview.byte_offset + accessor.byteOffset];
}

template<typename T, typename W>
mv_internal void
mvFillBuffer(mvGLTFModel& model, mvGLTFAccessor& accessor, std::vector<W>& outBuffer, u32 componentCap = 4)
{
    mvVerifyBufferViewStride(model, accessor);
    mvGLTFBufferView bufferView = model.bufferviews[accessor.buffer_view_index];
    char* bufferRawData = model.buffers[bufferView.buffer_index].data;
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
mvLoadGLTFAssets(mvAssetManager& assetManager, mvGLTFModel& model)
{
    for (u32 currentMesh = 0u; currentMesh < model.mesh_count; currentMesh++)
    {
        mvGLTFMesh& glmesh = model.meshes[currentMesh];

        std::vector<u32> origIndexBuffer;
        u8 indexCompCount = mvGetAccessorItemCompCount(model.accessors[glmesh.indices_index]);

        mvGLTFComponentType indexCompType = model.accessors[glmesh.indices_index].component_type;

        switch (indexCompType)
        {
        case MV_IMP_BYTE:
        case MV_IMP_UNSIGNED_BYTE:
            mvFillBuffer<u8>(model, model.accessors[glmesh.indices_index], origIndexBuffer);
            break;

        case MV_IMP_SHORT:
        case MV_IMP_UNSIGNED_SHORT:
            mvFillBuffer<u16>(model, model.accessors[glmesh.indices_index], origIndexBuffer);
            break;

        case MV_IMP_INT:
        case MV_IMP_UNSIGNED_INT:
            mvFillBuffer<u32>(model, model.accessors[glmesh.indices_index], origIndexBuffer);
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
        for (u32 i = 0; i < glmesh.attribute_count; i++)
        {
            auto& attribute = glmesh.attributes[i];
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

        mvVertexLayout currentlayout = mvCreateVertexLayout(attributes);

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

            f32 x0 = positionAttributeBuffer[i0*3];
            f32 y0 = positionAttributeBuffer[i0*3+1];
            f32 z0 = positionAttributeBuffer[i0*3+2];

            f32 nx0 = normalAttributeBuffer[i0*3];
            f32 ny0 = normalAttributeBuffer[i0*3 + 1];
            f32 nz0 = normalAttributeBuffer[i0*3 + 2];

            //f32 u0 = fmod(textureAttributeBuffer[i0*2] + 1.0f, 1.0f);
            //f32 v0 = fmod(textureAttributeBuffer[i0*2 + 1] + 1.0f, 1.0f);
            f32 u0 = textureAttributeBuffer[i0 * 2];
            f32 v0 = textureAttributeBuffer[i0 * 2 + 1];

            //f32 tx0 = tangentAttributeBuffer[i0];
            //f32 ty0 = tangentAttributeBuffer[i0 + 1];
            //f32 tz0 = tangentAttributeBuffer[i0 + 2];

            //f32 btx0 = bitangentAttributeBuffer[i0];
            //f32 bty0 = bitangentAttributeBuffer[i0 + 1];
            //f32 btz0 = bitangentAttributeBuffer[i0 + 2];

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
            combinedVertexBuffer.push_back(0.0f); // we will calculate
            combinedVertexBuffer.push_back(0.0f); // we will calculate
            combinedVertexBuffer.push_back(0.0f); // we will calculate

            indexBuffer.push_back(indexBuffer.size());
        }

        // calculate normals, tangents, and bitangents
        for (size_t i = 0; i < indexBuffer.size()-2; i += 3)
        {

            size_t i0 = indexBuffer[i];
            size_t i1 = indexBuffer[i + 1];
            size_t i2 = indexBuffer[i + 2];

            mvVec3 p0 = *((mvVec3*)&combinedVertexBuffer[i0*14]);
            mvVec3 p1 = *((mvVec3*)&combinedVertexBuffer[i1*14]);
            mvVec3 p2 = *((mvVec3*)&combinedVertexBuffer[i2*14]);

            mvVec3 n0 = *((mvVec3*)&combinedVertexBuffer[i0 * 14 + 3]);
            mvVec3 n1 = *((mvVec3*)&combinedVertexBuffer[i1 * 14 + 3]);
            mvVec3 n2 = *((mvVec3*)&combinedVertexBuffer[i2 * 14 + 3]);

            mvVec2 tex0 = *((mvVec2*)&combinedVertexBuffer[i0*14+6]);
            mvVec2 tex1 = *((mvVec2*)&combinedVertexBuffer[i1*14+6]);
            mvVec2 tex2 = *((mvVec2*)&combinedVertexBuffer[i2*14+6]);

            // calculate normals
            //mvVec3 n = cNormalize(cCross(p1 - p0, p2 - p0));
            //*((mvVec3*)&combinedVertexBuffer[i0 * 14 + 3]) = n;
            //*((mvVec3*)&combinedVertexBuffer[i1 * 14 + 3]) = n;
            //*((mvVec3*)&combinedVertexBuffer[i2 * 14 + 3]) = n;

            // calculate tangents & bitangents
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
            *((mvVec3*)&combinedVertexBuffer[i0 * 14 + 8]) = mvNormalize(tangent - n0 * (tangent * n0));
            *((mvVec3*)&combinedVertexBuffer[i1 * 14 + 8]) = mvNormalize(tangent - n1 * (tangent * n1));
            *((mvVec3*)&combinedVertexBuffer[i2 * 14 + 8]) = mvNormalize(tangent - n2 * (tangent * n2));
            
            //mvVec3 newBitangent = cNormalize(bitangent - n * (bitangent * n));
            *((mvVec3*)&combinedVertexBuffer[i0 * 14 + 11]) = mvNormalize(bitangent - n0 * (bitangent * n0));
            *((mvVec3*)&combinedVertexBuffer[i1 * 14 + 11]) = mvNormalize(bitangent - n1 * (bitangent * n1));
            *((mvVec3*)&combinedVertexBuffer[i2 * 14 + 11]) = mvNormalize(bitangent - n2 * (bitangent * n2));

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

        // left hand
        for (size_t i = 0; i < vertexBuffer.size(); i += 14)
        {
            vertexBuffer[i + 2]  *= -1.0f;                    // z
            vertexBuffer[i + 5]  *= -1.0f;                    // nz
            vertexBuffer[i + 10] *= -1.0f;                    // tz
            vertexBuffer[i + 13] *= -1.0f;                    // bz
            //vertexBuffer[i + 7] = 1.0f - vertexBuffer[i + 7]; // v
        }

        // left hand
        for (size_t i = 0; i < indexBuffer.size(); i += 3)
        {

            size_t i0 = indexBuffer[i];
            size_t i2 = indexBuffer[i + 2];

            indexBuffer[i] = i2;
            indexBuffer[i + 2] = i0;
        }
        mvMesh newMesh{};
        newMesh.pbr = true;
        newMesh.name = glmesh.name;
        newMesh.layout = mvCreateVertexLayout(
            {
                mvVertexElement::Position3D,
                mvVertexElement::Normal,
                mvVertexElement::Texture2D,
                mvVertexElement::Tangent,
                mvVertexElement::Bitangent
            }
        );

        // upload index buffer
        if (glmesh.material_index != -1)
        {
            mvGLTFMaterial& material = model.materials[glmesh.material_index];
            if (material.base_color_texture != -1)
            {
                mvGLTFTexture& texture = model.textures[material.base_color_texture];
                std::string uri = model.images[texture.image_index].uri;
                newMesh.albedoTexture = mvGetTextureAsset(&assetManager, model.root + uri);
                newMesh.diffuseTexture = newMesh.albedoTexture;
            }

            if (material.normal_texture != -1)
            {
                mvGLTFTexture& texture = model.textures[material.normal_texture];
                std::string uri = model.images[texture.image_index].uri;
                newMesh.normalTexture = mvGetTextureAsset(&assetManager, model.root + uri);
            }

            if (material.metallic_roughness_texture != -1)
            {
                mvGLTFTexture& texture = model.textures[material.metallic_roughness_texture];
                std::string uri = model.images[texture.image_index].uri;
                newMesh.metalRoughnessTexture = mvGetTextureAsset(&assetManager, model.root + uri);
            }

        }
        
        newMesh.indexBuffer = mvGetBufferAsset(&assetManager, indexBuffer.data(), indexBuffer.size() * sizeof(u32), D3D11_BIND_INDEX_BUFFER, std::string(glmesh.name) + "_indexbuffer");
        newMesh.vertexBuffer = mvGetBufferAsset(&assetManager,
            vertexBuffer.data(),
            vertexBuffer.size() * sizeof(f32),
            D3D11_BIND_VERTEX_BUFFER,
            std::string(glmesh.name) + "_vertexBuffer");

        mvRegistryMeshAsset(&assetManager, newMesh);
    }

}