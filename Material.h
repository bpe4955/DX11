#pragma once
#include <DirectXMath.h>
#include <memory>
#include "SimpleShader.h"
#include <string>
#include "Transform.h"
#include "Camera.h"
class Material
{
public:
	Material(DirectX::XMFLOAT4 _colorTint,
		float _roughness,
		std::shared_ptr<SimpleVertexShader> _vertShader,
		std::shared_ptr<SimplePixelShader> _pixelShader);

	// Getters
	DirectX::XMFLOAT4 GetColorTint();
	float GetRoughness();
	std::shared_ptr<SimpleVertexShader> GetVertShader();
	std::shared_ptr<SimplePixelShader> GetPixelShader();

	// Setters
	void SetColorTint(DirectX::XMFLOAT4 _colorTint);
	void SetRoughness(float _roughness);
	void SetVertShader(std::shared_ptr<SimpleVertexShader> _vertShader);
	void SetPixelShader(std::shared_ptr<SimplePixelShader> _pixelShader);
	void AddTextureSRV(std::string shaderVariableName, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv);
	void AddSampler(std::string samplerVariableName, Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler);

	// Function
	void PrepareMaterial(Transform* transform, std::shared_ptr<Camera> camera);

private:
	DirectX::XMFLOAT4 colorTint;
	float roughness;
	std::shared_ptr<SimpleVertexShader> vertShader;
	std::shared_ptr<SimplePixelShader> pixelShader;

	// Textures
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> textureSRVs;
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11SamplerState>> samplers;
};

