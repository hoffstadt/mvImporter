#pragma once

#include <vector>
#include <string>
#include "mvTypes.h"
#include "mvMath.h"

struct mvAssetManager;
struct mvAnimationChannel;
struct mvAnimation;

void advance_animations(mvAssetManager& am, mvAnimation& animation, f32 tcurrent);

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