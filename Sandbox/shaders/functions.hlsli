
static const float M_PI = 3.14159265359;

struct NormalInfo
{
    float3 ng; // Geometry normal
    float3 t; // Geometry tangent
    float3 b; // Geometry bitangent
    float3 n; // Shading normal
    float3 ntex; // Normal from texture, scaling is accounted for.
};

float clampedDot(float3 x, float3 y)
{
    return clamp(dot(x, y), 0.0, 1.0);
}


float max3(float3 v)
{
    return max(max(v.x, v.y), v.z);
}


float applyIorToRoughness(float roughness, float ior)
{
    // Scale roughness with IOR so that an IOR of 1.0 results in no microfacet refraction and
    // an IOR of 1.5 results in the default amount of microfacet refraction.
    return roughness * clamp(ior * 2.0 - 2.0, 0.0, 1.0);
}