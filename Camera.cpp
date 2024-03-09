#include "Camera.h"

using namespace DirectX;

// Constructors
Camera::Camera(float _viewWidth, float _viewHeight, DirectX::XMFLOAT3 _position, DirectX::XMFLOAT3 _rotation,
	float _fov, float _nearDist, float _farDist, float _moveSpeed, float _mouseSens) :
	fov(_fov),
	nearDist(_nearDist),
	farDist(_farDist),
	moveSpeed(_moveSpeed),
	mouseSens(_mouseSens),
	isPerspective(true),
	orthoScale(1.0f / 350.0f),
	dirtyView(true)
{
	transform = Transform();
	transform.SetPosition(_position);
	transform.SetRotation(_rotation);
	UpdateViewMatrix();
	UpdateProjMatrix(_viewWidth, _viewHeight);
}
Camera::Camera(float _viewWidth, float _viewHeight, DirectX::XMFLOAT3 _position)
	: Camera(_viewWidth, _viewHeight, _position, DirectX::XMFLOAT3(0, DirectX::XM_PI, 0),
		DirectX::XM_PIDIV2, 0.01f, 750.0f, 2.0f, 0.015f) {}

// Getters
DirectX::XMFLOAT4X4 Camera::GetViewMatrix()
{
	if (dirtyView) { UpdateViewMatrix(); }
	return viewMatrix;
}
DirectX::XMFLOAT4X4 Camera::GetProjMatrix()
{
	return projMatrix;
}
DirectX::XMFLOAT3 Camera::GetPosition() { return XMFLOAT3(transform.GetPosition()); }
DirectX::XMFLOAT3 Camera::GetPitchYawRoll() { return XMFLOAT3(transform.GetPitchYawRoll()); }
float Camera::GetOrthoScale()
{
	return orthoScale;
}
float Camera::GetFov() { return fov; }
float Camera::GetNearDist() { return nearDist; }
float Camera::GetFarDist() { return farDist; }
float Camera::GetMoveSpeed() { return moveSpeed; }
float Camera::GetMouseSens() { return mouseSens; }
bool Camera::GetIsPerspective() { return isPerspective; }
Transform Camera::GetTransform() { return transform; }

// Setters - Clamped
void Camera::SetOrthoScale(float _orthoScale)
{
	if (_orthoScale > 1) { orthoScale = 1 / _orthoScale; }
	else { orthoScale = _orthoScale; }
}
void Camera::SetFov(float _fov)
{
	if (_fov < XM_PIDIV4) { fov = XM_PIDIV4; return; }
	if (_fov > XM_PIDIV2) { fov = XM_PIDIV2; return; }
	fov = _fov;
}
void Camera::SetNearDist(float _nearDist)
{
	if (_nearDist < 0.005f) { nearDist = 0.005f; return; }
	if (_nearDist >= farDist) { nearDist = farDist-1; return; }
	nearDist = _nearDist;
}
void Camera::SetFarDist(float _farDist)
{
	if (_farDist <= nearDist) { farDist = nearDist + 1; return; }
	if (_farDist > 1500.0f) { farDist = 1500.0f; return; }
	farDist = _farDist;
}
void Camera::SetMoveSpeed(float _moveSpeed)
{
	if (_moveSpeed < 0.1f) { moveSpeed = 0.1f; return; }
	moveSpeed = _moveSpeed;
}
void Camera::SetMouseSens(float _mouseSens)
{
	if (_mouseSens < 0.001f) { mouseSens = 0.001f; return; }
	if (_mouseSens > 0.1f) { mouseSens = 0.1f; return; }
	mouseSens = _mouseSens;
}



// Update
void Camera::UpdateViewMatrix()
{
	XMFLOAT3 position = transform.GetPosition();
	XMFLOAT3 forward = transform.GetForward();
	//XMFLOAT3 up = transform.GetUp();
	XMFLOAT3 up(0, 1, 0);
	XMStoreFloat4x4(&viewMatrix,
		XMMatrixLookToLH(XMLoadFloat3(&position),
			XMLoadFloat3(&forward),
			XMLoadFloat3(&up)));
	// Potentially change the GetUp to just always be (0,1,0)
	dirtyView = false;
}

void Camera::UpdateProjMatrix(float _viewWidth, float _viewHeight)
{
	if (isPerspective) {
		XMStoreFloat4x4(&projMatrix,
			XMMatrixPerspectiveFovLH(fov, _viewWidth / _viewHeight, nearDist, farDist));
	}
	else {
		XMStoreFloat4x4(&projMatrix,
			XMMatrixOrthographicLH(_viewWidth * orthoScale, _viewHeight * orthoScale, nearDist, farDist));
	}
}
void Camera::UpdateProjMatrix(bool _isPerspective, float _viewWidth, float _viewHeight)
{
	isPerspective = _isPerspective;
	UpdateProjMatrix(_viewWidth, _viewHeight);
}

void Camera::Update(float dt)
{
	// Get User Input
	Input& input = Input::GetInstance();
	CheckInput(input, dt);
	// Update the view matrix
	if (dirtyView) { UpdateViewMatrix(); }
}

void Camera::CheckInput(Input& input, float dt)
{
	// Movement
	float b = 1;
	if (input.KeyDown(VK_CONTROL)) { b = 3; }
	if (input.KeyDown('W')) { transform.TranslateRelative(0, 0, moveSpeed * dt * b); dirtyView = true; }
	if (input.KeyDown('S')) { transform.TranslateRelative(0, 0, -moveSpeed * dt * b); dirtyView = true; }
	if (input.KeyDown('A')) { transform.TranslateRelative(-moveSpeed * dt * b, 0, 0); dirtyView = true; }
	if (input.KeyDown('D')) { transform.TranslateRelative(moveSpeed * dt * b, 0, 0); dirtyView = true; }
	if (input.KeyDown(VK_SHIFT)) { transform.TranslateAbsolute(0, -moveSpeed * dt * b, 0);  dirtyView = true; }
	if (input.KeyDown(VK_SPACE)) { transform.TranslateAbsolute(0, moveSpeed * dt * b, 0); dirtyView = true; }

	// Rotation
	if (input.MouseLeftDown()) {
		dirtyView = true;
		int cursorMovementX = input.GetMouseXDelta();
		int cursorMovementY = input.GetMouseYDelta();

		transform.Rotate(0, cursorMovementX * mouseSens, 0);
		// Clamp the pitch rotation
		transform.Rotate(cursorMovementY * mouseSens, 0, 0);
		if (transform.GetPitchYawRoll().x > DirectX::XM_PIDIV2)
		{
			transform.SetRotation(DirectX::XM_PIDIV2-0.0001f, transform.GetPitchYawRoll().y, 0);
		}
		else if (transform.GetPitchYawRoll().x < -DirectX::XM_PIDIV2)
		{
			transform.SetRotation(-DirectX::XM_PIDIV2+0.0001f, transform.GetPitchYawRoll().y, 0);
		}
	}
}
