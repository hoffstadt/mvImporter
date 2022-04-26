#pragma once

#include <vector>
#include <string>
#include "mvGraphics.h"

// forward declarations
struct mvAnimationChannel;
struct mvAnimation;
struct mvSkin;
struct mvModel;

void advance_animations(mvModel& model, mvAnimation& animation, float tcurrent);
void compute_joints    (mvGraphics& graphics, mvModel& model, sMat4 transform, mvSkin& skin);

struct mvSkin
{
    unsigned int       skeleton = 0u;
    mvAssetID          joints[256];
    unsigned int       jointCount = 0u;
    mvTexture          jointTexture;
    std::vector<float> inverseBindMatrices;
    std::vector<float> textureData;
};

struct mvAnimationChannel
{
    mvAssetID          node = -1;
    std::vector<float> inputdata;
    std::vector<float> outputdata;
    std::string        path;
    std::string        interpolation = "LINEAR";
    float              tprev = 0.0f;
    int                prevKey = 0;
};

struct mvAnimation
{
    mvAnimationChannel* channels = nullptr;
    unsigned int        channelCount = 0u;
    float               tmax = 0.0f;
};