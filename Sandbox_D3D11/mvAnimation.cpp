#include "mvAnimation.h"
#include "mvAssetLoader.h"
#include <assert.h>

static mvVec4
interpolate_quat(mvAnimationChannel& channel, float tcurrent, float tmax)
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
    int nextKey = 0;
    for (int i = channel.prevKey; i < channel.inputdata.size(); i++)
    {
        if (tcurrent <= channel.inputdata[i])
        {
            nextKey = clamp(i, 1, (int)channel.inputdata.size()-1);
            break;
        }
    }

    channel.prevKey = clamp(nextKey - 1, 0, nextKey);

    float keyDelta = channel.inputdata[nextKey] - channel.inputdata[channel.prevKey];

    // Normalize t: [t0, t1] -> [0, 1]
    float tn = (tcurrent - channel.inputdata[channel.prevKey]) / keyDelta;

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
            int prevIndex = channel.prevKey * 4 * 3;
            int nextIndex = nextKey * 4 * 3;
            int A = 0;
            int V = 1 * 4;
            int B = 2 * 4;

            float tSq = tn * tn;
            float tCub = tSq * tn;

            mvVec4 result{};

            for (int i = 0; i < 4; i++)
            {
                float v0 = *(float*)&channel.outputdata[prevIndex + i + V];
                float a = keyDelta * *(float*)&channel.outputdata[nextIndex + i + A];
                float b = keyDelta * *(float*)&channel.outputdata[prevIndex + i + B];
                float v1 = *(float*)&channel.outputdata[nextIndex + i + V];

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

    return mvVec4{};
}

static void
interpolate(mvAnimationChannel& channel, float tcurrent, float tmax, unsigned int stride, float* out)
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
    int nextKey = 0;
    for (int i = channel.prevKey; i < channel.inputdata.size(); i++)
    {
        if (tcurrent <= channel.inputdata[i])
        {
            nextKey = clamp(i, 1, (int)channel.inputdata.size() - 1);
            break;
        }
    }

    channel.prevKey = clamp(nextKey - 1, 0, nextKey);

    float keyDelta = channel.inputdata[nextKey] - channel.inputdata[channel.prevKey];

    // Normalize t: [t0, t1] -> [0, 1]
    float tn = (tcurrent - channel.inputdata[channel.prevKey]) / keyDelta;

    if (channel.interpolation == "LINEAR")
    {

        for (int i = 0; i < stride; i++)
        {
            float prev = channel.outputdata[channel.prevKey * stride + i];
            float next = channel.outputdata[nextKey * stride + i];
            out[i] = prev * (1.0f - tn) + next * tn;
        }

    }
    else if (channel.interpolation == "STEP")
    {
        //return *(mvVec3*)&channel.outputdata[channel.prevKey * 3];
        for (int i = 0; i < stride; i++)
        {
            out[i] = channel.outputdata[channel.prevKey * stride + i];
        }
    }
    else if (channel.interpolation == "CUBICSPLINE")
    {
        int prevIndex = channel.prevKey * stride * 3;
        int nextIndex = nextKey * stride * 3;
        int A = 0;
        int V = 1 * stride;
        int B = 2 * stride;

        float tSq = tn * tn;
        float tCub = tSq * tn;

        mvVec3 result{};

        for (int i = 0; i < stride; i++)
        {
            float v0 = *(float*)&channel.outputdata[prevIndex + i + V];
            float a = keyDelta * *(float*)&channel.outputdata[nextIndex + i + A];
            float b = keyDelta * *(float*)&channel.outputdata[prevIndex + i + B];
            float v1 = *(float*)&channel.outputdata[nextIndex + i + V];

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
advance_animations(mvModel& model, mvAnimation& animation, float tcurrent)
{
    if (animation.tmax == 0.0f)
    {
        for (int i = 0; i < animation.channelCount; i++)
        {
            float localmaxt = animation.channels[i].inputdata.back();
            if (localmaxt > animation.tmax)
                animation.tmax = localmaxt;
        }
    }

    for (int i = 0; i < animation.channelCount; i++)
    {
        mvAnimationChannel& channel = animation.channels[i];
        mvNode& node = model.nodes[channel.node];
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
            mvMesh& mesh = model.meshes[node.mesh];
            std::vector<float> weightsAnimated;
            weightsAnimated.resize(mesh.weightCount);
            interpolate(channel, tcurrent, animation.tmax, mesh.weightCount, weightsAnimated.data());
            for (int j = 0; j < mesh.weightCount; j++)
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
compute_joints(mvGraphics& graphics, mvModel& model, mvMat4 transform, mvSkin& skin)
{
    unsigned int textureWidth = ceil(sqrt(skin.jointCount * 8));

    for (unsigned int i = 0; i < skin.jointCount; i++)
    {
        int joint = skin.joints[i];
        mvNode& node = model.nodes[joint];
        mvMat4 ibm = (*(mvMat4*)&skin.inverseBindMatrices[i * 16]);
        mvMat4 jointMatrix = transform * node.worldTransform * ibm;
        mvMat4 invertJoint = invert(jointMatrix);
        mvMat4 normalMatrix = transpose(invertJoint);

        *(mvMat4*)&skin.textureData[i * 32] = jointMatrix;
        *(mvMat4*)&skin.textureData[i * 32 + 16] = normalMatrix;
    }

    update_dynamic_texture(graphics, skin.jointTexture, textureWidth, textureWidth, skin.textureData.data());
}