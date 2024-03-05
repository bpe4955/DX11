#ifndef __GGP_LIGHTING__
#define __GGP_LIGHTING__

#define LIGHT_TYPE_DIR   0
#define LIGHT_TYPE_POINT 1
#define LIGHT_TYPE_SPOT  2
#define MAX_SPECULAR_EXPONENT 256.0f
#define MAX_NUM_LIGHTS 10

struct Light
{
    int Type;
    float3 Direction;
    float Range;
    float3 Position;
    float Intensity;
    float3 Color;
    float SpotFalloff;
    float3 Padding;
};

cbuffer lightData : register(b0)
{
    float4 colorTint;
    float roughness;
    Light lights[MAX_NUM_LIGHTS];
    int numLights;
    float3 cameraPosition;
    float3 ambient;
}

// Move this to a "Helper.hlsli"
float random(float2 s)
{
    return frac(sin(dot(s, float2(12.9898, 78.233))) * 43758.5453123);
}

float Lambert(float3 normal, float3 lightDir)
{
    // angle between normal and direction to the light (negate Light Direction)
    return dot(normal, -lightDir);
}

float Phong(float3 normal, float3 lightDir, float3 viewVector, float specularPower)
{
    // Reflection of the Light coming off the surface
    float3 refl = reflect(lightDir, normal);
    // Get the angle between the view and reflection, saturate, raise to power
    return pow(saturate(dot(viewVector, refl)), specularPower);
}


float3 DirectionalLight(float3 normal,float3 normLightDir, Light light, float3 viewVector, float specularPower)
{
    float diffuse = Lambert(normal, normLightDir);
    float specular = Phong(normal, normLightDir, viewVector, specularPower);
    
    return light.Intensity * (diffuse + specular) * light.Color;
}

//float3 SpotLight()
//{
//    
//}

float3 totalLight(float3 _normal, float3 worldPosition)
{
    float3 normal = normalize(_normal);
    float3 viewVector = normalize(cameraPosition - worldPosition);
    float3 totalLight = ambient;
    float specularExponent = (1.0f - roughness) * MAX_SPECULAR_EXPONENT;
    for (int i = 0; i < numLights; i++)
    {
        switch (lights[i].Type)
        {
            case LIGHT_TYPE_DIR:
                float3 normLightDir = normalize(lights[i].Direction);
                totalLight += DirectionalLight(normal, normLightDir, lights[i], viewVector, specularExponent);
                break;
            case LIGHT_TYPE_POINT:
            
                break;
            case LIGHT_TYPE_SPOT:
            
                break;
        }
    }
    return totalLight;
}


#endif