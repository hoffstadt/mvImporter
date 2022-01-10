
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

#ifdef MATERIAL_SPECULARGLOSSINESS
MaterialInfo getSpecularGlossinessInfo(MaterialInfo info)
{
    info.f0 = u_SpecularFactor;
    info.perceptualRoughness = u_GlossinessFactor;

#ifdef HAS_SPECULAR_GLOSSINESS_MAP
    vec4 sgSample = texture(u_SpecularGlossinessSampler, getSpecularGlossinessUV());
    info.perceptualRoughness *= sgSample.a; // glossiness to roughness
    info.f0 *= sgSample.rgb; // specular
#endif // ! HAS_SPECULAR_GLOSSINESS_MAP

    info.perceptualRoughness = 1.0 - info.perceptualRoughness; // 1 - glossiness
    info.c_diff = info.baseColor.rgb * (1.0 - max(max(info.f0.r, info.f0.g), info.f0.b));
    return info;
}
#endif

NormalInfo getNormalInfo(VSOut input)
{

    float2 UV = input.UV0;
    float3 uv_dx = ddx(float3(UV, 0.0));
    float3 uv_dy = ddy(float3(UV, 0.0));

    float3 t_ = (uv_dy.y * ddx(input.Pos).xyz - uv_dx.y * ddy(input.Pos).xyz) /
        (uv_dx.x * uv_dy.y - uv_dy.x * uv_dx.y);

    float3 n, t, b, ng;

    // Compute geometrical TBN:
#ifdef HAS_NORMALS
#ifdef HAS_TANGENTS
    // Trivial TBN computation, present as vertex attribute.
    // Normalize eigenvectors as matrix is linearly interpolated.
    float3x3 TBN = transpose(input.TBN);
    t = normalize(TBN[0]);
    b = normalize(TBN[1]);
    ng = normalize(TBN[2]);
#else
    // Normals are either present as vertex attributes or approximated.
    ng = normalize(input.v_Normal);
    t = normalize(t_ - ng * dot(ng, t_));
    b = cross(ng, t);
#endif
#else
    ng = normalize(cross(ddx(input.Pos), ddy(input.Pos)));
    t = normalize(t_ - ng * dot(ng, t_));
    b = cross(ng, t);
#endif

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
    //normalInfo.n = ng;
#ifdef HAS_NORMAL_MAP
    normalInfo.ntex = NormalTexture.Sample(NormalTextureSampler, input.UV0).xyz * 2.0 - 1.0;
    if (!input.frontFace) // backface
    {
        normalInfo.ntex.x = -normalInfo.ntex.x;
        normalInfo.ntex.z = -normalInfo.ntex.z;
        //normalInfo.ntex.y = -normalInfo.ntex.y;
    }
    else // front face
    {
        normalInfo.ntex.y = -normalInfo.ntex.y;
    }

    normalInfo.ntex = normalize(normalInfo.ntex);
    normalInfo.n = normalize(mul(input.TBN, normalInfo.ntex));
    //normalInfo.n = normalize(mul(float3x3(t, b, ng), normalInfo.ntex));
    //return normalize(mul(input.TBN, tangentNormal));
#else
    normalInfo.n = ng;
#endif

    normalInfo.t = t;
    normalInfo.b = b;
    return normalInfo;
}

#ifdef MATERIAL_CLEARCOAT
float3 getClearcoatNormal(VSOut input, NormalInfo normalInfo)
{
#ifdef HAS_CLEARCOAT_NORMAL_MAP
        float3 n = ClearCoatNormalTexture.Sample(ClearCoatNormalTextureSampler, input.UV0).rgb * 2.0 - float3(1.0.xxx);
        n *= float3(material.clearcoatNormalScale, material.clearcoatNormalScale, 1.0);
        n = mul(float3x3(normalInfo.t, normalInfo.b, normalInfo.ng), normalize(n));
        //n = mul(input.TBN, normalize(n));
        return n;
#else
    return normalInfo.ng;
#endif
}
#endif

#ifdef MATERIAL_TRANSMISSION
MaterialInfo getTransmissionInfo(VSOut input, MaterialInfo info)
{
    info.transmissionFactor = input.transmissionFactor;

#ifdef HAS_TRANSMISSION_MAP
    float4 transmissionSample = TransmissionTexture.Sample(TransmissionSampler, input.UV0);
    info.transmissionFactor *= transmissionSample.r;
#endif
    return info;
}
#endif


#ifdef MATERIAL_VOLUME
MaterialInfo getVolumeInfo(VSOut input, MaterialInfo info)
{
    info.thickness = input.thicknessFactor;
    info.attenuationColor = input.attenuationColor;
    info.attenuationDistance = input.attenuationDistance;

#ifdef HAS_THICKNESS_MAP
    float4 thicknessSample = ThicknessTexture.Sample(ThicknessTextureSampler, input.UV0));
    info.thickness *= thicknessSample.g;
