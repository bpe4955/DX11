#include "ShaderIncludes.hlsli"
#include "PBR.hlsli"

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
    //Light
    float3 normal = normalize(input.normal);
    float3 tangent = normalize(input.tangent);
    float3 viewVector = normalize(cameraPosition - input.worldPosition);
	
    // (float3 normal, Light light, float3 viewVector, float specularPower, float3 worldPosition, float2 uv, float3 tangent)
    float3 totalColor = colorTint.rgb * float3(totalLight(normal, input.worldPosition, input.uv, tangent) 
    + SpotLight(normal, spotLight, viewVector, input.worldPosition, input.uv, tangent));
    //float3 totalColor = colorTint.rgb * float3(totalLight(normal, input.worldPosition, input.uv, tangent)) + (temp * 0);
    return float4(pow(totalColor, 1.0f / 2.2f), 1);
}