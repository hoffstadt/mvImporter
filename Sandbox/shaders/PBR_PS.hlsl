
static const float PI = 3.14159265359;

// Fresnel function ----------------------------------------------------
float3 conductor_fresnel(float3 bsdf, float dotVH, float3 F0)
{
    return bsdf * (F0 + (1.0 - F0) * pow(1.0 - abs(dotVH), 5.0));
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
    float3 WorldNormal      : NORMAL0;
    float2 UV               : TEXCOORD0;
    float4 dshadowWorldPos  : dshadowPosition; // light pos
    float4 oshadowWorldPos  : oshadowPosition; // light pos
    float3x3 TBN            : TangentBasis;
};

float3 calculateNormal(VSOut input)
{

    float3 N = normalize(input.WorldNormal);
    
    if (material.useNormalMap && info.useNormalMap)
    {
        float3 tangentNormal = NormalTexture.Sample(Sampler, input.UV).xyz * 2.0 - 1.0;
        tangentNormal.y = -tangentNormal.y;
        return normalize(mul(input.TBN, tangentNormal));
    }
    
    return N;
}

float3 specular_brdf(float r2, float3 L, float3 V, float3 N)
{

	// Precalculate vectors and dot products
    float3 H = normalize(V + L);
    float dotNV = clamp(dot(N, V), 0.0, 1.0);
    float dotNL = clamp(dot(N, L), 0.0, 1.0);
    float dotNH = clamp(dot(N, H), 0.0, 1.0);
    float dotHL = clamp(dot(H, L), 0.0, 1.0);
    float dotHV = clamp(dot(H, L), 0.0, 1.0);
    
    float a2 = r2 * r2;
    float dotNH2 = dotNH * dotNH;
    float dotNV2 = dotNV * dotNV;

    float3 color = float3(0.0, 0.0, 0.0);

    //if (dotNL > 0.0)
    {
        float D = (a2 + dotNH) / (PI * (dotNH2 * (a2 - 1) + 1) * (dotNH2 * (a2 - 1) + 1));
        
        float k1 = dotNL + sqrt(a2 + (1 - a2) * dotNH2);
        float k2 = dotNV + sqrt(a2 + (1 - a2) * dotNV2);
        
        float V = dotHL * dotHV / (k1 * k2);
        
        return float3(1.0, 1.0, 1.0) * (V * D);
    }

    return color;
}

float4 diffuse_brdf(float4 color)
{
    return (1 / PI) * color;
}

float3 fresnel_mix(float ior, float dotVH, float3 base, float3 layer)
{
    float f0 = pow((1 - ior) / (1 + ior), 2);
    float fr = f0 + (1 - f0) * pow(1 - abs(dotVH), 5);
    return lerp(base, layer, fr);
}

float4 get_base_color(VSOut input)
{
    float4 base_color = material.albedo;
    
    if (material.useAlbedoMap && info.useAlbedo)
    {
        base_color = AlbedoTexture.Sample(Sampler, input.UV).rgba;
        base_color = pow(base_color, float4(2.2, 2.2, 2.2, 1.0)) * material.albedo;
    }
    
    return base_color;
}

float3 get_f0(float uior)
{
    float value = pow((uior - 1.0) / (uior + 1.0), 2);
    return float3(value, value, value);
}


float4 main(VSOut input) : SV_Target
{
    float4 base_color = get_base_color(input);
    clip(base_color.a < 0.1f ? -1 : 1); // bail if highly translucent
    
    float ior = 1.5;
    float3 f0 = float3(0.04, 0.04, 0.04);
    
    f0 = get_f0(ior);
    
    float reflectance = max(max(f0.r, f0.g), f0.b);
    
    float3 f90 = float3(1.0, 1.0, 1.0);

    float3 N = calculateNormal(input);
    float3 irradiance = info.ambientColor;
    float metallic = material.metalness;
    float roughness = material.roughness;
    float3 emissive = material.emisiveFactor;
    
    // flip normal when backface
    if (dot(N, input.WorldPos - info.camPos) > 0.0f)
    {
        N = -N;
    }
    
    float3 V = normalize(info.camPos - input.WorldPos);
    float3 R = reflect(-V, N);
    
    if (material.useMetalMap && info.useMetalness)
    {
        metallic = MetalRoughnessTexture.Sample(Sampler, input.UV).b;
    }
    else if (material.useMetalMap)
    {
        metallic = 0.0;
    }
    
    if (material.useRoughnessMap && info.useRoughness)
    {
        roughness = MetalRoughnessTexture.Sample(Sampler, input.UV).g;
    }
    else if (material.useRoughnessMap)
    {
        roughness = 0.0;
    }
    
    if (material.useEmissiveMap && info.useEmissiveMap)
    {
        float3 emissivity = EmmissiveTexture.Sample(Sampler, input.UV).rgb;
        emissivity = pow(emissivity, float3(2.2, 2.2, 2.2));
        emissive = emissivity * material.emisiveFactor;
    }
    else if (material.useEmissiveMap)
    {
        emissive = float3(0.0, 0.0, 0.0);
    }
    
    //-----------------------------------------------------------------------------
    // directional light
    //-----------------------------------------------------------------------------
    float3 L = -DirectionalLight.viewLightDir;
    
    float r2 = roughness * roughness;
    float3 H = normalize(V + L);
    float dotVH = dot(V, H);
    
    float3 metal_brdf = specular_brdf(r2, L, V, N) * (base_color.rgb + (1 - base_color.rgb) * pow(1 - abs(dotVH), 5));
    float3 dielectric_brdf = lerp(diffuse_brdf(base_color).rgb, specular_brdf(r2, L, V, N), 0.04 + (1 - 0.04) * pow(1 - abs(dotVH), 5));;
    float3 material_color = lerp(dielectric_brdf, metal_brdf, metallic);
    
    float3 brdf = material_color;
    
    float3 final_color = brdf + emissive;
    final_color = pow(final_color, float3(0.4545.xxx));
    return float4(final_color, base_color.a);
}