#include "Transform.h"

using namespace DirectX;

Transform::Transform() :
    translation(0, 0, 0),
    scale(1, 1, 1),
    pitchYawRoll(0, 0, 0),
    hasChanged(false)
{
    XMStoreFloat4x4(&worldMatrix, XMMatrixIdentity());
}

// Getters
DirectX::XMFLOAT3 Transform::GetPosition() { return translation; }
DirectX::XMFLOAT3 Transform::GetScale() { return scale; }
DirectX::XMFLOAT3 Transform::GetPitchYawRoll() { return pitchYawRoll; }

DirectX::XMFLOAT4X4 Transform::GetWorldMatrix()
{
    if(hasChanged) { 
        UpdateWorldMatrix(); 
        hasChanged = false;
    }
    return worldMatrix;
}

// Setters
void Transform::SetPosition(float x, float y, float z)
{
    translation.x = x;
    translation.y = y;
    translation.z = z;
    hasChanged = true;
}

void Transform::SetScale(float x, float y, float z)
{
    scale.x = x;
    scale.y = y;
    scale.z = z;
    hasChanged = true;
}

void Transform::SetRotation(float pitch, float yaw, float roll)
{
    pitchYawRoll.x = pitch;
    pitchYawRoll.y = yaw;
    pitchYawRoll.z = roll;
    hasChanged = true;
}

// Transformers
void Transform::TranslateAbsolute(float x, float y, float z)
{
    translation.x += x;
    translation.y += y;
    translation.z += z;
    hasChanged = true;
}

void Transform::TranslateAbsolute(DirectX::XMFLOAT3 _translation)
{
}

void Transform::TranslateRelative(float x, float y, float z)
{
}

void Transform::TranslateRelative(DirectX::XMFLOAT3 _translation)
{
}

void Transform::Scale(float x, float y, float z)
{
    scale.x *= x;
    scale.y *= y;
    scale.z *= z;
    hasChanged = true;
}

void Transform::Scale(DirectX::XMFLOAT3 _scale)
{
}

void Transform::Rotate(float p, float y, float r)
{
    pitchYawRoll.x += p;
    pitchYawRoll.y += y;
    pitchYawRoll.z += r;
    hasChanged = true;
}

void Transform::Rotate(DirectX::XMFLOAT3 _rotation)
{
}

// Update
void Transform::UpdateWorldMatrix()
{
    // Create the three matrices that make up the world matrix
    XMMATRIX t = XMMatrixTranslationFromVector(XMLoadFloat3(&translation));
    XMMATRIX s = XMMatrixScalingFromVector(XMLoadFloat3(&scale));
    XMMATRIX r = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&pitchYawRoll));

    // Combine into a single world matrix
    XMMATRIX worldMat = s * r * t;
    XMStoreFloat4x4(&worldMatrix, worldMat);
}
