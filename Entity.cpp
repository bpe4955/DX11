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
	Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer)
{
	//Use transform matrix to fill ConstBuff
	VertexShaderData vsData;
	vsData.world = transform->GetWorldMatrix();
	vsData.colorTint = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	D3D11_MAPPED_SUBRESOURCE mappedBuffer = {};
	context->Map(constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuffer);
	memcpy(mappedBuffer.pData, &vsData, sizeof(vsData));
	context->Unmap(constantBuffer.Get(), 0);
	//Draw Mesh geometry
	mesh->Draw();
}
