#pragma once
#include "Transform.h"
#include "Mesh.h"
#include "BuffStructs.h"
#include <memory>

class Entity
{
public:
	Entity(std::shared_ptr<Mesh> _mesh);

	//Getters
	std::shared_ptr<Transform> GetTransform();
	std::shared_ptr<Mesh> GetMesh();

	void Draw(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context,
		Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer);

private:
	std::shared_ptr<Transform> transform;
	std::shared_ptr<Mesh> mesh;
};

