#ifndef __GGP_LIGHTING__
#define __GGP_LIGHTING__

#define LIGHT_TYPE_DIR   0
#define LIGHT_TYPE_POINT 1
#define LIGHT_TYPE_SPOT  2
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
    float2 uvOffset;
    float2 uvScale;
    bool hasMask;
    bool hasMetalMap;
    bool hasRoughMap;
    bool hasNormalMap;
    bool hasEnvironmentMap;
}

// Textures
Texture2D Albedo : register(t0);
Texture2D RoughnessMap : register(t1);
Texture2D MetalnessMap : register(t2);
Texture2D NormalMap : register(t3);
Texture2D TextureMask : register(t4);
TextureCube EnvironmentMap : register(t5);
SamplerState Sampler : register(s0);

// Constants
static const float F0_NON_METAL = 0.04f;
static const float MIN_ROUGHNESS = 0.0000001f; // 6 zeros after decimal
static const float PI = 3.14159265359f;



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

float ConeAttenuate(float3 toLight, Light light)
{
    // Calculate the spot falloff
    return pow(saturate(dot(-toLight, normalize(light.Direction))), light.SpotFalloff);
}

float Lambert(float3 normal, float3 dirToLight)
{
    // angle between normal and direction to the light (negate Light Direction)
    return saturate(dot(normal, dirToLight));
}

// Calculates diffuse amount based on energy conservation
//
// diffuse   - Diffuse amount
// F         - Fresnel result from microfacet BRDF
// metalness - surface metalness amount 
float3 DiffuseEnergyConserve(float3 diffuse, float3 F, float metalness)
{
    return diffuse * (1 - F) * (1 - metalness);
}

// Normal Distribution Function: GGX (Trowbridge-Reitz)
//
// a - Roughness
// h - Half vector
// n - Normal
// 
// D(h, n, a) = a^2 / pi * ((n dot h)^2 * (a^2 - 1) + 1)^2
float D_GGX(float3 n, float3 h, float roughness)
{
	// Pre-calculations
    float NdotH = saturate(dot(n, h));
    float NdotH2 = NdotH * NdotH;
    float a = roughness * roughness;
    float a2 = max(a * a, MIN_ROUGHNESS); // Applied after remap!

	// ((n dot h)^2 * (a^2 - 1) + 1)
	// Can go to zero if roughness is 0 and NdotH is 1
    float denomToSquare = NdotH2 * (a2 - 1) + 1;

	// Final value
    return a2 / (PI * denomToSquare * denomToSquare);
}

// Fresnel term - Schlick approx.
// 
// v - View vector
// h - Half vector
// f0 - Value when l = n
//
// F(v,h,f0) = f0 + (1-f0)(1 - (v dot h))^5
float3 F_Schlick(float3 v, float3 h, float3 f0)
{
	// Pre-calculations
    float VdotH = saturate(dot(v, h));

	// Final value
    return f0 + (1 - f0) * pow(1 - VdotH, 5);
}

// Geometric Shadowing - Schlick-GGX
// - k is remapped to a / 2, roughness remapped to (r+1)/2 before squaring!
//
// n - Normal
// v - View vector
//
// G_Schlick(n,v,a) = (n dot v) / ((n dot v) * (1 - k) * k)
//
// Full G(n,v,l,a) term = G_SchlickGGX(n,v,a) * G_SchlickGGX(n,l,a)
float G_SchlickGGX(float3 n, float3 v, float roughness)
{
	// End result of remapping:
    float k = pow(roughness + 1, 2) / 8.0f;
    float NdotV = saturate(dot(n, v));

	// Final value
	// Note: Numerator should be NdotV (or NdotL depending on parameters).
	// However, these are also in the BRDF's denominator, so they'll cancel!
	// We're leaving them out here AND in the BRDF function as the
	// dot products can get VERY small and cause rounding errors.
    return 1 / (NdotV * (1 - k) + k);
}

