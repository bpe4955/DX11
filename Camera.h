#pragma once
#include "Input.h"
#include "Transform.h"

class Camera
{
public:
	// Constructors
	Camera(float _viewWidth, float _viewHeight, DirectX::XMFLOAT3 _position, DirectX::XMFLOAT3 _rotation,
		float _fov, float _nearDist, float _farDist, float _moveSpeed, float _mouseSens);
	Camera(float _viewWidth, float _viewHeight, DirectX::XMFLOAT3 _position);

	// Getters and Setters
	DirectX::XMFLOAT4X4 GetViewMatrix();
	DirectX::XMFLOAT4X4 GetProjMatrix();
	DirectX::XMFLOAT3 GetPosition();
	DirectX::XMFLOAT3 GetPitchYawRoll();
	float GetOrthoScale();
	void SetOrthoScale(float _orthoScale);
	float GetFov();
	void SetFov(float _fov);
	float GetNearDist();
	void SetNearDist(float _nearDist);
	float GetFarDist();
	void SetFarDist(float _farDist);
	float GetMoveSpeed();
	void SetMoveSpeed(float _moveSpeed);
	float GetMouseSens();
	void SetMouseSens(float _mouseSens);
	bool GetIsPerspective();


	/// <summary>
	/// Processes user input, adjusts transform, and updates the view matrix
	/// </summary>
	/// <param name="dt">Delta Time</param>
	void Update(float dt);
	/// <summary>
	/// Updates the Projection Matrix
	/// </summary>
	/// <param name="_aspectRatio">Aspect Ratio of the viewport</param>
	/// <param name="_viewWidth">The viewport's width</param>
	/// <param name="_viewHeight">The viewport's height</param>
	void UpdateProjMatrix(float _viewWidth, float _viewHeight);
	/// <summary>
	/// Changes the projection matrix between Orthographic and Perspective, then updates it
	/// </summary>
	/// <param name="_isPerspective">Determine if the Projection Matrix should be Perspective(true) or Orthographic(false)</param>
	/// <param name="_viewWidth">The viewport's width</param>
	/// <param name="_viewHeight">The viewport's height</param>
	void UpdateProjMatrix(bool _isPerspective, float _viewWidth, float _viewHeight);

private:
	// Necessary data
	Transform transform;
	DirectX::XMFLOAT4X4 viewMatrix;
	DirectX::XMFLOAT4X4 projMatrix;
	// Optimization data
	bool dirtyView;
	// Customization data
	float fov;
	float nearDist;
	float farDist;
	float moveSpeed;
	float mouseSens;
	bool isPerspective;
	float orthoScale;

	// Functions
	void UpdateViewMatrix();

	// Helper Functions
	void CheckInput(Input& input, float dt);
};

