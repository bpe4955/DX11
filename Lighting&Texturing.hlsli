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

cbuffer lightTexData : register(b0)
{
    float4 colorTint;
    float roughness;
    Light lights[MAX_NUM_LIGHTS];
    int numLights;
    float3 cameraPosition;
    float3 ambient;
    float2 uvOffset;
    float2 uvScale;
    bool hasSpecMap;
    bool hasMask;
}

// Textures
Texture2D SurfaceTexture : register(t0);
Texture2D SpecularMap : register(t1);
Texture2D TextureMask : register(t2);
SamplerState Sampler : register(s0);

// Move this to a "Helper.hlsli"
float random(float2 s)
{
    return frac(sin(dot(s, float2(12.9898, 78.233))) * 43758.5453123);
}

float Attenuate(Light light, float3 worldPos)
{
    float dist = distance(light.Position, worldPos);
    float att = saturate(1.0f - (dist * dist / (light.Range * light.Range)));
    return att * att;
}

float Attenuate(float dist, float range, float3 worldPos)
{
    float att = saturate(1.0f - (dist * dist / (range * range)));
    return att * att;
}

float ConeAttenuate(float3 dir, Light light, float3 worldPosition)
{
    float angleFromCenter = max(dot(normalize(worldPosition - light.Position), light.Direction), 0.0f);
    return pow(angleFromCenter, light.SpotFalloff);
}

float Lambert(float3 normal, float3 lightDir)
{
    // angle between normal and direction to the light (negate Light Direction)
    return saturate(dot(normal, -lightDir));
}

float Phong(float3 normal, float3 lightDir, float3 viewVector, float specularPower)
{
    float ret = 0.0f;
    if (specularPower != 0)
    {
    // Reflection of the Light coming off the surface
        float3 refl = reflect(lightDir, normal);
    // Get the angle between the view and reflection, saturate, raise to power
        ret = pow(saturate(dot(viewVector, refl)), specularPower);
    }
    return ret;
}


float3 DirectionalLight(float3 normal, Light light, float3 viewVector, float specularPower, float3 surfaceColor, float specScale)
{
    float3 normLightDir = normalize(light.Direction);
    float diffuse = Lambert(normal, normLightDir);
    float specular = Phong(normal, normLightDir, viewVector, specularPower) * specScale;
    
    return light.Intensity * (diffuse * surfaceColor + specular) * light.Color;
}

float3 PointLight(float3 normal, Light light, float3 viewVector, float specularPower, float3 worldPosition, float3 surfaceColor, float specScale)
{
    float3 normLightDir = normalize(worldPosition - light.Position);
    float diffuse = Lambert(normal, normLightDir);
    float specular = Phong(normal, normLightDir, viewVector, specularPower) * specScale;
    
    return light.Intensity * (diffuse * surfaceColor + specular) * Attenuate(light, worldPosition) * light.Color;
}

float3 SpotLight(float3 normal, Light light, float3 viewVector, float specularPower, float3 worldPosition, float3 surfaceColor, float specScale)
{
    float3 normLightDir = normalize(light.Direction);
    float diffuse = Lambert(normal, normLightDir);
    float specular = Phong(normal, normLightDir, viewVector, specularPower) * specScale;
    
    return light.Intensity * (diffuse * surfaceColor + specular) * Attenuate(light, worldPosition) * ConeAttenuate(normLightDir, light, worldPosition) * light.Color;
}

// When getting spotlight calculations outside of totalLight
float3 SpotLight(float3 normal, Light light, float3 viewVector, float specularPower, float3 worldPosition, float2 uv)
{
    // Texturing
    uv += uvOffset;
    uv *= uvScale;
    float3 surfaceColor = SurfaceTexture.Sample(Sampler, uv).rgb * colorTint.rbg;
    float specScale;
    if (!hasSpecMap)
    {
        specScale = 1.0f;
    }
    else
    {
        specScale = SpecularMap.Sample(Sampler, uv).r;
    }
    float3 mask;
    if (hasMask)
    {
        mask = TextureMask.Sample(Sampler, uv).rgb;
        surfaceColor *= mask;
    }
    // Lighting
    float3 normLightDir = normalize(light.Direction);
    float diffuse = Lambert(normal, normLightDir);
    float specular = Phong(normal, normLightDir, viewVector, specularPower) * specScale;
    
    return light.Intensity * (diffuse * surfaceColor + specular) * Attenuate(light, worldPosition) * ConeAttenuate(normLightDir, light, worldPosition) * light.Color;
}

float3 totalLight(float3 _normal, float3 worldPosition, float2 uv)
{
    // Texturing
    uv += uvOffset;
    uv *= uvScale;
    float3 surfaceColor = SurfaceTexture.Sample(Sampler, uv).rgb * colorTint.rbg;
    float specScale;
    if (!hasSpecMap)
    {
        specScale = 1.0f;
    }
    else
    {
        specScale = SpecularMap.Sample(Sampler, uv).r;
    }
    float3 mask;
    if (hasMask)
    {
        mask = TextureMask.Sample(Sampler, uv).rgb;
        surfaceColor *= mask;
    }
    // Lighting
    float3 normal = normalize(_normal);
    float3 viewVector = normalize(cameraPosition - worldPosition);
    float3 totalLight = ambient * surfaceColor;
    float specularPower = (1.0f - roughness) * MAX_SPECULAR_EXPONENT;
    for (int i = 0; i < numLights; i++)
    {
        switch (lights[i].Type)
        {
            case LIGHT_TYPE_DIR:
                totalLight += DirectionalLight(normal, lights[i], viewVector, specularPower, surfaceColor, specScale);
                break;
            case LIGHT_TYPE_POINT:
                totalLight += PointLight(normal, lights[i], viewVector, specularPower, worldPosition, surfaceColor, specScale);
                break;
            case LIGHT_TYPE_SPOT:
                totalLight += SpotLight(normal, lights[i], viewVector, specularPower, worldPosition, surfaceColor, specScale);
                break;
        }
    }
    return totalLight;
}


#endif