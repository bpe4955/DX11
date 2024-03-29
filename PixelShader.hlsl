#include "ShaderIncludes.hlsli"
#include "Lighting&Texturing.hlsli"


/// <summary>
/// Layout of our constant buffer
/// </summary>
cbuffer DataFromCPU : register(b1)
{
    float totalTime;
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
    float3 mask = float3(x * s, y * s, s);
    
    return float4(rgb * mask, 1) * colorTint * float4(totalLight(input.normal, input.worldPosition, input.uv, input.tangent), 1);
}