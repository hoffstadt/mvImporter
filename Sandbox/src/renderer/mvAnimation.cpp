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

static void
interpolate(mvAnimationChannel& channel, f32 tcurrent, f32 tmax, u32 stride, f32* out)
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

        for (i32 i = 0; i < stride; i++)
        {
            f32 prev = channel.outputdata[channel.prevKey * stride + i];
            f32 next = channel.outputdata[nextKey * stride + i];
            out[i] = prev * (1.0f - tn) + next * tn;
        }

    }
    else if (channel.interpolation == "STEP")
    {
        //return *(mvVec3*)&channel.outputdata[channel.prevKey * 3];
        for (i32 i = 0; i < stride; i++)
        {
            out[i] = channel.outputdata[channel.prevKey * stride + i];
        }
    }
    else if (channel.interpolation == "CUBICSPLINE")
    {
        i32 prevIndex = channel.prevKey * stride * 3;
        i32 nextIndex = nextKey * stride * 3;
        i32 A = 0;
        i32 V = 1 * stride;
        i32 B = 2 * stride;

        f32 tSq = tn * tn;
        f32 tCub = tSq * tn;

        mvVec3 result{};

        for (i32 i = 0; i < stride; i++)
        {
            f32 v0 = *(float*)&channel.outputdata[prevIndex + i + V];
            f32 a = keyDelta * *(float*)&channel.outputdata[nextIndex + i + A];
            f32 b = keyDelta * *(float*)&channel.outputdata[prevIndex + i + B];
            f32 v1 = *(float*)&channel.outputdata[nextIndex + i + V];

            out[i] = ((2 * tCub - 3 * tSq + 1) * v0) + ((tCub - 2 * tSq + tn) * b) + ((-2 * tCub + 3 * tSq) * v1) + ((tCub - tSq) * a);
            //result[i] = ((2 * tCub - 3 * tSq + 1) * v0) + ((tCub - 2 * tSq + tn) * b) + ((-2 * tCub + 3 * tSq) * v1) + ((tCub - tSq) * a);
        }

    }
    else
    {
        assert(false);
    }


}

void
advance_animations(mvAssetManager& am, mvAnimation& animation, f32 tcurrent)
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
            interpolate(channel, tcurrent, animation.tmax, 3, &node.animationTranslation.x);
            node.translationAnimated = true;
        }
        else if (channel.path == "scale")
        {
            interpolate(channel, tcurrent, animation.tmax, 3, &node.animationScale.x);
            node.scaleAnimated = true;

        }
        else if (channel.path == "weights")
        {
            mvMesh& mesh = am.meshes[node.mesh].asset;
            std::vector<f32> weightsAnimated;
            weightsAnimated.resize(mesh.weightCount);
            interpolate(channel, tcurrent, animation.tmax, mesh.weightCount, weightsAnimated.data());
            for (i32 j = 0; j < mesh.weightCount; j++)
            {
                mesh.weightsAnimated[j * 4] = weightsAnimated[j];
            }

        }
        else
        {
            assert(false);
        }

        mvVec3& trans = node.translationAnimated ? node.animationTranslation : node.translation;
        mvVec3& scal = node.scaleAnimated ? node.animationScale : node.scale;
        mvVec4& rot = node.rotationAnimated ? node.animationRotation : node.rotation;
        node.transform = rotation_translation_scale(rot, trans, scal);
    }
}

void
compute_joints(mvAssetManager& am, mvMat4 transform, mvSkin& skin)
{
    u32 textureWidth = ceil(sqrt(skin.jointCount * 8));

    for (u32 i = 0; i < skin.jointCount; i++)
    {
        s32 joint = skin.joints[i];
        mvNode& node = am.nodes[joint].asset;
        mvMat4 ibm = (*(mvMat4*)&skin.inverseBindMatrices[i * 16]);
        mvMat4 jointMatrix = transform * node.worldTransform * ibm;
        mvMat4 normalMatrix = transpose(invert(jointMatrix));

        *(mvMat4*)&skin.textureData[i * 32] = jointMatrix;
        *(mvMat4*)&skin.textureData[i * 32 + 16] = normalMatrix;
    }

    update_dynamic_texture(skin.jointTexture, textureWidth, textureWidth, skin.textureData);
}