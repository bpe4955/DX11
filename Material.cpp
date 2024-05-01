#include "Material.h"

// Constructor
Material::Material(DirectX::XMFLOAT4 _colorTint, float _roughness,
	std::shared_ptr<SimpleVertexShader> _vertShader, std::shared_ptr<SimplePixelShader> _pixelShader) :
	colorTint(_colorTint),
	roughness(_roughness),
	transparency(1.0f),
	vertShader(_vertShader),
	pixelShader(_pixelShader) 
{
	if (roughness < 0.0f) { roughness = 0.0f; }
	else if (roughness > 1.0f) { roughness = 1.0f; }
	uvOffset = DirectX::XMFLOAT2(0.0f, 0.0f);
	uvScale = DirectX::XMFLOAT2(1.0f, 1.0f);
}

// Getters
DirectX::XMFLOAT4 Material::GetColorTint() { return colorTint; }
float Material::GetRoughness() { return roughness; }
std::shared_ptr<SimpleVertexShader> Material::GetVertShader() { return vertShader; }
std::shared_ptr<SimplePixelShader> Material::GetPixelShader() { return pixelShader; }
bool Material::HasTextureSRV(std::string name) { return textureSRVs.find(name) != textureSRVs.end(); }
float Material::GetTransparency() { return transparency; }

// Setters
void Material::SetColorTint(DirectX::XMFLOAT4 _colorTint) { colorTint = _colorTint; }
void Material::SetRoughness(float _roughness) {
	roughness = _roughness; 
	if (roughness < 0.0f) { roughness = 0.0f; }
	else if (roughness > 1.0f) { roughness = 1.0f; }
; }
void Material::SetVertShader(std::shared_ptr<SimpleVertexShader> _vertShader) { vertShader = _vertShader; }
void Material::SetPixelShader(std::shared_ptr<SimplePixelShader> _pixelShader) { pixelShader = _pixelShader; }
void Material::SetUVOffset(DirectX::XMFLOAT2 _uvOffset) { uvOffset = _uvOffset; }
void Material::AddUVOffset(DirectX::XMFLOAT2 _uvOffset) { uvOffset = DirectX::XMFLOAT2(uvOffset.x + _uvOffset.x, uvOffset.y + _uvOffset.y); }
void Material::SetUVScale(DirectX::XMFLOAT2 _uvScale) { uvScale = _uvScale; }
void Material::AddTextureSRV(std::string shaderVariableName, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv) 
{
	textureSRVs.insert({ shaderVariableName, srv });
}
void Material::AddSampler(std::string samplerVariableName, Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler)
{
	samplers.insert({ samplerVariableName, sampler });
}
void Material::SetTransparency(float _transparency) { transparency = _transparency >= 0.0f && _transparency <= 1.0f ? _transparency : 1.0f; }


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
	if (pixelShader->HasVariable("hasSpecMap"))
	{
		bool hasSpecMap = textureSRVs.count("SpecularMap") != 0;
		pixelShader->SetData("hasSpecMap", &hasSpecMap, sizeof(bool));
	}
	if (pixelShader->HasVariable("hasRoughMap"))
	{
		bool hasRoughMap = textureSRVs.count("RoughnessMap") != 0;
		pixelShader->SetData("hasRoughMap", &hasRoughMap, sizeof(bool));
	}
	if (pixelShader->HasVariable("hasMetalMap"))
	{
		bool hasMetalMap = textureSRVs.count("MetalnessMap") != 0;
		pixelShader->SetData("hasMetalMap", &hasMetalMap, sizeof(bool));
	}
	if (pixelShader->HasVariable("hasOpacityMap"))
	{
		bool hasOpacityMap = textureSRVs.count("OpacityMap") != 0;
		pixelShader->SetData("hasOpacityMap", &hasOpacityMap, sizeof(bool));
	}
	//if (pixelShader->HasVariable("hasShadowMap"))
	//{
	//	bool hasShadowMap = textureSRVs.count("ShadowMap") != 0;
	//	pixelShader->SetData("hasShadowMap", &hasShadowMap, sizeof(bool));
	//}
	bool hasMask = textureSRVs.count("TextureMask") != 0;
	pixelShader->SetData("hasMask", &hasMask, sizeof(bool));
	bool hasNormalMap = textureSRVs.count("NormalMap") != 0;
	pixelShader->SetData("hasNormalMap", &hasNormalMap, sizeof(bool));
	bool hasEnvironmentMap = textureSRVs.count("EnvironmentMap") != 0;
	pixelShader->SetData("hasEnvironmentMap", &hasEnvironmentMap, sizeof(bool));
	pixelShader->SetFloat("roughness", roughness);
	pixelShader->SetFloat("transparency", transparency);
	pixelShader->SetFloat2("uvOffset", uvOffset);
	pixelShader->SetFloat2("uvScale", uvScale);

	for (auto& t : textureSRVs) { pixelShader->SetShaderResourceView(t.first.c_str(), t.second); }
	for (auto& s : samplers) { pixelShader->SetSamplerState(s.first.c_str(), s.second); }


	pixelShader->CopyAllBufferData();
}
