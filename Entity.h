#pragma once
#include "Transform.h"
#include "Mesh.h"
#include "BuffStructs.h"
#include <memory>
#include "Camera.h"
#include "SimpleShader.h"
#include "Material.h"

class Entity
{
public:
	Entity(std::shared_ptr<Mesh> _mesh, std::shared_ptr<Material> _material);

	// Getters
	std::shared_ptr<Transform> GetTransform();
	std::shared_ptr<Mesh> GetMesh();
	std::shared_ptr<Material> GetMaterial();

	// Setters
	void setTransform(std::shared_ptr<Transform> _transform);
	void SetMesh(std::shared_ptr<Mesh> _mesh);
	void SetMaterial(std::shared_ptr<Material> _material);
	static void SetDefaultRastState(Microsoft::WRL::ComPtr<ID3D11RasterizerState> _defaultRastState);
	static void SetCullBackRastState(Microsoft::WRL::ComPtr<ID3D11RasterizerState> _cullBackRastState);

	// Functions
	void Draw(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context,
		std::shared_ptr<SimpleVertexShader> vs,
		std::shared_ptr<SimplePixelShader> ps,
		std::shared_ptr<Camera> camera);

	void Draw(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context,
		std::shared_ptr<Camera> camera);

private:
	static Microsoft::WRL::ComPtr<ID3D11RasterizerState> defaultRastState;
	static Microsoft::WRL::ComPtr<ID3D11RasterizerState> cullBackRastState;
	std::shared_ptr<Transform> transform;
	std::shared_ptr<Mesh> mesh;
	std::shared_ptr<Material> material;
};

