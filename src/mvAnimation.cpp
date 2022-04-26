#include "mvAnimation.h"
#include "mvAssetLoader.h"
#include "sMath.h"
#include <assert.h>

static sVec4
slerpQuat(sVec4 q1, sVec4 q2, float t)
{

	// from https://glmatrix.net/docs/quat.js.html
	sVec4 qn1 = Semper::normalize(q1);
	sVec4 qn2 = Semper::normalize(q2);

	sVec4 qresult{};

	float ax = qn1.x;
	float ay = qn1.y;
	float az = qn1.z;
	float aw = qn1.w;

	float bx = qn2.x;
	float by = qn2.y;
	float bz = qn2.z;
	float bw = qn2.w;

	float omega = 0.0f;
	float cosom = 0.0f;
	float sinom = 0.0f;
	float scale0 = 0.0f;
	float scale1 = 0.0f;

	// calc cosine
	cosom = ax * bx + ay * by + az * bz + aw * bw;

	// adjust signs (if necessary)
	if (cosom < 0.0f) 
	{
		cosom = -cosom;
		bx = -bx;
		by = -by;
		bz = -bz;
		bw = -bw;
	}

	// calculate coefficients
	if (1.0f - cosom > 0.000001f)
	{
		// standard case (slerp)
		omega = acos(cosom);
		sinom = sin(omega);
		scale0 = sin((1.0f - t) * omega) / sinom;
		scale1 = sin(t * omega) / sinom;
	}
	else 
	{
		// "from" and "to" quaternions are very close
		//  ... so we can do a linear interpolation
		scale0 = 1.0f - t;
		scale1 = t;
	}

	// calculate final values
	qresult[0] = scale0 * ax + scale1 * bx;
	qresult[1] = scale0 * ay + scale1 * by;
	qresult[2] = scale0 * az + scale1 * bz;
	qresult[3] = scale0 * aw + scale1 * bw;

	qresult = Semper::normalize(qresult);

	return qresult;
}

static sVec4
interpolate_quat(mvAnimationChannel& channel, float tcurrent, float tmax)
{
    
    // Wrap t around, so the animation loops.
    // Make sure that t is never earlier than the first keyframe and never later then the last keyframe.
    tcurrent = fmod(tcurrent, tmax);
    tcurrent = Semper::clamp(channel.inputdata[0], tcurrent, channel.inputdata.back());

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
            nextKey = Semper::clamp(1, i, (int)channel.inputdata.size()-1);
            break;
        }
    }

    channel.prevKey = Semper::clamp(0, nextKey - 1, nextKey);

    float keyDelta = channel.inputdata[nextKey] - channel.inputdata[channel.prevKey];

    // Normalize t: [t0, t1] -> [0, 1]
    float tn = (tcurrent - channel.inputdata[channel.prevKey]) / keyDelta;

    if (channel.path == "rotation")
    {
        if (channel.interpolation == "LINEAR")
        {
            sVec4 q0 = *(sVec4*)&channel.outputdata[channel.prevKey*4];
            sVec4 q1 = *(sVec4*)&channel.outputdata[nextKey*4];
            return slerpQuat(q0, q1, tn);
        }
        else if (channel.interpolation == "STEP")
        {
            return *(sVec4*)&channel.outputdata[channel.prevKey * 4];
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

            sVec4 result{};

            for (int i = 0; i < 4; i++)
            {
                float v0 = *(float*)&channel.outputdata[prevIndex + i + V];
                float a = keyDelta * *(float*)&channel.outputdata[nextIndex + i + A];
                float b = keyDelta * *(float*)&channel.outputdata[prevIndex + i + B];
                float v1 = *(float*)&channel.outputdata[nextIndex + i + V];

                result[i] = ((2 * tCub - 3 * tSq + 1) * v0) + ((tCub - 2 * tSq + tn) * b) + ((-2 * tCub + 3 * tSq) * v1) + ((tCub - tSq) * a);
            }

            return Semper::normalize(result);
        }
        else
        {
            assert(false);
        }
    }

    if (channel.interpolation == "LINEAR")
    {
        sVec4 q0 = *(sVec4*)&channel.outputdata[channel.prevKey * 4];
        sVec4 q1 = *(sVec4*)&channel.outputdata[nextKey * 4];
        return slerpQuat(q0, q1, tn);
    }
    else if (channel.interpolation == "STEP")
    {
        return *(sVec4*)&channel.outputdata[channel.prevKey * 4];
    }
    else if (channel.interpolation == "CUBICSPLINE")
    {
        assert(false);
    }
    else
    {
        assert(false);
    }

    return sVec4{};
}

static void
interpolate(mvAnimationChannel& channel, float tcurrent, float tmax, unsigned int stride, float* out)
{

    // Wrap t around, so the animation loops.
    // Make sure that t is never earlier than the first keyframe and never later then the last keyframe.
    tcurrent = fmod(tcurrent, tmax);
    tcurrent = Semper::clamp(channel.inputdata[0], tcurrent, channel.inputdata.back());

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
            nextKey = Semper::clamp(1, i,  (int)channel.inputdata.size() - 1);
            break;
        }
    }

    channel.prevKey = Semper::clamp(0, nextKey - 1, nextKey);

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

        sVec3 result{};

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

        sVec3& trans = node.translationAnimated ? node.animationTranslation : node.translation;
        sVec3& scal = node.scaleAnimated ? node.animationScale : node.scale;
        sVec4& rot = node.rotationAnimated ? node.animationRotation : node.rotation;
        node.transform = Semper::rotation_translation_scale(rot, trans, scal);
    }
}

void
compute_joints(mvGraphics& graphics, mvModel& model, sMat4 transform, mvSkin& skin)
{
    unsigned int textureWidth = ceil(sqrt(skin.jointCount * 8));

    for (unsigned int i = 0; i < skin.jointCount; i++)
    {
        int joint = skin.joints[i];
        mvNode& node = model.nodes[joint];
        sMat4 ibm = (*(sMat4*)&skin.inverseBindMatrices[i * 16]);
        sMat4 jointMatrix = transform * node.worldTransform * ibm;
        sMat4 invertJoint = Semper::invert(jointMatrix);
        sMat4 normalMatrix = Semper::transpose(invertJoint);

        *(sMat4*)&skin.textureData[i * 32] = jointMatrix;
        *(sMat4*)&skin.textureData[i * 32 + 16] = normalMatrix;
    }

    update_dynamic_texture(graphics, skin.jointTexture, textureWidth, textureWidth, skin.textureData.data());
}