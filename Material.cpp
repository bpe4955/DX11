#include "Material.h"

// Constructor
Material::Material(DirectX::XMFLOAT4 _colorTint, float _roughness,
	std::shared_ptr<SimpleVertexShader> _vertShader, std::shared_ptr<SimplePixelShader> _pixelShader) :
	colorTint(_colorTint),
	roughness(_roughness),
	vertShader(_vertShader),
	pixelShader(_pixelShader) 
{
	if (roughness < 0.0f) { roughness = 0.0f; }
	else if (roughness > 1.0f) { roughness = 1.0f; }
}

// Getters
DirectX::XMFLOAT4 Material::GetColorTint() { return colorTint; }
float Material::GetRoughness() { return roughness; }
std::shared_ptr<SimpleVertexShader> Material::GetVertShader() { return vertShader; }
std::shared_ptr<SimplePixelShader> Material::GetPixelShader() { return pixelShader; }

// Setters
void Material::SetColorTint(DirectX::XMFLOAT4 _colorTint) { colorTint = _colorTint; }
void Material::SetRoughness(float _roughness) {
	roughness = _roughness; 
	if (roughness < 0.0f) { roughness = 0.0f; }
	else if (roughness > 1.0f) { roughness = 1.0f; }
; }
void Material::SetVertShader(std::shared_ptr<SimpleVertexShader> _vertShader) { vertShader = _vertShader; }
void Material::SetPixelShader(std::shared_ptr<SimplePixelShader> _pixelShader) { pixelShader = _pixelShader; }
void Material::AddTextureSRV(std::string shaderVariableName, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv) 
{
	textureSRVs.insert({ shaderVariableName, srv });
}
void Material::AddSampler(std::string samplerVariableName, Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler)
{
	samplers.insert({ samplerVariableName, sampler });
}


// Functions
void Material::PrepareMaterial(Transform* transform, std::shared_ptr<Camera> camera)
{
	// Set active shaders
	vertShader->SetShader();
	pixelShader->SetShader();

	// Provide data for vertex shader's cbuffer
	// Strings must match names in VertexShader.hlsl
	vertShader->SetMatrix4x4("world", transform->GetWorldMatrix());
	vertShader->SetMatrix4x4("worldInvTranspose", transform->GetWorldInverseTransposeMatrix());
	vertShader->SetMatrix4x4("view", camera->GetViewMatrix());
	vertShader->SetMatrix4x4("proj", camera->GetProjMatrix());

	// Copy Buffer Data to GPU
	vertShader->CopyAllBufferData();


	// Pixel Shader
	// Provide data for pixel shader's cbuffer
	// Strings must match names in PixelShader.hlsl
	pixelShader->SetFloat3("cameraPosition", camera->GetPosition());
	pixelShader->SetFloat4("colorTint", colorTint);
	bool hasSpecMap = textureSRVs.count("SpecularMap") != 0;
	pixelShader->SetData("hasSpecMap", &hasSpecMap, sizeof(bool));
	pixelShader->SetFloat("roughness", roughness);

	for (auto& t : textureSRVs) { pixelShader->SetShaderResourceView(t.first.c_str(), t.second); }
	for (auto& s : samplers) { pixelShader->SetSamplerState(s.first.c_str(), s.second); }


	pixelShader->CopyAllBufferData();
}
