#pragma once

#include "mvTypes.h"

// forward declarations
struct mvAssetManager;
struct mvGLTFModel;

mvAssetID load_gltf_assets(mvAssetManager& assetManager, mvGLTFModel& model);