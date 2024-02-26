#include "ShaderIncludes.hlsli"

/// <summary>
/// Layout of our constant buffer
/// </summary>
cbuffer DataFromCPU : register(b0)
{
	float4 colorTint;
}

// --------------------------------------------------------
// The entry point (main method) for our pixel shader
// 
// - Input is the data coming down the pipeline (defined by the struct)
// - Output is a single color (float4)
// - Has a special semantic (SV_TARGET), which means 
//    "put the output of this into the current render target"
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
float4 main(VertexToPixel input) : SV_TARGET
{
	// Just return the input color
	// - This color (like most values passing through the rasterizer) is 
	//   interpolated for each pixel between the corresponding vertices 
	//   of the triangle we're rendering
	//return input.color;

	float x = sin(input.screenPosition.x / 10.0f);
	float y = cos(input.screenPosition.y / 10.0f);

	float2 target = float2(200, 200);
	float2 pixelCoord = input.screenPosition.xy;

	float dist = distance(target, pixelCoord);

	float s = sin(dist / 10.0f);
	return float4(x*s, y*s, s, 1);
}