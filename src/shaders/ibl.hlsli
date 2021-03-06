
float3 getDiffuseLight(float3 n)
{
    n.x = -n.x;
    //n.z = -n.z;

    float3 color = ginfo.ambientColor;

#ifdef USE_IBL
    color = IrradianceMap.Sample(IrradianceMapSampler, n).rgb;
    color = pow(abs(color), float3(0.4545.xxx));
#endif

    return color;
}

float4 getSpecularSample(float3 reflection, float lod)
{
    reflection.x = -reflection.x;
    //reflection.z = -reflection.z;

    float4 color = float4(1.0f.xxxx);

#ifdef USE_IBL
    color = SpecularMap.SampleLevel(SpecularMapSampler, reflection, lod);
#endif
    //color = pow(abs(color), float4(0.4545.xxxx));
    return color;
}

#ifdef USE_IBL
float3 getIBLRadianceGGX(float3 n, float3 v, float roughness, float3 F0, float specularWeight)
{
    float NdotV = clampedDot(n, v);
    float lod = roughness * float(miplevelsoutput - 1); // mip count is 10
    float3 reflection = normalize(reflect(-v, n));

    float2 brdfSamplePoint = clamp(float2(NdotV, roughness), float2(0.0, 0.0), float2(1.0, 1.0));
    float2 f_ab = u_GGXLUT.Sample(u_GGXLUTSampler, brdfSamplePoint).rg;
    //f_ab = pow(f_ab, float2(0.4545.xx));
    float4 specularSample = getSpecularSample(reflection, lod);

    float3 specularLight = specularSample.rgb;

    // see https://bruop.github.io/ibl/#single_scattering_results at Single Scattering Results
    // Roughness dependent fresnel, from Fdez-Aguera
    float3 Fr = max(float3((1.0 - roughness).xxx), F0) - F0;
    float3 k_S = F0 + Fr * pow(1.0 - NdotV, 5.0);
    float3 FssEss = k_S * f_ab.x + f_ab.y;

    return specularWeight * specularLight * FssEss;
}
#endif

#ifdef MATERIAL_TRANSMISSION
vec3 getTransmissionSample(vec2 fragCoord, float roughness, float ior)
{
    float framebufferLod = log2(float(u_TransmissionFramebufferSize.x)) * applyIorToRoughness(roughness, ior);
    float3 transmittedLight = textureLod(u_TransmissionFramebufferSampler, fragCoord.xy, framebufferLod).rgb;
    return transmittedLight;
}
#endif


#ifdef MATERIAL_TRANSMISSION
float3 getIBLVolumeRefraction(float3 n, float3 v, float perceptualRoughness, float3 baseColor, float3 f0, float3 f90,
    float3 position, float4x4 modelMatrix, float4x4 viewMatrix, float4x4 projMatrix, float ior, float thickness, float3 attenuationColor, float attenuationDistance)
{
    float3 transmissionRay = getVolumeTransmissionRay(n, v, thickness, ior, modelMatrix);
    float3 refractedRayExit = position + transmissionRay;

    // Project refracted vector on the framebuffer, while mapping to normalized device coordinates.
    float4 ndcPos = projMatrix * viewMatrix * float4(refractedRayExit, 1.0);
    float2 refractionCoords = ndcPos.xy / ndcPos.w;
    refractionCoords += 1.0;
    refractionCoords /= 2.0;

    // Sample framebuffer to get pixel the refracted ray hits.
    float3 transmittedLight = getTransmissionSample(refractionCoords, perceptualRoughness, ior);

    float3 attenuatedColor = applyVolumeAttenuation(transmittedLight, length(transmissionRay), attenuationColor, attenuationDistance);

    // Sample GGX LUT to get the specular component.
    float NdotV = clampedDot(n, v);
    float2 brdfSamplePoint = clamp(vec2(NdotV, perceptualRoughness), float2(0.0, 0.0), float2(1.0, 1.0));
    float2 brdf = u_GGXLUT.Sample(u_GGXLUTSampler, brdfSamplePoint).rg;
    float3 specularColor = f0 * brdf.x + f90 * brdf.y;

    return (1.0 - specularColor) * attenuatedColor * baseColor;
}
#endif


#ifdef USE_IBL
//specularWeight is
//introduced withKHR_materials_specular
float3 getIBLRadianceLambertian(float3 n, float3 v, float roughness, float3 diffuseColor, float3 F0, float specularWeight)
{
    float NdotV = clampedDot(n, v);
    float2 brdfSamplePoint = clamp(float2(NdotV, roughness), float2(0.0, 0.0), float2(1.0, 1.0));
    float2 f_ab = u_GGXLUT.Sample(u_GGXLUTSampler, brdfSamplePoint).rg;
    //f_ab = pow(f_ab, float2(0.4545.xx));
    float3 irradiance = getDiffuseLight(n);

    // see https://bruop.github.io/ibl/#single_scattering_results at Single Scattering Results
    // Roughness dependent fresnel, from Fdez-Aguera

    float3 Fr = max(float3((1.0 - roughness).xxx), F0) - F0;
    float3 k_S = F0 + Fr * pow(1.0 - NdotV, 5.0);
    float3 FssEss = specularWeight * k_S * f_ab.x + f_ab.y; // <--- GGX / specular light contribution (scale it down if the specularWeight is low)

    // Multiple scattering, from Fdez-Aguera
    float Ems = (1.0 - (f_ab.x + f_ab.y));
    float3 F_avg = specularWeight * (F0 + (1.0 - F0) / 21.0);
    float3 FmsEms = Ems * FssEss * F_avg / (1.0 - F_avg * Ems);
    float3 k_D = diffuseColor * (1.0 - FssEss + FmsEms); // we use +FmsEms as indicated by the formula in the blog post (might be a typo in the implementation)

    return (FmsEms + k_D) * irradiance;
}
#endif


//float3 getIBLRadianceCharlie(float3 n, float3 v, float sheenRoughness, float3 sheenColor)
//{
//    float NdotV = clampedDot(n, v);
//    float lod = sheenRoughness * float(u_MipCount - 1);
//    float3 reflection = normalize(reflect(-v, n));

//    float2 brdfSamplePoint = clamp(float2(NdotV, sheenRoughness), float2(0.0, 0.0), float2(1.0, 1.0));
//    float brdf = texture(u_CharlieLUT, brdfSamplePoint).b;
//    float4 sheenSample = getSheenSample(reflection, lod);

//    float3 sheenLight = sheenSample.rgb;
//    return sheenLight * sheenColor * brdf;
//}