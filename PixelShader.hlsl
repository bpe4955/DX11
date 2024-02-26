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
	// New
    float ran = random(input.uv);
    float3 noise = float3(-ran, -ran, ran);
    float r = sin(totalTime);
    float g = cos(totalTime);
    float b = tan(totalTime);
    float3 rgb = float3(r, g, b) + noise;
    
    // Previous
    float x = sin(input.screenPosition.x / 10.0f);
    float y = cos(input.screenPosition.y / 10.0f);
    float2 target = float2(800, 800);
    float2 pixelCoord = input.screenPosition.xy;
    float dist = distance(target*input.uv*input.uv, pixelCoord);
    float s = sin(dist / 10.0f);
    float3 mask = (x * s, y * s, s);
    
    return float4(rgb * mask, 1) * colorTint;
}