#pragma once

#include <vector>
#include "mvTypes.h"

// forward declarations
struct mvAssetManager;
struct mvGLTFModel;

struct mvModel
{
    b8 loaded = false;
    mvAssetID defaultScene = -1;
    std::vector<mvAssetID> skins;
    std::vector<mvAssetID> cameras;
    std::vector<mvAssetID> meshes;
    std::vector<mvAssetID> nodes;
    std::vector<mvAssetID> animations;
    std::vector<mvAssetID> scenes;
    float                  minBoundary[3];
    float                  maxBoundary[3];
};

mvModel load_gltf_assets  (mvAssetManager& assetManager, mvGLTFModel& model);
void    unload_gltf_assets(mvAssetManager& assetManager, mvModel& model);