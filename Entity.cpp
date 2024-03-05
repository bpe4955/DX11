#include "Entity.h"

Entity::Entity(std::shared_ptr<Mesh> _mesh, std::shared_ptr<Material> _material)
{
	mesh = _mesh;
	material = _material;
	transform = std::make_shared<Transform>();
}

// Getters
std::shared_ptr<Transform> Entity::GetTransform() { return transform; }
std::shared_ptr<Mesh> Entity::GetMesh() { return mesh; }
std::shared_ptr<Material> Entity::GetMaterial() { return material; }

// Setters
void Entity::setTransform(std::shared_ptr<Transform> _transform) { transform = _transform; }
void Entity::SetMesh(std::shared_ptr<Mesh> _mesh) { mesh = _mesh; }
void Entity::SetMaterial(std::shared_ptr<Material> _material) { material = _material; }

// Functions
void Entity::Draw(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context,
	std::shared_ptr<SimpleVertexShader> vs,
	std::shared_ptr<SimplePixelShader> ps,
	std::shared_ptr<Camera> camera)
{
	SetShaders(vs, ps, camera);

	// Draw Mesh geometry
	mesh->Draw();
}

void Entity::Draw(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context,
	std::shared_ptr<Camera> camera)
{
	// Get Reference to Shaders
	std::shared_ptr<SimpleVertexShader> vs = material->GetVertShader();
	std::shared_ptr<SimplePixelShader> ps = material->GetPixelShader();

	SetShaders(vs, ps, camera);

	// Draw Mesh geometry
	mesh->Draw();
}

void Entity::SetShaders(std::shared_ptr<SimpleVertexShader> vs, std::shared_ptr<SimplePixelShader> ps, std::shared_ptr<Camera> camera)
{
	// Set active shaders
	vs->SetShader();
	ps->SetShader();

	// Provide data for vertex shader's cbuffer
	// Strings must match names in VertexShader.hlsl
	vs->SetMatrix4x4("world", transform->GetWorldMatrix());
	vs->SetMatrix4x4("worldInvTranspose", transform->GetWorldInverseTransposeMatrix());
	vs->SetMatrix4x4("view", camera->GetViewMatrix());
	vs->SetMatrix4x4("proj", camera->GetProjMatrix());

	// Copy Buffer Data to GPU
	vs->CopyAllBufferData();

	// Provide data for pixel shader's cbuffer
	// Strings must match names in PixelShader.hlsl
	ps->SetFloat4("colorTint", material->GetColorTint());
	ps->SetFloat("roughness", material->GetRoughness());
	ps->SetFloat3("cameraPosition", camera->GetPosition());

	// Copy Buffer Data to GPU
	ps->CopyAllBufferData();
}
