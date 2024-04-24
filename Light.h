#pragma once

#include <DirectXMath.h>

#define LIGHT_TYPE_DIR   0
#define LIGHT_TYPE_POINT 1
#define LIGHT_TYPE_SPOT  2
#define MAX_NUM_LIGHTS 10

struct Light
{
    int Type;
    DirectX::XMFLOAT3 Direction;
    float Range;
    DirectX::XMFLOAT3 Position;
    float Intensity;
    DirectX::XMFLOAT3 Color;
    float SpotFalloff;
    float Fov;
    DirectX::XMFLOAT2 Padding;
};
