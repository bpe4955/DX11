#include "Material.h"

// Constructor
Material::Material(DirectX::XMFLOAT4 _colorTint, std::shared_ptr<SimpleVertexShader> _vertShader, std::shared_ptr<SimplePixelShader> _pixelShader) :
	colorTint(_colorTint),
	vertShader(_vertShader),
	pixelShader(_pixelShader) {}

// Getters
DirectX::XMFLOAT4 Material::GetColorTint() { return colorTint; }
std::shared_ptr<SimpleVertexShader> Material::GetVertShader() { return vertShader; }
std::shared_ptr<SimplePixelShader> Material::GetPixelShader() { return pixelShader; }

// Setters
void Material::SetColorTint(DirectX::XMFLOAT4 _colorTint) { colorTint = _colorTint; }
void Material::SetVertShader(std::shared_ptr<SimpleVertexShader> _vertShader) { vertShader = _vertShader; }
void Material::SetPixelShader(std::shared_ptr<SimplePixelShader> _pixelShader) { pixelShader = _pixelShader; }
