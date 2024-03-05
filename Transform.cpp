#include "Transform.h"

using namespace DirectX;

Transform::Transform() :
	translation(0, 0, 0),
	scale(1, 1, 1),
	pitchYawRoll(0, 0, 0),
	right(1, 0, 0),
	up(0, 1, 0),
	forward(0, 0, 1),
	dirtyMatrix(false),
	dirtyRotation(false)
{
	XMStoreFloat4x4(&worldMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&worldInverseTransposeMatrix, XMMatrixIdentity());
}

// Getters
DirectX::XMFLOAT3 Transform::GetPosition() { return translation; }
DirectX::XMFLOAT3 Transform::GetScale() { return scale; }
DirectX::XMFLOAT3 Transform::GetPitchYawRoll() { return pitchYawRoll; }

DirectX::XMFLOAT3 Transform::GetRight()
{
	if (dirtyRotation) { UpdateDirections(); }
	return right;
}
DirectX::XMFLOAT3 Transform::GetUp()
{
	if (dirtyRotation) { UpdateDirections(); }
	return up;
}
DirectX::XMFLOAT3 Transform::GetForward()
{
	if (dirtyRotation) { UpdateDirections(); }
	return forward;
}

DirectX::XMFLOAT4X4 Transform::GetWorldMatrix()
{
	if (dirtyMatrix) { UpdateWorldMatrix(); }
	return worldMatrix;
}
DirectX::XMFLOAT4X4 Transform::GetWorldInverseTransposeMatrix()
{
	if (dirtyMatrix) { UpdateWorldMatrix(); }
	return worldInverseTransposeMatrix;
}


// Setters
void Transform::SetPosition(float x, float y, float z)
{
	translation.x = x;
	translation.y = y;
	translation.z = z;
	dirtyMatrix = true;
}
void Transform::SetPosition(DirectX::XMFLOAT3 _position)
{
	translation = _position;
	dirtyMatrix = true;
}

void Transform::SetScale(float x, float y, float z)
{
	scale.x = x;
	scale.y = y;
	scale.z = z;
	dirtyMatrix = true;
}
void Transform::SetScale(DirectX::XMFLOAT3 _scale)
{
	scale = _scale;
	dirtyMatrix = true;
}

void Transform::SetRotation(float pitch, float yaw, float roll)
{
	pitchYawRoll.x = pitch;
	pitchYawRoll.y = yaw;
	pitchYawRoll.z = roll;
	dirtyMatrix = true;
	dirtyRotation = true;
}
void Transform::SetRotation(DirectX::XMFLOAT3 _rotation)
{
	pitchYawRoll = _rotation;
	dirtyMatrix = true;
	dirtyRotation = true;
}

//void Transform::SetRotation(DirectX::XMFLOAT4 quaternion)
//{
//}

// Transformers
void Transform::TranslateAbsolute(float x, float y, float z)
{
	translation.x += x;
	translation.y += y;
	translation.z += z;
	dirtyMatrix = true;
}
void Transform::TranslateAbsolute(DirectX::XMFLOAT3 _translation)
{
	XMVECTOR posVec = XMLoadFloat3(&translation);
	XMVECTOR offsetVec = XMLoadFloat3(&_translation);

	posVec = XMVectorAdd(posVec, offsetVec);

	XMStoreFloat3(&translation, posVec);
	dirtyMatrix = true;
}

void Transform::TranslateRelative(float x, float y, float z)
{
	XMVECTOR transVec = XMLoadFloat3(&translation);
	XMVECTOR moveVec = XMVectorSet(x, y, z, 0);
	XMVECTOR pitchYawRollVec = XMLoadFloat3(&pitchYawRoll);
	XMVECTOR quatVec = XMQuaternionRotationRollPitchYawFromVector(pitchYawRollVec);

	XMStoreFloat3(&translation, XMVectorAdd(transVec, XMVector3Rotate(moveVec, quatVec)));
	dirtyMatrix = true;
}
void Transform::TranslateRelative(DirectX::XMFLOAT3 _translation)
{
	XMVECTOR transVec = XMLoadFloat3(&translation);
	XMVECTOR moveVec = XMLoadFloat3(&_translation);
	XMVECTOR pitchYawRollVec = XMLoadFloat3(&pitchYawRoll);
	XMVECTOR quatVec = XMQuaternionRotationRollPitchYawFromVector(pitchYawRollVec);

	XMStoreFloat3(&translation, XMVectorAdd(transVec, XMVector3Rotate(moveVec, quatVec)));
	dirtyMatrix = true;
}

void Transform::Scale(float x, float y, float z)
{
	scale.x *= x;
	scale.y *= y;
	scale.z *= z;
	dirtyMatrix = true;
}
void Transform::Scale(DirectX::XMFLOAT3 _scale)
{
	XMVECTOR scaleVec = XMLoadFloat3(&scale);
	XMVECTOR offsetVec = XMLoadFloat3(&_scale);

	scaleVec = XMVectorMultiply(scaleVec, offsetVec);

	XMStoreFloat3(&scale, scaleVec);
	dirtyMatrix = true;
}

void Transform::Rotate(float p, float y, float r)
{
	pitchYawRoll.x += p;
	pitchYawRoll.y += y;
	pitchYawRoll.z += r;
	dirtyMatrix = true;
	dirtyRotation = true;
}
void Transform::Rotate(DirectX::XMFLOAT3 _rotation)
{
	XMVECTOR rotVec = XMLoadFloat3(&pitchYawRoll);
	XMVECTOR offsetVec = XMLoadFloat3(&_rotation);

	rotVec = XMVectorAdd(rotVec, offsetVec);

	XMStoreFloat3(&pitchYawRoll, rotVec);
	dirtyMatrix = true;
	dirtyRotation = true;
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
	XMStoreFloat4x4(&worldInverseTransposeMatrix,
		XMMatrixInverse(0, XMMatrixTranspose(worldMat)));

	dirtyMatrix = false;
}

void Transform::UpdateDirections()
{
	// Load values for math
	XMVECTOR r = XMVectorSet(1, 0, 0, 0);
	XMVECTOR u = XMVectorSet(0, 1, 0, 0);
	XMVECTOR f = XMVectorSet(0, 0, 1, 0);
	XMVECTOR quatVec = XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3(&pitchYawRoll));

	// Store values
	XMStoreFloat3(&right, XMVector3Rotate(r, quatVec));
	XMStoreFloat3(&up, XMVector3Rotate(u, quatVec));
	XMStoreFloat3(&forward, XMVector3Rotate(f, quatVec));
}
