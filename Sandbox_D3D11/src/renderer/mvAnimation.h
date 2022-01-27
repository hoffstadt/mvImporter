#pragma once

#include <vector>
#include <string>
#include "mvTypes.h"
#include "mvMath.h"
#include "mvTextures.h"

// forward declarations
struct mvAnimationChannel;
struct mvAnimation;
struct mvSkin;
struct mvModel;

void advance_animations(mvModel& model, mvAnimation& animation, f32 tcurrent);
void compute_joints    (mvModel& model, mvMat4 transform, mvSkin& skin);

struct mvSkin
{
    u32       skeleton = 0u;
    mvAssetID joints[256];
    u32       jointCount = 0u;
    mvTexture jointTexture;
    std::vector<f32> inverseBindMatrices;
    std::vector<f32> textureData;
};

struct mvAnimationChannel
{
    mvAssetID        node = -1;
    std::vector<f32> inputdata;
    std::vector<f32> outputdata;
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