#pragma once

#include <DirectXMath.h>

class Transform
{
public:
	Transform();
	// No need for destructor

	// Getters
	DirectX::XMFLOAT3 GetPosition();
	DirectX::XMFLOAT3 GetScale();
	DirectX::XMFLOAT3 GetPitchYawRoll();
	DirectX::XMFLOAT4X4 GetWorldMatrix();

	// Setters
	void SetPosition(float x, float y, float z);
	void SetScale(float x, float y, float z);
	void SetRotation(float pitch, float yaw, float roll);

	// Transformers
	void TranslateAbsolute(float x, float y, float z);
	void TranslateAbsolute(DirectX::XMFLOAT3 _translation);
	void TranslateRelative(float x, float y, float z);
	void TranslateRelative(DirectX::XMFLOAT3 _translation);
	void Scale(float x, float y, float z);
	void Scale(DirectX::XMFLOAT3 _scale);
	void Rotate(float p, float y, float r);
	void Rotate(DirectX::XMFLOAT3 _rotation);

private:
	bool hasChanged;
	//Raw Transform Data
	DirectX::XMFLOAT3 translation;
	DirectX::XMFLOAT3 scale;
	DirectX::XMFLOAT3 pitchYawRoll;
	//DirectX::XMFLOAT4 rotationQuaternion;

	// Combined into a single matrix
	DirectX::XMFLOAT4X4 worldMatrix;

	void UpdateWorldMatrix();
};

