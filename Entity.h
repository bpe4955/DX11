#pragma once
#include "Transform.h"
#include "Mesh.h"
#include "BuffStructs.h"
#include <memory>
#include "Camera.h"
#include "SimpleShader.h"

class Entity
{
public:
	Entity(std::shared_ptr<Mesh> _mesh);

	//Getters
	std::shared_ptr<Transform> GetTransform();
	std::shared_ptr<Mesh> GetMesh();

	void Draw(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context,
		std::shared_ptr<SimpleVertexShader> vs,
		std::shared_ptr<SimplePixelShader> ps,
		std::shared_ptr<Camera> camera);

private:
	std::shared_ptr<Transform> transform;
	std::shared_ptr<Mesh> mesh;
};

