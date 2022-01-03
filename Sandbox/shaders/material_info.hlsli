
struct MaterialInfo
{
    float ior;
    float perceptualRoughness; // roughness value, as authored by the model creator (input to shader)
    float3 f0; // full reflectance color (n incidence angle)

    float alphaRoughness; // roughness mapped to a more linear change in the roughness (proposed by [2])
    float3 c_diff;

    float3 f90; // reflectance color at grazing angle
    float metallic;

    float3 baseColor;

    float sheenRoughnessFactor;
    float3 sheenColorFactor;

    float3 clearcoatF0;
    float3 clearcoatF90;
    float clearcoatFactor;
    float3 clearcoatNormal;
    float clearcoatRoughness;

    // KHR_materials_specular 
    float specularWeight; // product of specularFactor and specularTexture.a

    float transmissionFactor;

    float thickness;
    float3 attenuationColor;
    float attenuationDistance;
};

//MaterialInfo getIorInfo(MaterialInfo info)
//{
//    info.f0 = vec3(pow((u_Ior - 1.0) / (u_Ior + 1.0), 2.0));
//    info.ior = u_Ior;
//    return info;
//}

float3 getClearcoatNormal(VSOut input, NormalInfo normalInfo)
{
    if (material.useClearcoatNormalMap)
    {
        float3 n = ClearCoatNormalTexture.Sample(ClearCoatNormalTextureSampler, input.UV).rgb * 2.0 - float3(1.0.xxx);
        n *= float3(material.clearcoatNormalScale, material.clearcoatNormalScale, 1.0);
        n = mul(float3x3(normalInfo.t, normalInfo.b, normalInfo.ng), normalize(n));
        //n = mul(input.TBN, normalize(n));
        return n;
    }
    return normalInfo.ng;
}

MaterialInfo getClearCoatInfo(VSOut input, MaterialInfo info, NormalInfo normalInfo)
{
    info.clearcoatFactor = material.clearcoatFactor;
    info.clearcoatRoughness = material.clearcoatRoughnessFactor;
    info.clearcoatF0 = float3(info.f0.xxx);
    info.clearcoatF90 = float3(1.0.xxx);

    if (material.useClearcoatMap)
    {
        float4 clearcoatSample = ClearCoatTexture.Sample(ClearCoatTextureSampler, input.UV);
        info.clearcoatFactor *= clearcoatSample.r;
    }

    if (material.useClearcoatRoughnessMap)
    {
        float4 clearcoatSampleRoughness = ClearCoatRoughnessTexture.Sample(ClearCoatRoughnessTextureSampler, input.UV);
        info.clearcoatRoughness *= clearcoatSampleRoughness.g;
    }

    info.clearcoatNormal = getClearcoatNormal(input, normalInfo);
    info.clearcoatRoughness = clamp(info.clearcoatRoughness, 0.0, 1.0);
    return info;
}

MaterialInfo getMetallicRoughnessInfo(VSOut input, MaterialInfo info)
{
    info.metallic = material.metalness;
    info.perceptualRoughness = material.roughness;

    if (material.useMetalMap)
    {
        // Roughness is stored in the 'g' channel, metallic is stored in the 'b' channel.
        // This layout intentionally reserves the 'r' channel for (optional) occlusion map data
        float4 mrSample = MetalRoughnessTexture.Sample(MetalRoughnessTextureSampler, input.UV);
        //mrSample = pow(mrSample, float4(0.4545f.xxx, 1.0f));
        if (ginfo.useRoughness)
        {
            info.perceptualRoughness *= mrSample.g;
        }
        if (ginfo.useMetalness)
        {
            info.metallic *= mrSample.b;
        }
    }

    // Achromatic f0 based on IOR.
    info.c_diff = lerp(info.baseColor.rgb, float3(0.0.xxx), info.metallic);
    info.f0 = lerp(info.f0, info.baseColor.rgb, info.metallic);
    return info;
}

float4 getBaseColor(VSOut input)
{
    float4 base_color = float4(1.0.xxxx);
    
    base_color = material.albedo;
    
    if (material.useAlbedoMap && ginfo.useAlbedo)
    {
        base_color *= sRGBToLinear(AlbedoTexture.Sample(AlbedoTextureSampler, input.UV).rgba);

    }
    
    return base_color;
}