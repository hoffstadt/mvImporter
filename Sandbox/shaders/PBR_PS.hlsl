
static const int miplevelsoutput = 7;

struct mvPointLight
{
    float3 viewLightPos;
    //-------------------------- ( 16 bytes )

    float3 diffuseColor;
    float diffuseIntensity;
    //-------------------------- ( 16 bytes )

    float attConst;
    float attLin;
    float attQuad;
    //-------------------------- ( 16 bytes )

    //-------------------------- ( 4*16 = 64 bytes )
};

struct mvMaterial
{
    float4 albedo;
    //-------------------------- ( 16 bytes )
    
    float metalness;
    float roughness;
    float radiance;
    float fresnel;
    //-------------------------- ( 16 bytes )
     
    float3 emisiveFactor;
    float occlusionStrength;
    //-------------------------- ( 16 bytes )
    
    bool doubleSided;
    float alphaCutoff;
    float clearcoatFactor;
    float clearcoatRoughnessFactor;
    //-------------------------- ( 16 bytes )

    float normalScale;
    float clearcoatNormalScale;
    float transmissionFactor;
    float thicknessFactor;
    //-------------------------- ( 16 bytes )

    float4 attenuationColor;
    //-------------------------- ( 16 bytes )

    float3 sheenColorFactor;
    float sheenRoughnessFactor;
    //-------------------------- ( 16 bytes )

    float attenuationDistance;
    float ior;
    //-------------------------- ( 16 bytes )

};

struct mvDirectionalLight
{
    float diffuseIntensity;
    float3 viewLightDir;
    //-------------------------- ( 16 bytes )
    
    float3 diffuseColor;
    //-------------------------- ( 16 bytes )
    
    //-------------------------- ( 2*16 = 32 bytes )
};

struct mvGlobalInfo
{

    float3 ambientColor;
    int pcfRange;
    //-------------------------- ( 16 bytes )
    
    float3 camPos;
    //-------------------------- ( 16 bytes )
    
    float4x4 projection;
    float4x4 model;
    float4x4 view;
    //-------------------------- ( 192 bytes )
    
    //-------------------------- ( 124 bytes )
    
};

//-----------------------------------------------------------------------------
// textures & samplers
//-----------------------------------------------------------------------------
#ifdef HAS_BASE_COLOR_MAP
Texture2D AlbedoTexture : register(t0);
SamplerState AlbedoTextureSampler : register(s0);
#endif

#ifdef HAS_NORMAL_MAP
Texture2D NormalTexture : register(t1);
SamplerState NormalTextureSampler : register(s1);
#endif

#ifdef HAS_METALLIC_ROUGHNESS_MAP
Texture2D MetalRoughnessTexture : register(t2);
SamplerState MetalRoughnessTextureSampler : register(s2);
#endif

#ifdef HAS_EMISSIVE_MAP
Texture2D EmmissiveTexture : register(t3);
SamplerState EmmissiveTextureSampler : register(s3);
#endif

#ifdef HAS_OCCLUSION_MAP
Texture2D OcclusionTexture : register(t4);
SamplerState OcclusionTextureSampler : register(s4);
#endif

#ifdef USE_PUNCTUAL
#ifdef SHADOWS_DIRECTIONAL
Texture2D DirectionalShadowMap : register(t5);
SamplerComparisonState DirectionalShadowMapSampler : register(s5);
#endif
#ifdef SHADOWS_OMNI
TextureCube ShadowMap : register(t6);
SamplerComparisonState ShadowMapSampler : register(s6);
#endif
#endif

#ifdef USE_IBL
TextureCube IrradianceMap : register(t7);
TextureCube SpecularMap : register(t8);
Texture2D u_GGXLUT : register(t9);
SamplerState IrradianceMapSampler : register(s7);
SamplerState SpecularMapSampler : register(s8);
SamplerState u_GGXLUTSampler : register(s9);
#endif

#ifdef HAS_CLEARCOAT_MAP
Texture2D ClearCoatTexture : register(t10);
SamplerState ClearCoatTextureSampler : register(s10);
#endif

#ifdef HAS_CLEARCOAT_ROUGHNESS_MAP
Texture2D ClearCoatRoughnessTexture : register(t11);
SamplerState ClearCoatRoughnessTextureSampler : register(s11);
#endif

#ifdef HAS_CLEARCOAT_NORMAL_MAP
Texture2D ClearCoatNormalTexture : register(t12);
SamplerState ClearCoatNormalTextureSampler : register(s12);
#endif

