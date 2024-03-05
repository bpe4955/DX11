#pragma once

#include <DirectXMath.h>

struct Light
{
    int Type;
    DirectX::XMFLOAT3 Direction;
    float Range;
    DirectX::XMFLOAT3 Position;
    float Intensity;
    DirectX::XMFLOAT3 Color;
    float SpotFalloff;
    DirectX::XMFLOAT3 Padding;
};
