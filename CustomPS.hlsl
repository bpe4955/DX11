#include "ShaderIncludes.hlsli"
#include "Lighting.hlsli"

/// <summary>
/// Layout of our constant buffer
/// </summary>
cbuffer DataFromCPU : register(b1)
{
    Light spotLight;
	float totalTime;
}

float4 main(VertexToPixel input) : SV_TARGET
{
	// Custom
	float intensity = 10;
	float y = sin(input.uv[0] * intensity * 2);
	float x = sin(input.uv[1] * intensity);
	float z = tan((x / y) * intensity);
	float3 xyz = float3(x, y, z);
	float pulse = tan((x) / y + totalTime);
	
	//Light
    float3 normal = normalize(input.normal);
    float3 viewVector = normalize(cameraPosition - input.worldPosition);
    float specularPower = (1.0f - roughness) * MAX_SPECULAR_EXPONENT;
	
    return colorTint * float4(totalLight(input.normal, input.worldPosition, input.uv) + SpotLight(normal, spotLight, viewVector, specularPower, input.worldPosition, input.uv), 1);
    //return float4(pulse.rrr+colorTint.xyz+xyz, 1) * float4(totalLight(input.normal, input.worldPosition), 1);
}