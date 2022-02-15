#pragma once

#include <vector>
#include "mvTypes.h"
#include "mvMaterials.h"

// forward declarations
struct mvGLTFModel;
struct mvCamera;
struct mvMesh;
struct mvNode;
struct mvAnimation;
struct mvScene;
struct mvSkin;

struct mvModel
{
    mvMaterialManager        materialManager;
    b8                       loaded = false;
    mvAssetID                defaultScene = -1;
    std::vector<mvSkin>      skins;
    std::vector<mvCamera>    cameras;
    std::vector<mvMesh>      meshes;
    std::vector<mvNode>      nodes;
    std::vector<mvAnimation> animations;
    std::vector<mvScene>     scenes;
    float                    minBoundary[3];
    float                    maxBoundary[3];
};

mvModel load_gltf_assets  (mvGLTFModel& model);
void    unload_gltf_assets(mvModel& model);