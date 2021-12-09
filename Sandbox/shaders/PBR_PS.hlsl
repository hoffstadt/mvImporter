
static const float PI = 3.14159265359;

// shadows
static const float zf = 100.0f;
static const float zn = 0.5f;
static const float c1 = (zf + zn) / (zf - zn);
static const float c0 = -(2 * zn * zf) / (zf - zn);
static const int PCFRANGE = 2;

//-----------------------------------------------------------------------------
// equations
//-----------------------------------------------------------------------------
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

float Attenuate(uniform float attConst, uniform float attLin, uniform float attQuad, const in float distFragToL)
{
    return 1.0f / (attConst + attLin * distFragToL + attQuad * (distFragToL * distFragToL));
}

// Normal Distribution function --------------------------------------
float D_GGX(float dotNH, float roughness)
{
    float alpha = roughness * roughness;
    float alpha2 = alpha * alpha;
    float denom = dotNH * dotNH * (alpha2 - 1.0) + 1.0;
    return (alpha2) / (PI * denom * denom);
}


// Geometric Shadowing function --------------------------------------
float G_SchlicksmithGGX(float dotNL, float dotNV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    float GL = dotNL / (dotNL * (1.0 - k) + k);
    float GV = dotNV / (dotNV * (1.0 - k) + k);
    return GL * GV;
}

