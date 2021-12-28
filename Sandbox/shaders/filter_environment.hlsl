
#define M_PI 3.1415926535897932384626433832795

// enum
const uint cLambertian = 0;
const uint cGGX = 1;
const uint cCharlie = 2;

cbuffer MetaData : register(b0)
{
    int resolution;
    int width;
    int height;
    float roughness;

    uint sampleCount;
    uint currentMipLevel;
    float lodBias;
    uint distribution;
};

TextureCube tex : register(t0);
SamplerState sam : register(s0);

RWStructuredBuffer<float4> FaceOut_0 : register(u0);
RWStructuredBuffer<float4> FaceOut_1 : register(u1);
RWStructuredBuffer<float4> FaceOut_2 : register(u2);
RWStructuredBuffer<float4> FaceOut_3 : register(u3);
RWStructuredBuffer<float4> FaceOut_4 : register(u4);
RWStructuredBuffer<float4> FaceOut_5 : register(u5);

// Hammersley Points on the Hemisphere
// CC BY 3.0 (Holger Dammertz)
// http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
// with adapted interface
float radicalInverse_VdC(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

// hammersley2d describes a sequence of points in the 2d unit square [0,1)^2
// that can be used for quasi Monte Carlo integration
float2 hammersley2d(int i, int N)
{
    return float2(float(i) / float(N), radicalInverse_VdC(uint(i)));
}

// Hemisphere Sample

// TBN generates a tangent bitangent normal coordinate frame from the normal
// (the normal must be normalized)
float3x3 generateTBN(float3 normal)
{
    float3 bitangent = float3(0.0, 1.0, 0.0);

    float NdotUp = dot(normal, float3(0.0, 1.0, 0.0));
    float epsilon = 0.00001;
    if (1.0 - abs(NdotUp) <= epsilon)
    {
        // Sampling +Y or -Y, so we need a more robust bitangent.
        if (NdotUp > 0.0)
        {
            bitangent = float3(0.0, 0.0, 1.0);
        }
        else
        {
            bitangent = float3(0.0, 0.0, -1.0);
        }
    }

    float3 tangent = normalize(cross(bitangent, normal));
    bitangent = cross(normal, tangent);

    return transpose(float3x3(tangent, bitangent, normal));
}

struct MicrofacetDistributionSample
{
    float pdf;
    float cosTheta;
    float sinTheta;
    float phi;
};

//float D_GGX(float NdotH, float roughness)
//{
//    float a = NdotH * roughness;
//    float k = roughness / (1.0 - NdotH * NdotH + a * a);
//    return k * k * (1.0 / M_PI);
//}

//// GGX microfacet distribution
//// https://www.cs.cornell.edu/~srm/publications/EGSR07-btdf.html
//// This implementation is based on https://bruop.github.io/ibl/,
////  https://www.tobias-franke.eu/log/2014/03/30/notes_on_importance_sampling.html
//// and https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch20.html
//MicrofacetDistributionSample GGX(float2 xi, float roughness)
//{
//    MicrofacetDistributionSample ggx;

//    // evaluate sampling equations
//    float alpha = roughness * roughness;
//    ggx.cosTheta = saturate(sqrt((1.0 - xi.y) / (1.0 + (alpha * alpha - 1.0) * xi.y)));
//    ggx.sinTheta = sqrt(1.0 - ggx.cosTheta * ggx.cosTheta);
//    ggx.phi = 2.0 * M_PI * xi.x;

//    // evaluate GGX pdf (for half vector)
//    ggx.pdf = D_GGX(ggx.cosTheta, alpha);

//    // Apply the Jacobian to obtain a pdf that is parameterized by l
//    // see https://bruop.github.io/ibl/
//    // Typically you'd have the following:
//    // float pdf = D_GGX(NoH, roughness) * NoH / (4.0 * VoH);
//    // but since V = N => VoH == NoH
//    ggx.pdf /= 4.0;

//    return ggx;
//}

//// NDF
//float D_Ashikhmin(float NdotH, float roughness)
//{
//    float alpha = roughness * roughness;
//    // Ashikhmin 2007, "Distribution-based BRDFs"
//    float a2 = alpha * alpha;
//    float cos2h = NdotH * NdotH;
//    float sin2h = 1.0 - cos2h;
//    float sin4h = sin2h * sin2h;
//    float cot2 = -cos2h / (a2 * sin2h);
//    return 1.0 / (M_PI * (4.0 * a2 + 1.0) * sin4h) * (4.0 * exp(cot2) + sin4h);
//}

//// NDF
//float D_Charlie(float sheenRoughness, float NdotH)
//{
//    sheenRoughness = max(sheenRoughness, 0.000001); //clamp (0,1]
//    float invR = 1.0 / sheenRoughness;
//    float cos2h = NdotH * NdotH;
//    float sin2h = 1.0 - cos2h;
//    return (2.0 + invR) * pow(sin2h, invR * 0.5) / (2.0 * M_PI);
//}

//MicrofacetDistributionSample Charlie(float2 xi, float roughness)
//{
//    MicrofacetDistributionSample charlie;

//    float alpha = roughness * roughness;
//    charlie.sinTheta = pow(xi.y, alpha / (2.0 * alpha + 1.0));
//    charlie.cosTheta = sqrt(1.0 - charlie.sinTheta * charlie.sinTheta);
//    charlie.phi = 2.0 * M_PI * xi.x;

//    // evaluate Charlie pdf (for half vector)
//    charlie.pdf = D_Charlie(alpha, charlie.cosTheta);

//    // Apply the Jacobian to obtain a pdf that is parameterized by l
//    charlie.pdf /= 4.0;

//    return charlie;
//}

MicrofacetDistributionSample Lambertian(float2 xi, float roughness)
{
    MicrofacetDistributionSample lambertian;

    // Cosine weighted hemisphere sampling
    // http://www.pbr-book.org/3ed-2018/Monte_Carlo_Integration/2D_Sampling_with_Multidimensional_Transformations.html#Cosine-WeightedHemisphereSampling
    lambertian.cosTheta = sqrt(1.0 - xi.y);
    lambertian.sinTheta = sqrt(xi.y); // equivalent to `sqrt(1.0 - cosTheta*cosTheta)`;
    lambertian.phi = 2.0 * M_PI * xi.x;

    lambertian.pdf = lambertian.cosTheta / M_PI; // evaluation for solid angle, therefore drop the sinTheta

    return lambertian;
}

// getImportanceSample returns an importance sample direction with pdf in the .w component
float4 getImportanceSample(int sampleIndex, float3 N, float roughness)
{
    // generate a quasi monte carlo point in the unit square [0.1)^2
    float2 xi = hammersley2d(sampleIndex, int(sampleCount));

    MicrofacetDistributionSample importanceSample;

    // generate the points on the hemisphere with a fitting mapping for
    // the distribution (e.g. lambertian uses a cosine importance)
    if (distribution == cLambertian)
    {
        importanceSample = Lambertian(xi, roughness);
    }
    //else if (distribution == cGGX)
    //{
    //    // Trowbridge-Reitz / GGX microfacet model (Walter et al)
    //    // https://www.cs.cornell.edu/~srm/publications/EGSR07-btdf.html
    //    importanceSample = GGX(xi, roughness);
    //}
    //else if (distribution == cCharlie)
    //{
    //    importanceSample = Charlie(xi, roughness);
    //}

    // transform the hemisphere sample to the normal coordinate frame
    // i.e. rotate the hemisphere to the normal direction
    float3 localSpaceDirection = normalize(float3(
        importanceSample.sinTheta * cos(importanceSample.phi),
        importanceSample.sinTheta * sin(importanceSample.phi),
        importanceSample.cosTheta
    ));
    float3x3 TBN = generateTBN(N);
    float3 direction = mul(TBN, localSpaceDirection);

    return float4(direction, importanceSample.pdf);
}

// Mipmap Filtered Samples (GPU Gems 3, 20.4)
// https://developer.nvidia.com/gpugems/gpugems3/part-iii-rendering/chapter-20-gpu-based-importance-sampling
// https://cgg.mff.cuni.cz/~jaroslav/papers/2007-sketch-fis/Final_sap_0073.pdf
float computeLod(float pdf)
{
    // // Solid angle of current sample -- bigger for less likely samples
    // float omegaS = 1.0 / (float(FilterParameters.sampleCoun) * pdf);
    // // Solid angle of texel
    // // note: the factor of 4.0 * UX3D_MATH_PI 
    // float omegaP = 4.0 * UX3D_MATH_PI / (6.0 * float(pFilterParameters.width) * float(pFilterParameters.width));
    // // Mip level is determined by the ratio of our sample's solid angle to a texel's solid angle 
    // // note that 0.5 * log2 is equivalent to log4
    // float lod = 0.5 * log2(omegaS / omegaP);

    // babylon introduces a factor of K (=4) to the solid angle ratio
    // this helps to avoid undersampling the environment map
    // this does not appear in the original formulation by Jaroslav Krivanek and Mark Colbert
    // log4(4) == 1
    // lod += 1.0;

    // We achieved good results by using the original formulation from Krivanek & Colbert adapted to cubemaps

    // https://cgg.mff.cuni.cz/~jaroslav/papers/2007-sketch-fis/Final_sap_0073.pdf
    float lod = 0.5 * log2(6.0 * float(resolution) * float(resolution) / (float(sampleCount) * pdf));

    return lod;
}

float3 filterColor(float3 N)
{
    //return tex.SampleLevel(sam, N, 3.0).rgb;
    float3 color = float3(0.f.xxx);
    float weight = 0.0f;

    for (int i = 0; i < int(sampleCount); i++)
    {
        float4 importanceSample = getImportanceSample(i, N, roughness);

        float3 H = importanceSample.xyz;
        float pdf = importanceSample.w;

        // mipmap filtered samples (GPU Gems 3, 20.4)
        float lod = computeLod(pdf);

        // apply the bias to the lod
        lod += lodBias;

        if (distribution == cLambertian)
        {
            //H.x = -H.x;
            //H.z = -H.z;
            // sample lambertian at a lower resolution to avoid fireflies
            float3 lambertian = tex.SampleLevel(sam, H, lod).rgb;
            
            lambertian = pow(lambertian, float3(0.4545.xxx));

            //// the below operations cancel each other out
            // lambertian *= NdotH; // lamberts law
            // lambertian /= pdf; // invert bias from importance sampling
            // lambertian /= UX3D_MATH_PI; // convert irradiance to radiance https://seblagarde.wordpress.com/2012/01/08/pi-or-not-to-pi-in-game-lighting-equation/

            color += lambertian;
        }
        //else if (distribution == cGGX || distribution == cCharlie)
        //{
        //    // Note: reflect takes incident vector.
        //    float3 V = N;
        //    float3 L = normalize(reflect(-V, H));
        //    float NdotL = dot(N, L);

        //    if (NdotL > 0.0)
        //    {
        //        if (roughness == 0.0)
        //        {
        //            // without this the roughness=0 lod is too high
        //            lod = lodBias;
        //        }
        //        float3 sampleColor = tex.SampleLevel(sam, L, lod).rgb;
        //        color += sampleColor * NdotL;
        //        weight += NdotL;
        //    }
        //}
    }

    if (weight != 0.0f)
    {
        color /= weight;
    }
    else
    {
        color /= float3(sampleCount.xxx);
    }
    
    return color.rgb;
}

float3 uvToXYZ(int face, float2 uv)
{
    if (face == 0) // right
        return float3(1.f, -uv.y, -uv.x);
    else if (face == 1) // left
        return float3(-1.f, -uv.y, uv.x);
    else if (face == 2) // top
        return float3(uv.x, 1.f, +uv.y);
    else if (face == 3) // bottom
        return float3(uv.x, -1.f, -uv.y);
    else if (face == 4) // front
        return float3(uv.x, -uv.y, 1.f);
    else //if(face == 5)
        return float3(-uv.x, -uv.y, -1.f);
}

float2 dirToUV(float3 dir)
{
    return float2(
            0.5f + 0.5f * atan2(dir.z, dir.x) / M_PI,
            1.f - acos(dir.y) / M_PI);
}

void writeFace(int pixel, int face, float3 colorIn)
{
    float4 color = float4(colorIn.rgb, 1.0f);
    
    color = pow(color, float4(2.2.xxx, 1.0f));

    if (face == 0)
        FaceOut_0[pixel] = color;
    else if (face == 1)
        FaceOut_1[pixel] = color;
    else if (face == 2)
        FaceOut_2[pixel] = color;
    else if (face == 3)
        FaceOut_3[pixel] = color;
    else if (face == 4)
        FaceOut_4[pixel] = color;
    else //if(face == 5)
        FaceOut_5[pixel] = color;
} 

[numthreads(16, 16, 3)]
void main(uint3 groupID : SV_GroupID, uint3 threadID : SV_GroupThreadID)
{
 
    const float xcoord = groupID.x * 16 + threadID.x;
    const float ycoord = groupID.y * 16 + threadID.y;
    const int face = groupID.z * 3 + threadID.z;
    const float xinc = 1.0f / (float) resolution;
    const float yinc = 1.0f / (float) resolution;
    const float2 inUV = float2(xcoord * xinc, ycoord * yinc);
    const int currentPixel = xcoord + ycoord * resolution;
    
    float2 newUV = inUV * float(1 << (currentMipLevel));
	 
    newUV = newUV * 2.0 - 1.0;
	
    
    float3 scan = uvToXYZ(face, newUV);
    float3 direction = normalize(scan);
    //direction.y = -direction.y;
    
    writeFace(currentPixel, face, filterColor(direction));
    //float3 colorIn = tex.SampleLevel(sam, direction, 0).rgb;
    //writeFace(currentPixel, face, colorIn);
  
}