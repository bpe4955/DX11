#include "Camera.h"

using namespace DirectX;

// Constructors
Camera::Camera(float _aspectRatio, DirectX::XMFLOAT3 _position, DirectX::XMFLOAT3 _rotation,
	float _fov, float _nearDist, float _farDist, float _moveSpeed, float _mouseSens) :
	fov(_fov),
	nearDist(_nearDist),
	farDist(_farDist),
	moveSpeed(_moveSpeed),
	mouseSens(_mouseSens),
	viewWidth(1280),
	viewHeight(720),
	isPerspective(true),
	dirtyView(true)
{
	transform = Transform();
	transform.SetPosition(_position);
	transform.SetRotation(_rotation);
	UpdateViewMatrix();
	UpdateProjMatrix(_aspectRatio);
}

Camera::Camera(float _aspectRatio, DirectX::XMFLOAT3 _position)
	: Camera(_aspectRatio, _position, DirectX::XMFLOAT3(0, 0, 0),
		DirectX::XM_PIDIV2, 0.01f, 750.0f, 2.0f, 0.075f) {}

//Getters
DirectX::XMFLOAT4X4 Camera::GetViewMatrix()
{
	if (dirtyView) { UpdateViewMatrix(); }
	return viewMatrix;
}
DirectX::XMFLOAT4X4 Camera::GetProjMatrix()
{
	return projMatrix;
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

void Camera::UpdateProjMatrix(float _aspectRatio)
{
	if (isPerspective) { UpdateProjMatrix(_aspectRatio, viewWidth, viewHeight); }
	else { throw std::invalid_argument("Camera must be perspective to use this function"); }
}
void Camera::UpdateProjMatrix(float _aspectRatio, float _viewWidth, float _viewHeight)
{
	viewWidth = _viewWidth;
	viewHeight = _viewHeight;
	if (isPerspective) {
		XMStoreFloat4x4(&projMatrix,
			XMMatrixPerspectiveFovLH(fov, _aspectRatio, nearDist, farDist));
	}
	else {
		XMStoreFloat4x4(&projMatrix,
			XMMatrixOrthographicLH(viewWidth, viewHeight, nearDist, farDist));
	}
}
void Camera::UpdateProjMatrix(bool _isPerspective, float _viewWidth, float _viewHeight)
{
	isPerspective = _isPerspective;
	viewWidth = _viewWidth;
	viewHeight = _viewHeight;
	if (isPerspective) {
		XMStoreFloat4x4(&projMatrix,
			XMMatrixPerspectiveFovLH(fov, viewWidth/viewHeight, nearDist, farDist));
	}
	else {
		XMStoreFloat4x4(&projMatrix,
			XMMatrixOrthographicLH(viewWidth, viewHeight, nearDist, farDist));
	}
}

/// <summary>
/// Processes user input, adjusts transform, and updates the view matrix
/// </summary>
/// <param name="dt">Delta Time</param>
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
	// Only check specific keys if a key has been pressed
	//bool anyKey;
	//if (input.GetKeyArray(&anyKey)) { 
		// Movement
		float b = 1;
		if (input.KeyDown(VK_CONTROL)) { b = 2; }
		if (input.KeyDown('W')) { transform.TranslateRelative(0, 0, moveSpeed * dt * b); }
		if (input.KeyDown('S')) { transform.TranslateRelative(0, 0, -moveSpeed * dt * b); }
		if (input.KeyDown('A')) { transform.TranslateRelative(-moveSpeed * dt * b, 0, 0); }
		if (input.KeyDown('D')) { transform.TranslateRelative(moveSpeed * dt * b, 0, 0); }
		if (input.KeyDown(VK_SHIFT)) { transform.TranslateAbsolute(0, -moveSpeed * dt * b, 0); }
		if (input.KeyDown(VK_SPACE)) { transform.TranslateAbsolute(0, moveSpeed * dt * b, 0); }
		// If any key was pressed, assume the view is dirty
		dirtyView = true; 
	//}

	// Rotation
	if (input.MouseLeftDown()) {
		int cursorMovementX = input.GetMouseXDelta();
		int cursorMovementY = input.GetMouseYDelta();

		transform.Rotate(0, cursorMovementX * mouseSens, 0);
		// Clamp the pitch rotation
		transform.Rotate(cursorMovementY * mouseSens, 0, 0);
		if (transform.GetPitchYawRoll().x > DirectX::XM_PIDIV2)
		{
			transform.SetRotation(DirectX::XM_PIDIV2, transform.GetPitchYawRoll().y, 0);
		}
		else if (transform.GetPitchYawRoll().x < -DirectX::XM_PIDIV2)
		{
			transform.SetRotation(-DirectX::XM_PIDIV2, transform.GetPitchYawRoll().y, 0);
		}
	}
}
