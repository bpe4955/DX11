#include "Entity.h"

// Static
Microsoft::WRL::ComPtr<ID3D11RasterizerState> Entity::defaultRastState;
Microsoft::WRL::ComPtr<ID3D11RasterizerState> Entity::cullBackRastState;

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
void Entity::SetDefaultRastState(Microsoft::WRL::ComPtr<ID3D11RasterizerState> _defaultRastState) { defaultRastState = _defaultRastState; }
void Entity::SetCullBackRastState(Microsoft::WRL::ComPtr<ID3D11RasterizerState> _cullBackRastState) { cullBackRastState = _cullBackRastState; }

// Functions
void Entity::Draw(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context,
	std::shared_ptr<SimpleVertexShader> vs,
	std::shared_ptr<SimplePixelShader> ps,
	std::shared_ptr<Camera> camera)
{
	// Prepare the shaders
	material->PrepareMaterial(transform.get(), camera);

	// Draw Mesh geometry
	mesh->Draw();
}

void Entity::Draw(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context,
	std::shared_ptr<Camera> camera)
{
	// Prepare the shaders
	material->PrepareMaterial(transform.get(), camera);
	bool isTransparent = material->GetTransparency() != 1.0f;

	// Draw Mesh geometry
	if (isTransparent) { context->RSSetState(cullBackRastState.Get()); }
	mesh->Draw();
	if (isTransparent) { context->RSSetState(defaultRastState.Get()); }
}

