#include "ShaderIncludes.hlsli"

/// <summary>
/// Layout of our constant buffer
/// </summary>
cbuffer DataFromCPU : register(b0)
{
    float4 colorTint;
    float totalTime;
}


float random(float2 s)
{
    return frac(sin(dot(s, float2(12.9898, 78.233))) * 43758.5453123);
}


float4 main(VertexToPixel input) : SV_TARGET
{
	// Custom
    float intensity = 100;
    float y = sin(input.uv[0] * intensity * 2);
    float x = sin(input.uv[1] * intensity);
    float z = tan((x / y) * intensity);
    float3 xyz = float3(x, y, z);
    float3 pulse = float3(1, 1, (tan(x / y) * tan(totalTime)));
    
    
    return float4(xyz * pulse, 1) * colorTint;
}