#include "ShaderIncludes.hlsli"

cbuffer DataFromCPU : register(b0)
{
    float4 colorTint;
}

TextureCube skyTexture : register(t0);
SamplerState Sampler : register(s0);

float4 main(VertexToPixel_Sky input) : SV_TARGET
{
    return skyTexture.Sample(Sampler, input.sampleDir) * colorTint;
}