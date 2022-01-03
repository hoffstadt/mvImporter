
// https://github.com/KhronosGroup/glTF/blob/master/extensions/2.0/Khronos/KHR_lights_punctual/README.md#range-property
float getRangeAttenuation(float range, float distance)
{
    if (range <= 0.0)
    {
        // negative range means unlimited
        return 1.0 / pow(distance, 2.0);
    }
    return max(min(1.0 - pow(distance / range, 4.0), 1.0), 0.0) / pow(distance, 2.0);
}

//float3 getLighIntensity(float3 pointToLight)
//{
//    float rangeAttenuation = 1.0;
//    float spotAttenuation = 1.0;
//    return rangeAttenuation * spotAttenuation * light.intensity * light.color;
//}

float3 getPunctualRadianceClearCoat(float3 clearcoatNormal, float3 v, float3 l, float3 h, float VdotH, float3 f0, float3 f90, float clearcoatRoughness)
{
    float NdotL = clampedDot(clearcoatNormal, l);
    float NdotV = clampedDot(clearcoatNormal, v);
    float NdotH = clampedDot(clearcoatNormal, h);
    return NdotL * BRDF_specularGGX(f0, f90, clearcoatRoughness * clearcoatRoughness, 1.0, VdotH, NdotL, NdotV, NdotH);
}