#endif
    return info;
}
#endif

#ifdef MATERIAL_CLEARCOAT
MaterialInfo getClearCoatInfo(VSOut input, MaterialInfo info, NormalInfo normalInfo)
{
    info.clearcoatFactor = material.clearcoatFactor;
    info.clearcoatRoughness = material.clearcoatRoughnessFactor;
    info.clearcoatF0 = float3(info.f0.xxx);
    info.clearcoatF90 = float3(1.0.xxx);

#ifdef HAS_CLEARCOAT_MAP
    float4 clearcoatSample = ClearCoatTexture.Sample(ClearCoatTextureSampler, input.UV0);
    info.clearcoatFactor *= clearcoatSample.r;
#endif

#ifdef HAS_CLEARCOAT_ROUGHNESS_MAP
    float4 clearcoatSampleRoughness = ClearCoatRoughnessTexture.Sample(ClearCoatRoughnessTextureSampler, input.UV0);
    info.clearcoatRoughness *= clearcoatSampleRoughness.g;
#endif

    info.clearcoatNormal = getClearcoatNormal(input, normalInfo);
    info.clearcoatRoughness = clamp(info.clearcoatRoughness, 0.0, 1.0);
    return info;
}
#endif

#ifdef MATERIAL_METALLICROUGHNESS
MaterialInfo getMetallicRoughnessInfo(VSOut input, MaterialInfo info)
{

#ifdef HAS_METALLIC_ROUGHNESS_MAP
    // Roughness is stored in the 'g' channel, metallic is stored in the 'b' channel.
    // This layout intentionally reserves the 'r' channel for (optional) occlusion map data
    float4 mrSample = MetalRoughnessTexture.Sample(MetalRoughnessTextureSampler, input.UV0);
    info.perceptualRoughness *= mrSample.g;
    info.metallic *= mrSample.b;
#endif

    // Achromatic f0 based on IOR.
    info.c_diff = lerp(info.baseColor.rgb, float3(0.0.xxx), info.metallic);
    info.f0 = lerp(info.f0, info.baseColor.rgb, info.metallic);
    return info;
}
#endif

#ifdef MATERIAL_SHEEN
MaterialInfo getSheenInfo(VSOut input, MaterialInfo info)
{
    info.sheenColorFactor = material.sheenColorFactor;
    info.sheenRoughnessFactor = material.sheenRoughnessFactor;

#ifdef HAS_SHEEN_COLOR_MAP
    float4 sheenColorSample = SheenColorTexture.Sample(SheenColorTextureSampler, input.UV0);
    info.sheenColorFactor *= sheenColorSample.rgb;
#endif

#ifdef HAS_SHEEN_ROUGHNESS_MAP
    float4 sheenRoughnessSample = SheenRoughnessTexture.Sample(SheenRoughnessTextureSampler, input.UV0);
    info.sheenRoughnessFactor *= sheenRoughnessSample.a;
#endif
    return info;
}
#endif

float4 getVertexColor(VSOut input)
{
    float4 color = float4(1.0.xxxx);

#ifdef HAS_VERTEX_COLOR_0_VEC3
    color.rgb = input.v_color0.rgb;
#endif
#ifdef HAS_VERTEX_COLOR_0_VEC4
    color = input.v_color0;
#endif

    return color;
}

float4 getBaseColor(VSOut input)
{
    float4 base_color = float4(1.0.xxxx);
    
#if defined(MATERIAL_SPECULARGLOSSINESS)
    baseColor = u_DiffuseFactor;
#elif defined(MATERIAL_METALLICROUGHNESS)
    base_color = material.albedo;
#endif
   
#if defined(MATERIAL_SPECULARGLOSSINESS) && defined(HAS_DIFFUSE_MAP)
    baseColor *= texture(u_DiffuseSampler, getDiffuseUV());
#elif defined(MATERIAL_METALLICROUGHNESS) && defined(HAS_BASE_COLOR_MAP)
    base_color *= sRGBToLinear(AlbedoTexture.Sample(AlbedoTextureSampler, input.UV0).rgba);
    //base_color *= AlbedoTexture.Sample(AlbedoTextureSampler, input.UV0).rgba;
#endif
    
    return base_color * getVertexColor(input);
}

#ifdef MATERIAL_IOR
MaterialInfo getIorInfo(MaterialInfo info)
{
    info.f0 = float3(pow((material.ior - 1.0) / (material.ior + 1.0), 2.0).xxx);
    info.ior = input.ior;
    return info;
}
#endif