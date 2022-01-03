
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

NormalInfo getNormalInfo(VSOut input)
{

    float2 UV = input.UV;
    float3 uv_dx = ddx(float3(UV, 0.0));
    float3 uv_dy = ddy(float3(UV, 0.0));

    float3 t_ = (uv_dy.y * ddx(input.Pos).xyz - uv_dx.y * ddy(input.Pos).xyz) /
        (uv_dx.x * uv_dy.y - uv_dy.x * uv_dx.y);

    float3 n, t, b, ng;

    // Compute geometrical TBN:
    // Trivial TBN computation, present as vertex attribute.
    // Normalize eigenvectors as matrix is linearly interpolated.
    float3x3 TBN = transpose(input.TBN);
    t = normalize(TBN[0]);
    b = normalize(TBN[1]);
    ng = normalize(TBN[2]);

    // For a back-facing surface, the tangential basis vectors are negated.
    if (!input.frontFace)
    {
        t *= -1.0;
        b *= -1.0;
        ng *= -1.0;
    }

    // Compute normals:
    NormalInfo normalInfo;
    normalInfo.ng = ng;
    normalInfo.n = ng;
    if (material.useNormalMap && ginfo.useNormalMap)
    {

        normalInfo.ntex = NormalTexture.Sample(NormalTextureSampler, input.UV).xyz * 2.0 - 1.0;
        //normalInfo.ntex.y = -normalInfo.ntex.y;
        //float u_NormalScale = -1.0;
        //normalInfo.ntex *= float3(u_NormalScale, u_NormalScale, 1.0);
        normalInfo.ntex = normalize(normalInfo.ntex);
        normalInfo.n = normalize(mul(input.TBN, normalInfo.ntex));
        //normalInfo.n = normalize(mul(float3x3(t, b, ng), normalInfo.ntex));
        //return normalize(mul(input.TBN, tangentNormal));

    }

    normalInfo.t = t;
    normalInfo.b = b;
    return normalInfo;
}

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