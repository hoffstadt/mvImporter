
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

struct mvDirectionalLight
{
    float diffuseIntensity;
    float3 viewLightDir;
    //-------------------------- ( 16 bytes )
    
    float3 diffuseColor;
    //-------------------------- ( 16 bytes )
    
    //-------------------------- ( 2*16 = 32 bytes )
};


//-----------------------------------------------------------------------------
// constant buffers
//-----------------------------------------------------------------------------
cbuffer mvPointLightCBuf       : register(b0) { mvPointLight PointLight; };
cbuffer mvDirectionalLightCBuf : register(b1) { mvDirectionalLight DirectionalLight; };

struct VSOut
{
    float3 viewPos          : Position;        // pixel pos           (view space)
    float3 viewNormal       : Normal;          // pixel norm          (view space)
    float4 pixelPos         : SV_Position;     // pixel pos           (screen space)
};

float Attenuate(uniform float attConst, uniform float attLin, uniform float attQuad, const in float distFragToL)
{
    return 1.0f / (attConst + attLin * distFragToL + attQuad * (distFragToL * distFragToL));
}

float4 main(VSOut input) : SV_Target
{
    float3 diffuse = { 0.0f, 0.0f, 0.0f };
    float3 ambient = { 0.04f, 0.04f, 0.04f };
    float3 specularReflected = { 0.0f, 0.0f, 0.0f };
    float3 specularReflectedColor = { 0.18f, 0.18f, 0.18f };
    float4 materialColor = { 0.45f, 0.45f, 0.85f, 1.0f };
    float specularPowerLoaded = 8.0f;

    // normalize the mesh normal
    input.viewNormal = normalize(input.viewNormal);
    
    //-----------------------------------------------------------------------------
    // point light
    //-----------------------------------------------------------------------------
    {
        // fragment to light vector data
        float3 lightVec = PointLight.viewLightPos - input.viewPos;
        float lightDistFromFrag = length(lightVec);
        float3 lightDirVec = lightVec / lightDistFromFrag;

        // attenuation
        const float att = Attenuate(PointLight.attConst, PointLight.attLin, PointLight.attQuad, lightDistFromFrag);

        // diffuse
        diffuse += PointLight.diffuseColor * PointLight.diffuseIntensity * att * max(0.0f, dot(lightDirVec, input.viewNormal));

        // specular

        // calculate reflected light vector
        const float3 w = input.viewNormal * dot(lightVec, input.viewNormal);
        const float3 r = normalize(w * 2.0f - lightVec);

        // vector from camera to fragment
        const float3 viewCamToFrag = normalize(input.viewPos);

        specularReflected += att * PointLight.diffuseColor * PointLight.diffuseIntensity * specularReflectedColor * 1.0f * pow(max(0.0f, dot(-r, viewCamToFrag)), specularPowerLoaded);
    }
    
    //-----------------------------------------------------------------------------
    // directional light
    //-----------------------------------------------------------------------------
    {
        
        float3 lightDir;
        
        lightDir = -DirectionalLight.viewLightDir;

        {
             // diffuse
            diffuse += DirectionalLight.diffuseColor * DirectionalLight.diffuseIntensity * max(0.0f, dot(normalize(lightDir), input.viewNormal));
    
            // specular
        
            // calculate reflected light vector
            const float3 w = input.viewNormal * dot(normalize(lightDir), input.viewNormal);
            const float3 r = normalize(w * 2.0f - normalize(lightDir));
        
            // vector from camera to fragment
            const float3 viewCamToFrag = normalize(input.viewPos);
        
            specularReflected += DirectionalLight.diffuseColor * DirectionalLight.diffuseIntensity * specularReflectedColor * 1.0f * pow(max(0.0f, dot(-r, viewCamToFrag)), specularPowerLoaded);
            
        }
        
    }
    
    //-----------------------------------------------------------------------------
    // final color
    //-----------------------------------------------------------------------------

    float3 litColor = saturate((diffuse + ambient) * materialColor.rgb + specularReflected);

	return float4(litColor, 1.0f);
	}