// Fresnel function ----------------------------------------------------
float3 F_Schlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}
float3 F_SchlickR(float cosTheta, float3 F0, float roughness)
{
    return F0 + (max((1.0 - roughness).xxx, F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

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
    
    bool useAlbedoMap;
    bool useNormalMap;
    bool useRoughnessMap;
    bool useMetalMap;
    //-------------------------- ( 16 bytes )
    
    float3 emisiveFactor;
    bool useEmissiveMap;
    //-------------------------- ( 16 bytes )
    
    bool hasAlpha;
    bool useOcclusionMap;
    float occlusionStrength;
    //-------------------------- ( 4 * 16 = 64 bytes )
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
    bool useShadows;
    
    bool useOmniShadows;
    bool useSkybox;
    bool useAlbedo;
    bool useMetalness;
    //-------------------------- ( 16 bytes )
    
    bool useRoughness;
    bool useIrradiance;
    bool useReflection;
    bool useEmissiveMap;
    
    float4x4 projection;
    float4x4 model;
    float4x4 view;
    
    float3 camPos;
    bool useOcclusionMap;
    //-------------------------- ( 1*16 = 16 bytes )
    
    bool useNormalMap;
    bool usePCF;
    int pcfRange;
};


//-----------------------------------------------------------------------------
// textures
//-----------------------------------------------------------------------------
Texture2D   AlbedoTexture        : register(t0);
Texture2D   NormalTexture        : register(t1);
Texture2D   MetalRoughnessTexture: register(t2);
Texture2D   EmmissiveTexture     : register(t3);
Texture2D   OcclusionTexture     : register(t4);
Texture2D   DirectionalShadowMap : register(t5);
TextureCube ShadowMap            : register(t6);
TextureCube Environment          : register(t7);

//-----------------------------------------------------------------------------
// samplers
//-----------------------------------------------------------------------------
SamplerState           Sampler            : register(s0);
SamplerComparisonState DShadowSampler     : register(s1);
SamplerComparisonState OShadowSampler     : register(s2);
SamplerState           EnvironmentSampler : register(s3);

//-----------------------------------------------------------------------------
// constant buffers
//-----------------------------------------------------------------------------
cbuffer mvPointLightCBuf       : register(b0) { mvPointLight PointLight; };
cbuffer mvMaterialCBuf         : register(b1) { mvMaterial material; };
cbuffer mvDirectionalLightCBuf : register(b2) { mvDirectionalLight DirectionalLight; };
cbuffer mvGlobalCBuf           : register(b3) { mvGlobalInfo info; };

struct VSOut
{   
    float4 Pos              : SV_Position;
    float3 WorldPos         : POSITION0;
    float3 Normal           : NORMAL0;
    float2 UV               : TEXCOORD0;
    float3 Tangent          : TEXCOORD1;
    float4 dshadowWorldPos  : dshadowPosition; // light pos
    float4 oshadowWorldPos  : oshadowPosition; // light pos
};

float3 calculateNormal(VSOut input)
{

    float3 N = normalize(input.Normal);
    float3 T = normalize(input.Tangent);
    float3 B = normalize(cross(N, T));
    float3x3 TBN = transpose(float3x3(T, B, N));
    
    if (material.useNormalMap && info.useNormalMap)
    {
        float3 tangentNormal = NormalTexture.Sample(Sampler, input.UV).xyz * 2.0 - 1.0;
        return normalize(mul(TBN, tangentNormal));
    }
    
    return N;
    //return normalize(input.ViewNormal);
}

float3 specularContribution(float3 albedo, float2 inUV, float3 L, float3 V, float3 N, float3 F0, float metallic, float roughness)
{
	// Precalculate vectors and dot products
    float3 H = normalize(V + L);
    float dotNH = clamp(dot(N, H), 0.0, 1.0);
    float dotNV = clamp(dot(N, V), 0.0, 1.0);
    float dotNL = clamp(dot(N, L), 0.0, 1.0);

	// Light color fixed
    float3 lightColor = float3(1.0, 1.0, 1.0);

    float3 color = float3(0.0, 0.0, 0.0);

    if (dotNL > 0.0)
    {
		// D = Normal distribution (Distribution of the microfacets)
        float D = D_GGX(dotNH, roughness);
		// G = Geometric shadowing term (Microfacets shadowing)
        float G = G_SchlicksmithGGX(dotNL, dotNV, roughness);
		// F = Fresnel factor (Reflectance depending on angle of incidence)
        float3 F = F_Schlick(dotNV, F0);
        float3 spec = D * F * G / (4.0 * dotNL * dotNV + 0.001);
        float3 kD = (float3(1.0, 1.0, 1.0) - F) * (1.0 - metallic);
        color += (kD * albedo / PI + spec) * dotNL;
    }

    return color;
}

float3 BRDF(float3 L, float3 V, float3 N, float metallic, float roughness)
{
	// Precalculate vectors and dot products
    float3 H = normalize(V + L);
    float dotNV = clamp(dot(N, V), 0.0, 1.0);
    float dotNL = clamp(dot(N, L), 0.0, 1.0);
    float dotLH = clamp(dot(L, H), 0.0, 1.0);
    float dotNH = clamp(dot(N, H), 0.0, 1.0);

	// Light color fixed
    float3 lightColor = float3(1.0, 1.0, 1.0);

    float3 color = float3(0.0, 0.0, 0.0);

    if (dotNL > 0.0)
    {
        float rroughness = max(0.05, roughness);
		// D = Normal distribution (Distribution of the microfacets)
        float D = D_GGX(dotNH, roughness);
		// G = Geometric shadowing term (Microfacets shadowing)
        float G = G_SchlicksmithGGX(dotNL, dotNV, rroughness);
		// F = Fresnel factor (Reflectance depending on angle of incidence)
        float3 F = F_Schlick(dotNV, metallic);

        float3 spec = D * F * G / (4.0 * dotNL * dotNV);

        color += spec * dotNL * lightColor;
    }

    return color;
}

float3 prefilteredReflection(float3 R, float roughness)
{
    const float MAX_REFLECTION_LOD = 9.0; // todo: param/const
    float lod = roughness * MAX_REFLECTION_LOD;
    float lodf = floor(lod);
    float lodc = ceil(lod);
    //float3 a = prefilteredMapTexture.SampleLevel(prefilteredMapSampler, R, lodf).rgb;
    //float3 b = prefilteredMapTexture.SampleLevel(prefilteredMapSampler, R, lodc).rgb;
    float3 a = Environment.SampleLevel(EnvironmentSampler, R, lodf).rgb;
    float3 b = Environment.SampleLevel(EnvironmentSampler, R, lodc).rgb;
    return lerp(a, b, lod - lodf);
}

float4 getIrradiance(float3 N)
{
    float3 up = float3(0.0, 1.0, 0.0);
    float3 right = normalize(cross(up, N));
    up = cross(N, right);

    const float TWO_PI = PI * 2.0;
    const float HALF_PI = PI * 0.5;

    float3 color = float3(0.0, 0.0, 0.0);
    float sampleDelta = 0.5;
    float nrSamples = 0.0;
    for (float phi = 0.0; phi < TWO_PI; phi += sampleDelta)
    {
        for (float theta = 0.0; theta < HALF_PI; theta += sampleDelta)
        {
            float3 tempVec = cos(phi) * right + sin(phi) * up;
            float3 sampleVector = cos(theta) * N + sin(theta) * tempVec;
            color += Environment.Sample(EnvironmentSampler, sampleVector).rgb * cos(theta) * sin(theta);
            nrSamples++;
        }
    }
    return float4(PI * color / float(nrSamples), 1.0);
}

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
    for (int x = -info.pcfRange; x <= info.pcfRange; x++)
    {
        [loop]
        for (int y = -info.pcfRange; y <= info.pcfRange; y++)
        {
            shadowLevel += DirectionalShadowMap.SampleCmpLevelZero(DShadowSampler, float2(spos.x + dx*x, spos.y + dy*y), depthCheck);
            count++;
        }
    }
    return shadowLevel / count;
}

float4 main(VSOut input) : SV_Target
{
    float4 albedo = material.albedo;
    
    if (material.useAlbedoMap && info.useAlbedo)
    {
        albedo = AlbedoTexture.Sample(Sampler, input.UV).rgba;

        // bail if highly translucent
        clip(albedo.a < 0.1f ? -1 : 1);

        albedo = pow(albedo, float4(2.2, 2.2, 2.2, 1.0));
        
        //// flip normal when backface
        //if (dot(input.ViewNormal, input.ViewPos) >= 0.0f)
        //{
        //    input.ViewNormal = -input.ViewNormal;
        //    input.Normal = -input.Normal;
        //}
       
    }
    else if (material.useAlbedoMap)
    {
        float albedo_alpha = AlbedoTexture.Sample(Sampler, input.UV).a;

        // bail if highly translucent
        clip(albedo_alpha < 0.1f ? -1 : 1);
    }

    float3 N = calculateNormal(input);
    float3 irradiance = info.ambientColor;
    float metallic = material.metalness;
    float roughness = material.roughness;
    
    float3 V = normalize(info.camPos - input.WorldPos);
    float3 R = reflect(-V, N);
    float dotNV = clamp(dot(N, V), 0.0, 1.0);
    
    if (material.useMetalMap && info.useMetalness)
    {
        metallic = MetalRoughnessTexture.Sample(Sampler, input.UV).b;
    }
    else if(!info.useMetalness)
    {
        metallic = 0.0f;
    }
    
    if (material.useRoughnessMap && info.useRoughness)
    {
        roughness = MetalRoughnessTexture.Sample(Sampler, input.UV).g;
    }
    
    float3 F0 = float3(0.04, 0.04, 0.04);
    F0 = lerp(F0, albedo.rgb, metallic);
    
    float3 Lo = float3(0.0, 0.0, 0.0);
    
    //-----------------------------------------------------------------------------
    // point light
    //-----------------------------------------------------------------------------
    input.oshadowWorldPos.z = -input.oshadowWorldPos.z;
    float shadowLevel = Shadow(input.oshadowWorldPos, ShadowMap, OShadowSampler);
    float3 L = normalize(PointLight.viewLightPos - input.WorldPos);
    float3 brdf = float3(0.0, 0.0, 0.0);
    
    if (shadowLevel != 0.0f && info.useOmniShadows)
    {
        Lo += specularContribution(albedo.rgb, input.UV, L, V, N, F0, metallic, roughness);
        
        brdf = BRDF(L, V, N, metallic, roughness);
        
        Lo *= shadowLevel;
        brdf *= shadowLevel;
        
    }
    else if (info.useOmniShadows)
    {
    }
    else
    {
        Lo += specularContribution(albedo.rgb, input.UV, L, V, N, F0, metallic, roughness);
        brdf = BRDF(L, V, N, metallic, roughness);
    }

    //-----------------------------------------------------------------------------
    // directional light
    //-----------------------------------------------------------------------------
    L = -DirectionalLight.viewLightDir;
    float2 projectTexCoord;
    float lightDepthValue;
    
    // Calculate the projected texture coordinates.
    projectTexCoord.x = 0.5f * input.dshadowWorldPos.x / input.dshadowWorldPos.w + 0.5f;
    projectTexCoord.y = -0.5f * input.dshadowWorldPos.y / input.dshadowWorldPos.w + 0.5f;
        
        // Determine if the projected coordinates are in the 0 to 1 range.  If so then this pixel is in the view of the light.
    if ((saturate(projectTexCoord.x) == projectTexCoord.x) && (saturate(projectTexCoord.y) == projectTexCoord.y) && info.useShadows)
    {
        
        // Calculate the depth of the light.
        lightDepthValue = input.dshadowWorldPos.z / input.dshadowWorldPos.w;
            
        if(info.usePCF)
        {
            shadowLevel = filterPCF(projectTexCoord, lightDepthValue);
        }
        else
        {
            shadowLevel = DirectionalShadowMap.SampleCmpLevelZero(DShadowSampler, projectTexCoord, lightDepthValue);
        }
        
        
        if (shadowLevel != 0.0f) // not in shadow
        {
            Lo += (shadowLevel * specularContribution(albedo.rgb, input.UV, L, V, N, F0, metallic, roughness));
            brdf += (shadowLevel * BRDF(L, V, N, metallic, roughness));
            
        }
            
    }
    else
    {
        Lo += specularContribution(albedo.rgb, input.UV, L, V, N, F0, metallic, roughness);
        brdf += BRDF(L, V, N, metallic, roughness);

    }
    
    if (info.useIrradiance)
    {
        irradiance = getIrradiance(-N);
    }
    else
    {
                //irradiance = shadowLevel * Environment.Sample(EnvironmentSampler, N).rgb;
        irradiance = info.ambientColor;
    }
    
    //float2 brdf = textureBRDFLUT.Sample(samplerBRDFLUT, float2(max(dot(N, V), 0.0), roughness)).rg;
    float3 reflection = prefilteredReflection(R, roughness).rgb;
    
    // Diffuse based on irradiance
    float3 diffuse = irradiance * albedo.rgb;

    float3 F = F_SchlickR(max(dot(N, V), 0.0), F0, roughness);
    
    // Specular reflectance
    float3 specular = (F * brdf.x + brdf.y);
    if(info.useReflection)
        specular *= reflection;
    
    // Ambient part
    float3 kD = 1.0 - F;
    kD *= 1.0 - metallic;
    float3 ambient = (kD * diffuse + specular);
    if(material.useOcclusionMap && info.useOcclusionMap)
    {
        float occlusionValue = OcclusionTexture.Sample(Sampler, input.UV).r;
        occlusionValue = 1.0 + material.occlusionStrength * (occlusionValue - 1.0);
        ambient.r *= occlusionValue;
        ambient.g *= occlusionValue;
        ambient.b *= occlusionValue;
    }
    
    if(material.useEmissiveMap && info.useEmissiveMap)
    {
        float3 emissivity = EmmissiveTexture.Sample(Sampler, input.UV).rgb;
        Lo += material.albedo.rgb * emissivity * material.emisiveFactor;
    }
    
    float3 color = ambient + Lo;
    
    // Tone mapping
    //color = Uncharted2Tonemap(color * uboParams.exposure);
    //color = color * (1.0f / Uncharted2Tonemap((11.2f).xxx));
    // Gamma correction
    //color = pow(color, (1.0f / uboParams.gamma).xxx);

    return float4(color, 1.0);
}