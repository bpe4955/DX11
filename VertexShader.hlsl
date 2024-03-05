#include "ShaderIncludes.hlsli"

/// <summary>
/// Layout of our constant buffer
/// </summary>
cbuffer DataFromCPU: register(b0)
{
    matrix world;
    matrix worldInvTranspose;
    matrix view;
    matrix proj;
}

//	cbuffer FrameData: register(b1)
//	matrix view
//	matrix proj

// --------------------------------------------------------
// The entry point (main method) for our vertex shader
// 
// - Input is exactly one vertex worth of data (defined by a struct)
// - Output is a single struct of data to pass down the pipeline
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
VertexToPixel main( VertexShaderInput input )
{
	// Set up output struct
	VertexToPixel output;
	
    matrix mvp = mul(proj, mul(view, world));
	
	output.screenPosition = mul(mvp, float4(input.localPosition, 1.0f));
	output.uv = input.uv;
    output.normal = mul((float3x3)worldInvTranspose, input.normal);
    output.worldPosition = mul(world, float4(input.localPosition, 1)).xyz;
	
	return output;
}