#ifdef HAS_SHEEN_COLOR_MAP
Texture2D SheenColorTexture : register(t13);
SamplerState SheenColorTextureSampler : register(s13);
#endif

#ifdef HAS_SHEEN_ROUGHNESS_MAP
Texture2D SheenRoughnessTexture: register(t14);
SamplerState SheenRoughnessTextureSampler : register(s14);
#endif

#ifdef HAS_TRANSMISSION_MAP
//Texture2D TransmissionTexture : register(t13);
//SamplerState TransmissionSampler : register(s13);
#endif

#ifdef HAS_THICKNESS_MAP
//Texture2D ThicknessTexture : register(t14);
//SamplerState ThicknessTextureSampler : register(s14);
#endif

//-----------------------------------------------------------------------------
// constant buffers
//-----------------------------------------------------------------------------
cbuffer mvPointLightCBuf       : register(b0) { mvPointLight PointLight; };
cbuffer mvMaterialCBuf         : register(b1) { mvMaterial material; };
cbuffer mvDirectionalLightCBuf : register(b2) { mvDirectionalLight DirectionalLight; };
cbuffer mvGlobalCBuf           : register(b3) { mvGlobalInfo ginfo; };

struct VSOut
{

    float4 Pos : SV_Position;

#ifdef HAS_NORMALS
#ifdef HAS_TANGENTS
    float3x3 TBN : TangentBasis;
#else
    float3 v_Normal: NORMAL0;
#endif
#endif

#ifdef HAS_VERTEX_COLOR_0_VEC3
    float3 v_color0 : COLOR0;
#endif

#ifdef HAS_VERTEX_COLOR_0_VEC4
    float4 v_color0 : COLOR0;
#endif

#ifdef HAS_VERTEX_COLOR_1_VEC3
    float3 v_color1 : COLOR1;
#endif

#ifdef HAS_VERTEX_COLOR_1_VEC4
    float4 v_color1 : COLOR1;
#endif

    float3 WorldPos : POSITION0;
    float3 WorldNormal : NORMAL1;
    float2 UV0 : TEXCOORD0;
    float2 UV1 : TEXCOORD1;
    float4 dshadowWorldPos : dshadowPosition; // light pos
    float4 oshadowWorldPos : oshadowPosition; // light pos
    bool frontFace : SV_IsFrontFace;

};

#include <tonemapping.hlsli>
#include <functions.hlsli>
#include <brdf.hlsli>
#include <punctual.hlsli>
#include <ibl.hlsli>
#include <material_info.hlsli>

#ifdef SHADOWS_OMNI
static const float zf = 100.0f;
static const float zn = 0.5f;
static const float c1 = (zf + zn) / (zf - zn);
static const float c0 = -(2 * zn * zf) / (zf - zn);
static const int PCFRANGE = 2;

float CalculateShadowDepth(const in float4 shadowPos)
{
    // get magnitudes for each basis component
    const float3 m = abs(shadowPos).xyz;
    // get the length in the dominant axis
    // (this correlates with shadow map face and derives comparison depth)
    const float major = max(m.x, max(m.y, m.z));
    // converting from distance in shadow light space to projected depth
    return (c1 * major + c0) / major;
}

float Shadow(const in float4 shadowPos, uniform TextureCube map, uniform SamplerComparisonState smplr)
{
    return map.SampleCmpLevelZero(smplr, shadowPos.xyz, CalculateShadowDepth(shadowPos));
}
#endif

#ifdef SHADOWS_DIRECTIONAL
float filterPCF(const in float2 spos, float depthCheck)
{
    float shadowLevel = 0.0f;
    float texWidth;
    float texHeight;
    float scale = 1.5;
    DirectionalShadowMap.GetDimensions(texWidth, texHeight);
    float dx = scale * 1.0 / texWidth;
    float dy = scale * 1.0 / texHeight;
    
    int count = 0;
    [loop]
    for (int x = -ginfo.pcfRange; x <= ginfo.pcfRange; x++)
    {
        [loop]
        for (int y = -ginfo.pcfRange; y <= ginfo.pcfRange; y++)
        {
            shadowLevel += DirectionalShadowMap.SampleCmpLevelZero(DirectionalShadowMapSampler, float2(spos.x + dx * x, spos.y + dy * y), depthCheck);
            count++;
        }
    }
    return shadowLevel / count;
}
#endif

