#pragma once

#include <string>
#include "mvMath.h"
#include "mvBuffers.h"
#include "mvPipeline.h"
#include "mvTextures.h"

// forward declarations
struct mvAssetManager;
struct mvGLTFModel;
struct mvMeshPrimitive;
struct mvMesh;
struct mvSkin;
struct mvNode;


mvMesh    create_cube         (f32 size = 1.0f);
mvMesh    create_textured_cube(f32 size = 1.0f);
mvMesh    create_textured_quad(f32 size = 1.0f);
mvMesh    create_frustum      (f32 width, f32 height, f32 nearZ, f32 farZ);
mvMesh    create_frustum2     (f32 fov, f32 aspect, f32 nearZ, f32 farZ);
mvMesh    create_ortho_frustum(f32 width, f32 height, f32 nearZ, f32 farZ);

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
    f32*           morphData = nullptr;
};

struct mvMesh
{
    std::string                  name;
    std::vector<mvMeshPrimitive> primitives;
    std::vector<f32>             weights;
    std::vector<f32>             weightsAnimated;
    u32                          weightCount;
    mvConstBuffer                morphBuffer;
};
