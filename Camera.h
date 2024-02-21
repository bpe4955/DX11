#pragma once
#include "Input.h"
#include "Transform.h"
#include "stdexcept"

class Camera
{
public:
	Camera(float _aspectRatio, DirectX::XMFLOAT3 _position, DirectX::XMFLOAT3 _rotation,
		float _fov, float _nearDist, float _farDist, float _moveSpeed, float _mouseSens);
	Camera(float _aspectRatio, DirectX::XMFLOAT3 _position);

	DirectX::XMFLOAT4X4 GetViewMatrix();
	DirectX::XMFLOAT4X4 GetProjMatrix();

	void Update(float dt);
	/// <summary>
	/// Updates the Projection Matrix given it is Perspective
	/// </summary>
	/// <param name="_aspectRatio">Aspect Ratio of the viewport</param>
	void UpdateProjMatrix(float _aspectRatio);
	/// <summary>
	/// Updates the Projection Matrix if it's Orthographic or Perspective
	/// </summary>
	/// <param name="_aspectRatio">Aspect Ratio of the viewport</param>
	/// <param name="_viewWidth">The viewport's width</param>
	/// <param name="_viewHeight">The viewport's height</param>
	void UpdateProjMatrix(float _aspectRatio, float _viewWidth, float _viewHeight);
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
	//bool dirtyProj;
	// Customization data
	float fov;
	//float aspectRatio;
	float nearDist;
	float farDist;
	float moveSpeed;
	float mouseSens;
	float isPerspective;

	void UpdateViewMatrix();
	//void UpdateProjMatrix(float _aspectRatio, bool _isPerspective);

	void CheckInput(Input& input, float dt);


};

