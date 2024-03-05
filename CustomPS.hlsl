#include "ShaderIncludes.hlsli"
#include "Lighting.hlsli"

/// <summary>
/// Layout of our constant buffer
/// </summary>
cbuffer DataFromCPU : register(b1)
{
    float totalTime;
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
    
    
    return colorTint * float4(totalLight(input.normal, input.worldPosition), 1);
}