float4 main(VSOut input) : SV_Target
{
    float4 finalColor;
    float4 baseColor = getBaseColor(input);

    
#if ALPHAMODE == 0
    baseColor.a = 1.0;
#endif

#ifdef MATERIAL_UNLIT
#if ALPHAMODE == 1
    if (baseColor.a < material.alphaCutoff)
    {
        discard;
    }
#endif
    finalColor = (float4(linearTosRGB(baseColor.rgb), baseColor.a));
#endif

    float3 v = normalize(ginfo.camPos - input.WorldPos);
    NormalInfo normalInfo = getNormalInfo(input);
    
    float3 n = normalInfo.n;
    float3 t = normalInfo.t;
    float3 b = normalInfo.b;

    float NdotV = clampedDot(n, v);
    float TdotV = clampedDot(t, v);
    float BdotV = clampedDot(b, v);
    
    MaterialInfo materialInfo;
    materialInfo.baseColor = baseColor.rgb;
    materialInfo.metallic = material.metalness;
    materialInfo.perceptualRoughness = material.roughness;
    materialInfo.alphaRoughness = 0.0;
    materialInfo.c_diff = float3(0.00.xxx);
    materialInfo.f90 = float3(0.0.xxx);
    materialInfo.sheenRoughnessFactor = 0.0f;
    materialInfo.sheenColorFactor = float3(0.0.xxx);
    materialInfo.clearcoatF0 = float3(0.0.xxx);
    materialInfo.clearcoatF90 = float3(0.0.xxx);
    materialInfo.clearcoatNormal = float3(0.0.xxx);
    materialInfo.attenuationColor = float3(0.0.xxx);
    materialInfo.clearcoatFactor = 0.0;
    materialInfo.clearcoatRoughness = 0.0;
    materialInfo.specularWeight = 0.0; // product of specularFactor and specularTexture.a
    materialInfo.transmissionFactor = 0.0;
    materialInfo.thickness = 0.0;
    materialInfo.attenuationDistance = 0.0;
    
    // The default index of refraction of 1.5 yields a dielectric normal incidence reflectance of 0.04.
    materialInfo.ior = 1.5;
    materialInfo.f0 = float3(0.04.xxx);

    materialInfo.specularWeight = 1.0;
    
#ifdef MATERIAL_IOR
    materialInfo = getIorInfo(materialInfo);
#endif

#ifdef MATERIAL_SPECULARGLOSSINESS
    materialInfo = getSpecularGlossinessInfo(materialInfo);
#endif

#ifdef MATERIAL_METALLICROUGHNESS
    materialInfo = getMetallicRoughnessInfo(input, materialInfo);
#endif

#ifdef MATERIAL_SHEEN
    materialInfo = getSheenInfo(input, materialInfo);
#endif

#ifdef MATERIAL_CLEARCOAT
    materialInfo = getClearCoatInfo(input, materialInfo, normalInfo);
#endif

#ifdef MATERIAL_SPECULAR
    materialInfo = getSpecularInfo(materialInfo);
#endif

#ifdef MATERIAL_TRANSMISSION
    materialInfo = getTransmissionInfo(materialInfo);
#endif

#ifdef MATERIAL_VOLUME
    materialInfo = getVolumeInfo(materialInfo);
#endif
    
    materialInfo.perceptualRoughness = clamp(materialInfo.perceptualRoughness, 0.0, 1.0);
    materialInfo.metallic = clamp(materialInfo.metallic, 0.0, 1.0);

    // Roughness is authored as perceptual roughness; as is convention,
    // convert to material roughness by squaring the perceptual roughness.
    materialInfo.alphaRoughness = materialInfo.perceptualRoughness * materialInfo.perceptualRoughness;

    // Compute reflectance.
    float reflectance = max(max(materialInfo.f0.r, materialInfo.f0.g), materialInfo.f0.b);

    // Anything less than 2% is physically impossible and is instead considered to be shadowing. Compare to "Real-Time-Rendering" 4th editon on page 325.
    materialInfo.f90 = float3(1.0.xxx);

    // LIGHTING
    float3 f_specular = float3(0.0.xxx);
    float3 f_diffuse = float3(0.0.xxx);
    float3 f_emissive = float3(0.0.xxx);
    float3 f_clearcoat = float3(0.0.xxx);
    float3 f_sheen = float3(0.0.xxx);
    float3 f_transmission = float3(0.0.xxx);

    float albedoSheenScaling = 1.0;

    // Calculate lighting contribution from image based lighting source (IBL)
#ifdef USE_IBL
    f_specular += getIBLRadianceGGX(n, v, materialInfo.perceptualRoughness, materialInfo.f0, materialInfo.specularWeight);
    f_diffuse += getIBLRadianceLambertian(n, v, materialInfo.perceptualRoughness, materialInfo.c_diff, materialInfo.f0, materialInfo.specularWeight);

#ifdef MATERIAL_CLEARCOAT
    f_clearcoat += getIBLRadianceGGX(materialInfo.clearcoatNormal, v, materialInfo.clearcoatRoughness, materialInfo.clearcoatF0, 1.0);
#endif

#ifdef MATERIAL_SHEEN
    f_sheen += getIBLRadianceCharlie(n, v, materialInfo.sheenRoughnessFactor, materialInfo.sheenColorFactor);
#endif
#endif

#if (defined(MATERIAL_TRANSMISSION) || defined(MATERIAL_VOLUME)) && (defined(USE_PUNCTUAL) || defined(USE_IBL))
    f_transmission += materialInfo.transmissionFactor * getIBLVolumeRefraction(
        n, v,
        materialInfo.perceptualRoughness,
        materialInfo.baseColor, materialInfo.f0, materialInfo.f90,
        v_Position, ginfo.model, ginfo.view, ginfo.projection,
        materialInfo.ior, materialInfo.thickness, materialInfo.attenuationColor, materialInfo.attenuationDistance);
#endif

    float ao = 1.0;
    // Apply optional PBR terms for additional (optional) shading
#ifdef HAS_OCCLUSION_MAP
    ao = OcclusionTexture.Sample(OcclusionTextureSampler, input.UV0).r;
        
    f_diffuse = lerp(f_diffuse, f_diffuse * ao, material.occlusionStrength);
    // apply ambient occlusion to all lighting that is not punctual
    f_specular = lerp(f_specular, f_specular * ao, material.occlusionStrength);
    f_sheen = lerp(f_sheen, f_sheen * ao, material.occlusionStrength);
    f_clearcoat = lerp(f_clearcoat, f_clearcoat * ao, material.occlusionStrength);
#endif
    
#ifdef USE_PUNCTUAL
    {
        
        //-----------------------------------------------------------------------------
        // point light
        //-----------------------------------------------------------------------------
        input.oshadowWorldPos.z = -input.oshadowWorldPos.z;
        float shadowLevel = 1.0f;
        float3 pointToLight = PointLight.viewLightPos - input.WorldPos;
#ifdef SHADOWS_OMNI
            shadowLevel = Shadow(input.oshadowWorldPos, ShadowMap, ShadowMapSampler);
#endif
        
        // BSTF
        float3 l = normalize(pointToLight); // Direction from surface point to light
        float3 h = normalize(l + v); // Direction of the vector between l and v, called halfway vector
        float NdotL = clampedDot(n, l);
        float NdotV = clampedDot(n, v);
        float NdotH = clampedDot(n, h);
        float LdotH = clampedDot(l, h);
        float VdotH = clampedDot(v, h);
        if (NdotL > 0.0 || NdotV > 0.0)
        {

        // Calculation of analytical light
        // https://github.com/KhronosGroup/glTF/tree/master/specification/2.0#acknowledgments AppendixB
        //float3 intensity = getLighIntensity(pointToLight);
        float3 intensity = DirectionalLight.diffuseIntensity * DirectionalLight.diffuseColor;
        f_diffuse += shadowLevel * intensity * NdotL * BRDF_lambertian(materialInfo.f0, materialInfo.f90, materialInfo.c_diff, materialInfo.specularWeight, VdotH);
        f_specular += shadowLevel * intensity * NdotL * BRDF_specularGGX(materialInfo.f0, materialInfo.f90, materialInfo.alphaRoughness, materialInfo.specularWeight, VdotH, NdotL, NdotV, NdotH);
        f_clearcoat += shadowLevel * intensity * getPunctualRadianceClearCoat(materialInfo.clearcoatNormal, v, l, h, VdotH,
            materialInfo.clearcoatF0, materialInfo.clearcoatF90, materialInfo.clearcoatRoughness);
#ifdef MATERIAL_SHEEN
        f_sheen += shadowLevel *intensity * getPunctualRadianceSheen(materialInfo.sheenColorFactor, materialInfo.sheenRoughnessFactor, NdotL, NdotV, NdotH);
        albedoSheenScaling = min(1.0 - max3(materialInfo.sheenColorFactor) * albedoSheenScalingLUT(NdotV, materialInfo.sheenRoughnessFactor),
            1.0 - max3(materialInfo.sheenColorFactor) * albedoSheenScalingLUT(NdotL, materialInfo.sheenRoughnessFactor));
#endif

#ifdef MATERIAL_CLEARCOAT
        f_clearcoat += shadowLevel * intensity * getPunctualRadianceClearCoat(materialInfo.clearcoatNormal, v, l, h, VdotH,
            materialInfo.clearcoatF0, materialInfo.clearcoatF90, materialInfo.clearcoatRoughness);
#endif

        }

        // BDTF
#ifdef MATERIAL_TRANSMISSION
        // If the light ray travels through the geometry, use the point it exits the geometry again.
        // That will change the angle to the light source, if the material refracts the light ray.
        vec3 transmissionRay = getVolumeTransmissionRay(n, v, materialInfo.thickness, materialInfo.ior, u_ModelMatrix);
        pointToLight -= transmissionRay;
        l = normalize(pointToLight);

        vec3 intensity = getLighIntensity(light, pointToLight);
        vec3 transmittedLight = intensity * getPunctualRadianceTransmission(n, v, l, materialInfo.alphaRoughness, materialInfo.f0, materialInfo.f90, materialInfo.baseColor, materialInfo.ior);

#ifdef MATERIAL_VOLUME
        transmittedLight = applyVolumeAttenuation(transmittedLight, length(transmissionRay), materialInfo.attenuationColor, materialInfo.attenuationDistance);
#endif

        f_transmission += materialInfo.transmissionFactor * transmittedLight;
#endif
    }
 
    {
        
        //-----------------------------------------------------------------------------
        // directional light
        //-----------------------------------------------------------------------------
        float2 projectTexCoord;
        float lightDepthValue;
        float shadowLevel = 1.0f;
        float3 pointToLight = -DirectionalLight.viewLightDir;
        
        // Calculate the projected texture coordinates.
        projectTexCoord.x = 0.5f * input.dshadowWorldPos.x / input.dshadowWorldPos.w + 0.5f;
        projectTexCoord.y = -0.5f * input.dshadowWorldPos.y / input.dshadowWorldPos.w + 0.5f;

        // BSTF
        float3 l = normalize(pointToLight);   // Direction from surface point to light
        float3 h = normalize(l + v);          // Direction of the vector between l and v, called halfway vector
        float NdotL = clampedDot(n, l);
        float NdotV = clampedDot(n, v);
        float NdotH = clampedDot(n, h);
        float LdotH = clampedDot(l, h);
        float VdotH = clampedDot(v, h);
        if (NdotL > 0.0 || NdotV > 0.0)
        {
            
#ifdef SHADOWS_DIRECTIONAL
            // Calculate the depth of the light.
            lightDepthValue = input.dshadowWorldPos.z / input.dshadowWorldPos.w;
            
            // Determine if the projected coordinates are in the 0 to 1 range.  If so then this pixel is in the view of the light.
            if ((saturate(projectTexCoord.x) == projectTexCoord.x) && (saturate(projectTexCoord.y) == projectTexCoord.y))
            {
                
                if (ginfo.pcfRange > 0)
                {
                    shadowLevel = filterPCF(projectTexCoord, lightDepthValue);
                }
                else
                {
                    shadowLevel = DirectionalShadowMap.SampleCmpLevelZero(DirectionalShadowMapSampler, projectTexCoord, lightDepthValue);
                }

                
            }
#endif
          
            // Calculation of analytical light
            // https://github.com/KhronosGroup/glTF/tree/master/specification/2.0#acknowledgments AppendixB
            //float3 intensity = getLighIntensity(pointToLight);
            float3 intensity = DirectionalLight.diffuseIntensity * DirectionalLight.diffuseColor;
            f_diffuse += shadowLevel * intensity * NdotL * BRDF_lambertian(materialInfo.f0, materialInfo.f90, materialInfo.c_diff, materialInfo.specularWeight, VdotH);
            f_specular += shadowLevel * intensity * NdotL * BRDF_specularGGX(materialInfo.f0, materialInfo.f90, materialInfo.alphaRoughness, materialInfo.specularWeight, VdotH, NdotL, NdotV, NdotH);
            f_clearcoat += shadowLevel * intensity * getPunctualRadianceClearCoat(materialInfo.clearcoatNormal, v, l, h, VdotH,
                materialInfo.clearcoatF0, materialInfo.clearcoatF90, materialInfo.clearcoatRoughness);
#ifdef MATERIAL_SHEEN
            f_sheen += shadowLevel * intensity * getPunctualRadianceSheen(materialInfo.sheenColorFactor, materialInfo.sheenRoughnessFactor, NdotL, NdotV, NdotH);
            albedoSheenScaling = min(1.0 - max3(materialInfo.sheenColorFactor) * albedoSheenScalingLUT(NdotV, materialInfo.sheenRoughnessFactor),
                1.0 - max3(materialInfo.sheenColorFactor) * albedoSheenScalingLUT(NdotL, materialInfo.sheenRoughnessFactor));
#endif

#ifdef MATERIAL_CLEARCOAT
            f_clearcoat += shadowLevel * intensity * getPunctualRadianceClearCoat(materialInfo.clearcoatNormal, v, l, h, VdotH,
                materialInfo.clearcoatF0, materialInfo.clearcoatF90, materialInfo.clearcoatRoughness);
#endif
        }

        // BDTF
#ifdef MATERIAL_TRANSMISSION
        // If the light ray travels through the geometry, use the point it exits the geometry again.
        // That will change the angle to the light source, if the material refracts the light ray.
        vec3 transmissionRay = getVolumeTransmissionRay(n, v, materialInfo.thickness, materialInfo.ior, u_ModelMatrix);
        pointToLight -= transmissionRay;
        l = normalize(pointToLight);

        vec3 intensity = getLighIntensity(light, pointToLight);
        vec3 transmittedLight = intensity * getPunctualRadianceTransmission(n, v, l, materialInfo.alphaRoughness, materialInfo.f0, materialInfo.f90, materialInfo.baseColor, materialInfo.ior);

#ifdef MATERIAL_VOLUME
        transmittedLight = applyVolumeAttenuation(transmittedLight, length(transmissionRay), materialInfo.attenuationColor, materialInfo.attenuationDistance);
#endif

        f_transmission += materialInfo.transmissionFactor * transmittedLight;
#endif
    }
#endif

    f_emissive = material.emisiveFactor;
#ifdef HAS_EMISSIVE_MAP
    f_emissive *= sRGBToLinear(EmmissiveTexture.Sample(EmmissiveTextureSampler, input.UV0).rgb);
    //f_emissive *= EmmissiveTexture.Sample(EmmissiveTextureSampler, input.UV0).rgb;
#endif

    float3 color = float3(0.0.xxx);
    
    // Layer blending
    float clearcoatFactor = 0.0;
    float3 clearcoatFresnel = float3(0.0.xxx);

#ifdef MATERIAL_CLEARCOAT
    clearcoatFactor = materialInfo.clearcoatFactor;
    clearcoatFresnel = F_Schlick(materialInfo.clearcoatF0, materialInfo.clearcoatF90, clampedDot(materialInfo.clearcoatNormal, v));
    f_clearcoat = f_clearcoat * clearcoatFactor;
#endif

#ifdef MATERIAL_TRANSMISSION
    vec3 diffuse = mix(f_diffuse, f_transmission, materialInfo.transmissionFactor);
#else
    float3 diffuse = f_diffuse;
#endif

    color = f_emissive + diffuse + f_specular;
    color = f_sheen + color * albedoSheenScaling;
    color = color * (1.0 - clearcoatFactor * clearcoatFresnel) + f_clearcoat;

#if ALPHAMODE == 1  || ALPHAMODE == 0
    // Late discard to avoid samplig artifacts. See https://github.com/KhronosGroup/glTF-Sample-Viewer/issues/267
    if (baseColor.a < material.alphaCutoff)
    {
        discard;
    }
    baseColor.a = 1.0;
#endif
    
#ifdef LINEAR_OUTPUT
    finalColor = float4(color.rgb, baseColor.a);
#else
    //finalColor = float4(pow(abs(color.rgb), float3(0.4545, 0.4545, 0.4545)), baseColor.a);
    finalColor = float4(toneMap(color.rgb), baseColor.a);
#endif

#ifdef HAS_VERTEX_COLOR_0_VEC4
    //finalColor = float4(1.0.xxxx);
    //finalColor= getBaseColor(input);
    //finalColor = input.v_color*sRGBToLinear(AlbedoTexture.Sample(AlbedoTextureSampler, input.UV0).rgba);
    //finalColor = input.v_color;
    //finalColor.a = 1.0f;
    //finalColor= input.v_color;
    //finalColor.a = 1.0f;
#endif

    return finalColor;
    
}