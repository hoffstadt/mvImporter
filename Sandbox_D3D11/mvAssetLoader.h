#pragma once

#include <vector>
#include "mvMaterials.h"
#include "mvGraphics.h"

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
    bool                     loaded = false;
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

mvModel load_gltf_assets  (mvGraphics& graphics, mvGLTFModel& model);
void    unload_gltf_assets(mvModel& model);