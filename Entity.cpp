#include "Entity.h"

Entity::Entity(std::shared_ptr<Mesh> _mesh)
{
	mesh = _mesh;
	transform = std::make_shared<Transform>();
}

// Getters
std::shared_ptr<Transform> Entity::GetTransform() { return transform; }
std::shared_ptr<Mesh> Entity::GetMesh() { return mesh; }

void Entity::Draw(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context,
	std::shared_ptr<SimpleVertexShader> vs,
	std::shared_ptr<SimplePixelShader> ps,
	std::shared_ptr<Camera> camera)
{
	// Set active shaders
	vs->SetShader();
	ps->SetShader();

	// Provide data for vertex shader's cbuffer
	vs->SetFloat4("color", DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
	vs->SetMatrix4x4("world", transform->GetWorldMatrix());
	vs->SetMatrix4x4("view", camera->GetViewMatrix());
	vs->SetMatrix4x4("proj", camera->GetProjMatrix());

	// Copy Buffer Data to GPU
	vs->CopyAllBufferData();

	// Draw Mesh geometry
	mesh->Draw();
}
