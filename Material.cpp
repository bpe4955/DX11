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
