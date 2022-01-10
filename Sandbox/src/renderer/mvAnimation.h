#pragma once

#include <vector>
#include <string>
#include "mvTypes.h"
#include "mvMath.h"
#include "mvTextures.h"

// forward declarations
struct mvAssetManager;
struct mvAnimationChannel;
struct mvAnimation;
struct mvSkin;

void advance_animations(mvAssetManager& am, mvAnimation& animation, f32 tcurrent);
void compute_joints    (mvAssetManager& am, mvMat4 transform, mvSkin& skin);

struct mvSkin
{
    u32       skeleton = 0u;
    mvAssetID joints[256];
    u32       jointCount = 0u;
    mvTexture jointTexture;
    std::vector<f32> inverseBindMatrices;
    f32* textureData = nullptr;
};

struct mvAnimationChannel
{
    mvAssetID        node = -1;
    std::vector<f32> inputdata;
    std::vector<u32> outputdata;
    std::string      path;
    std::string      interpolation = "LINEAR";
    f32              tprev = 0.0f;
    i32              prevKey = 0;
};

struct mvAnimation
{
    mvAnimationChannel* channels = nullptr;
    u32                 channelCount = 0u;
    f32                 tmax = 0.0f;
};