// Cook-Torrance Microfacet BRDF (Specular)
//
// f(l,v) = D(h)F(v,h)G(l,v,h) / 4(n dot l)(n dot v)
// - parts of the denominator are canceled out by numerator (see below)
//
// D() - Normal Distribution Function - Trowbridge-Reitz (GGX)
// F() - Fresnel - Schlick approx
// G() - Geometric Shadowing - Schlick-GGX
float3 MicrofacetBRDF(float3 n, float3 l, float3 v, float roughness, float3 f0, out float3 F_out)
{
	// Other vectors
    float3 h = normalize(v + l);

	// Run numerator functions
    float D = D_GGX(n, h, roughness);
    float3 F = F_Schlick(v, h, f0);
    float G = G_SchlickGGX(n, v, roughness) * G_SchlickGGX(n, l, roughness);
	
	// Pass F out of the function for diffuse balance
    F_out = F;

	// Final specular formula
	// Note: The denominator SHOULD contain (NdotV)(NdotL), but they'd be
	// canceled out by our G() term.  As such, they have been removed
	// from BOTH places to prevent floating point rounding errors.
    float3 specularResult = (D * F * G) / 4;

	// One last non-obvious requirement: According to the rendering equation,
	// specular must have the same NdotL applied as diffuse!  We'll apply
	// that here so that minimal changes are required elsewhere.
    return specularResult * max(dot(n, l), 0);
}

float3 DirectionalLight(float3 normal, Light light, float3 surfaceColor, float3 viewVector, float roughness, float3 specColor, float metalness)
{
    // Calculate Light
    float3 toLight = normalize(-light.Direction);
    float diffuse = Lambert(normal, toLight);
    float3 fresnelResult;
    float3 specular = MicrofacetBRDF(normal, toLight, viewVector, roughness, specColor, fresnelResult); 
    // Calculate diffuse with energy conservation, including diffuse for metals
    float3 balancedDiff = DiffuseEnergyConserve(diffuse, specular, metalness);
    // Combine the final diffuse and specular values for this light
    return (balancedDiff * surfaceColor + specular) * light.Intensity * light.Color;
}

float3 PointLight(float3 worldPosition, float3 normal, Light light, float3 surfaceColor, float3 viewVector, float roughness, float3 specColor, float metalness)
{
    // Calculate Light
    float3 toLight = normalize(light.Position - worldPosition); 
    float diffuse = Lambert(normal, toLight);
    float3 fresnelResult;
    float3 specular = MicrofacetBRDF(normal, toLight, viewVector, roughness, specColor, fresnelResult);
    // Calculate diffuse with energy conservation, including diffuse for metals
    float3 balancedDiff = DiffuseEnergyConserve(diffuse, specular, metalness);
    // Combine the final diffuse and specular values for this light
    return (balancedDiff * surfaceColor + specular) * light.Color * (light.Intensity * Attenuate(light, worldPosition));
}

float3 SpotLight(float3 worldPosition, float3 normal, Light light, float3 surfaceColor, float3 viewVector, float roughness, float3 specColor, float metalness)
{
    // Calculate Light
    float3 toLight = normalize(light.Position - worldPosition);
    float diffuse = Lambert(normal, toLight);
    float3 fresnelResult;
    float3 specular = MicrofacetBRDF(normal, toLight, viewVector, roughness, specColor, fresnelResult);
    // Calculate diffuse with energy conservation, including diffuse for metals
    float3 balancedDiff = DiffuseEnergyConserve(diffuse, specular, metalness);
    // Combine the final diffuse and specular values for this light
    return (balancedDiff * surfaceColor + specular) * light.Color * (light.Intensity * Attenuate(light, worldPosition) * ConeAttenuate(toLight, light));
}

// Assuming normal and tangent are already normalized
float3 normalMapCalc(float2 uv, float3 normal, float3 tangent)
{
    float3 unpackedNormal = NormalMap.Sample(Sampler, uv).rgb * 2.0f - 1.0f;
    float3 N = normal;
    float3 T = normalize(tangent - N * dot(tangent, N));
    float3 B = cross(T, N);
    float3x3 TBN = float3x3(T, B, N);
    return normalize(mul(unpackedNormal, TBN));
}

