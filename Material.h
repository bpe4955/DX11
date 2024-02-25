#pragma once
#include <DirectXMath.h>
#include <memory>
#include "SimpleShader.h"
class Material
{
public:
	Material(DirectX::XMFLOAT4 _colorTint,
		std::shared_ptr<SimpleVertexShader> _vertShader,
		std::shared_ptr<SimplePixelShader> _pixelShader);

	// Getters
	DirectX::XMFLOAT4 GetColorTint();
	std::shared_ptr<SimpleVertexShader> GetVertShader();
	std::shared_ptr<SimplePixelShader> GetPixelShader();

	// Setters
	void SetColorTint(DirectX::XMFLOAT4 _colorTint);
	void SetVertShader(std::shared_ptr<SimpleVertexShader> _vertShader);
	void SetPixelShader(std::shared_ptr<SimplePixelShader> _pixelShader);

private:
	DirectX::XMFLOAT4 colorTint;
	std::shared_ptr<SimpleVertexShader> vertShader;
	std::shared_ptr<SimplePixelShader> pixelShader;
};

