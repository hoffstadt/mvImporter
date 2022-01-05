#include "mvAnimation.h"
#include "mvAssetManager.h"
#include <assert.h>

static mvVec4
interpolate_quat(mvAnimationChannel& channel, f32 tcurrent, f32 tmax)
{
    
    // Wrap t around, so the animation loops.
    // Make sure that t is never earlier than the first keyframe and never later then the last keyframe.
    tcurrent = fmod(tcurrent, tmax);
    tcurrent = clamp(tcurrent, channel.inputdata[0], channel.inputdata.back());

    if (channel.tprev > tcurrent)
    {
        channel.prevKey = 0;
    }

    channel.tprev = tcurrent;

     // Find next keyframe: min{ t of input | t > prevKey }
    i32 nextKey = 0;
    for (i32 i = channel.prevKey; i < channel.inputdata.size(); i++)
    {
        if (tcurrent <= channel.inputdata[i])
        {
            nextKey = clamp(i, 1, (i32)channel.inputdata.size()-1);
            break;
        }
    }

    channel.prevKey = clamp(nextKey - 1, 0, nextKey);

    f32 keyDelta = channel.inputdata[nextKey] - channel.inputdata[channel.prevKey];

    // Normalize t: [t0, t1] -> [0, 1]
    f32 tn = (tcurrent - channel.inputdata[channel.prevKey]) / keyDelta;

    if (channel.path == "rotation")
    {
        if (channel.interpolation == "LINEAR")
        {
            mvVec4 q0 = *(mvVec4*)&channel.outputdata[channel.prevKey*4];
            mvVec4 q1 = *(mvVec4*)&channel.outputdata[nextKey*4];
            return slerpQuat(q0, q1, tn);
        }
        else if (channel.interpolation == "STEP")
        {
            return *(mvVec4*)&channel.outputdata[channel.prevKey * 4];
        }
        else if(channel.interpolation == "CUBICSPLINE")
        {
            i32 prevIndex = channel.prevKey * 4 * 3;
            i32 nextIndex = nextKey * 4 * 3;
            i32 A = 0;
            i32 V = 1 * 4;
            i32 B = 2 * 4;

            f32 tSq = tn * tn;
            f32 tCub = tSq * tn;

            mvVec4 result{};

            for (i32 i = 0; i < 4; i++)
            {
                f32 v0 = *(float*)&channel.outputdata[prevIndex + i + V];
                f32 a = keyDelta * *(float*)&channel.outputdata[nextIndex + i + A];
                f32 b = keyDelta * *(float*)&channel.outputdata[prevIndex + i + B];
                f32 v1 = *(float*)&channel.outputdata[nextIndex + i + V];

                result[i] = ((2 * tCub - 3 * tSq + 1) * v0) + ((tCub - 2 * tSq + tn) * b) + ((-2 * tCub + 3 * tSq) * v1) + ((tCub - tSq) * a);
            }

            return normalize(result);
        }
        else
        {
            assert(false);
        }
    }

    if (channel.interpolation == "LINEAR")
    {
        mvVec4 q0 = *(mvVec4*)&channel.outputdata[channel.prevKey * 4];
        mvVec4 q1 = *(mvVec4*)&channel.outputdata[nextKey * 4];
        return slerpQuat(q0, q1, tn);
    }
    else if (channel.interpolation == "STEP")
    {
        return *(mvVec4*)&channel.outputdata[channel.prevKey * 4];
    }
    else if (channel.interpolation == "CUBICSPLINE")
    {
        assert(false);
    }
    else
    {
        assert(false);
    }


}

static mvVec3
interpolate(mvAnimationChannel& channel, f32 tcurrent, f32 tmax)
{

    // Wrap t around, so the animation loops.
    // Make sure that t is never earlier than the first keyframe and never later then the last keyframe.
    tcurrent = fmod(tcurrent, tmax);
    tcurrent = clamp(tcurrent, channel.inputdata[0], channel.inputdata.back());

    if (channel.tprev > tcurrent)
    {
        channel.prevKey = 0;
    }

    channel.tprev = tcurrent;

    // Find next keyframe: min{ t of input | t > prevKey }
    i32 nextKey = 0;
    for (i32 i = channel.prevKey; i < channel.inputdata.size(); i++)
    {
        if (tcurrent <= channel.inputdata[i])
        {
            nextKey = clamp(i, 1, (i32)channel.inputdata.size() - 1);
            break;
        }
    }

    channel.prevKey = clamp(nextKey - 1, 0, nextKey);

    f32 keyDelta = channel.inputdata[nextKey] - channel.inputdata[channel.prevKey];

    // Normalize t: [t0, t1] -> [0, 1]
    f32 tn = (tcurrent - channel.inputdata[channel.prevKey]) / keyDelta;

    if (channel.interpolation == "LINEAR")
    {
        mvVec3 result{};
        mvVec3 prev = *(mvVec3*)&channel.outputdata[channel.prevKey * 3];
        mvVec3 next = *(mvVec3*)&channel.outputdata[nextKey * 3];

        result.x = prev.x * (1.0f - tn) + next.x * tn;
        result.y = prev.y * (1.0f - tn) + next.y * tn;
        result.z = prev.z * (1.0f - tn) + next.z * tn;

        return result;
    }
    else if (channel.interpolation == "STEP")
    {
        return *(mvVec3*)&channel.outputdata[channel.prevKey * 3];
    }
    else if (channel.interpolation == "CUBICSPLINE")
    {
        i32 prevIndex = channel.prevKey * 3 * 3;
        i32 nextIndex = nextKey * 3 * 3;
        i32 A = 0;
        i32 V = 1 * 3;
        i32 B = 2 * 3;

        f32 tSq = tn * tn;
        f32 tCub = tSq * tn;

        mvVec3 result{};

        for (i32 i = 0; i < 3; i++)
        {
            f32 v0 = *(float*)&channel.outputdata[prevIndex + i + V];
            f32 a = keyDelta * *(float*)&channel.outputdata[nextIndex + i + A];
            f32 b = keyDelta * *(float*)&channel.outputdata[prevIndex + i + B];
            f32 v1 = *(float*)&channel.outputdata[nextIndex + i + V];

            result[i] = ((2 * tCub - 3 * tSq + 1) * v0) + ((tCub - 2 * tSq + tn) * b) + ((-2 * tCub + 3 * tSq) * v1) + ((tCub - tSq) * a);
        }

        return result;
    }
    else
    {
        assert(false);
        return *(mvVec3*)&channel.outputdata[channel.prevKey * 3];
    }


}

void
advance(mvAssetManager& am, mvAnimation& animation, f32 tcurrent)
{
    if (animation.tmax == 0.0f)
    {
        for (i32 i = 0; i < animation.channelCount; i++)
        {
            f32 localmaxt = animation.channels[i].inputdata.back();
            if (localmaxt > animation.tmax)
                animation.tmax = localmaxt;
        }
    }

    for (i32 i = 0; i < animation.channelCount; i++)
    {
        mvAnimationChannel& channel = animation.channels[i];
        mvNode& node = am.nodes[channel.node].asset;
        if (channel.path == "rotation")
        {

            node.animationRotation = interpolate_quat(channel, tcurrent, animation.tmax);
            node.rotationAnimated = true;
        }
        else if (channel.path == "translation")
        {
            node.animationTranslation = interpolate(channel, tcurrent, animation.tmax);
            node.translationAnimated = true;
        }
        else if (channel.path == "scale")
        {
            node.animationScale = interpolate(channel, tcurrent, animation.tmax);
            node.scaleAnimated = true;

        }
        else
        {
            assert(false);
        }
    }
}