// When getting spotlight calculations outside of totalLight
float3 SpotLight(float3 normal, Light light, float3 viewVector, float3 worldPosition, float2 uv, float3 tangent)
{
    // Texturing
    uv = uv * uvScale + uvOffset;
    float3 surfaceColor = pow(Albedo.Sample(Sampler, uv).rgb, 2.2f);
    surfaceColor = hasMask ? (surfaceColor * TextureMask.Sample(Sampler, uv).rgb) : surfaceColor;
    normal = hasNormalMap ? normalMapCalc(uv, normal, tangent) : normal;
    float roughness = hasRoughMap ? RoughnessMap.Sample(Sampler, uv).r : 0.2f;
    float metalness = hasMetalMap ? MetalnessMap.Sample(Sampler, uv).r : 0.0f;
    // Specular color determination
    float3 specularColor = lerp(F0_NON_METAL, surfaceColor.rgb, metalness);
    // Lighting
    float3 toLight = normalize(light.Position - worldPosition);
    float diffuse = Lambert(normal, toLight);
    float3 fresnelResult;
    float3 specular = MicrofacetBRDF(normal, toLight, viewVector, roughness, specularColor, fresnelResult);
    // Calculate diffuse with energy conservation, including diffuse for metals
    float3 balancedDiff = DiffuseEnergyConserve(diffuse, specular, metalness);
    // Combine the final diffuse and specular values for this light
    return (balancedDiff * surfaceColor + specular) * light.Color * (light.Intensity * Attenuate(light, worldPosition) * ConeAttenuate(toLight, light));
}


// assuming input values are normalized
float3 totalLight(float3 normal, float3 worldPosition, float2 uv, float3 tangent)
{
    // Texturing
    uv = uv * uvScale + uvOffset;
    float3 surfaceColor = pow(Albedo.Sample(Sampler, uv).rgb, 2.2f);
    surfaceColor = hasMask ? (surfaceColor * TextureMask.Sample(Sampler, uv).rgb) : surfaceColor;
    normal = hasNormalMap ? normalMapCalc(uv, normal, tangent) : normal;
    float roughness = hasRoughMap ? RoughnessMap.Sample(Sampler, uv).r : 0.2f;
    float metalness = hasMetalMap ? MetalnessMap.Sample(Sampler, uv).r : 0.0f;
    // Specular color determination -----------------
    // Assume albedo texture is actually holding specular color where metalness == 1
    // Note the use of lerp here - metal is generally 0 or 1, but might be in between
    // because of linear texture sampling, so we lerp the specular color to match
    float3 specularColor = lerp(F0_NON_METAL, surfaceColor.rgb, metalness);
    // Lighting
    float3 viewVector = normalize(cameraPosition - worldPosition);
    float3 totalLight = float3(0, 0, 0);
    
    // Light Calculations
    for (int i = 0; i < numLights; i++)
    {
        switch (lights[i].Type)
        {
            case LIGHT_TYPE_DIR:
                totalLight += DirectionalLight(normal, lights[i], surfaceColor, viewVector, roughness, specularColor, metalness);
                break;
            case LIGHT_TYPE_POINT:
                totalLight += PointLight(worldPosition, normal, lights[i], surfaceColor, viewVector, roughness, specularColor, metalness);
                break;
            case LIGHT_TYPE_SPOT:
                totalLight += SpotLight(worldPosition, normal, lights[i], surfaceColor, viewVector, roughness, specularColor, metalness);
                break;
        }
    }
    float3 finalColor = totalLight;
    // EnvironmentMap Reflections
    //if (hasEnvironmentMap)
    //{
    //    float3 reflectionVector = reflect(-viewVector, normal); // Need camera to pixel vector, so negate
    //    float3 reflectionColor = EnvironmentMap.Sample(Sampler, reflectionVector).rgb;
	//    // Interpolate between the surface color and reflection color using a Fresnel term
    //    // Fresnel term - Schlick approx.
    //    // 
    //    // v - View vector
    //    // h - Half vector
    //    // f0 - Value when l = n
    //    //
    //    // F(v,h,f0) = f0 + (1-f0)(1 - (v dot h))^5
    //     
    //    float3 h = normalize(viewVector + normalize(-light.Direction));
    //    finalColor = lerp(totalLight, reflectionColor, F_Schlick(viewVector, viewVector, F0_NON_METAL));
    //}
    return finalColor;
}


#endif