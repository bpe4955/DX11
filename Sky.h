#pragma once
#include "DXCore.h"
#include <memory>
#include "Mesh.h"
#include "SimpleShader.h"
#include "Camera.h"

class Sky
{
public:
	/// <summary>
	/// Sky Constructor
	/// </summary>
	/// <param name="_mesh">Shared Pointer to the Sky's Mesh</param>
	/// <param name="_sampleState">ComPtr of the Sampler State</param>
	/// <param name="device">ComPtr of the Device</param>
	/// <param name="context">ComPtr of the Device Context</param>
	/// <param name="filePath">File Path to the folder holding the six textures</param>
	Sky(std::shared_ptr<Mesh> _mesh,
		Microsoft::WRL::ComPtr<ID3D11SamplerState> _sampleState,
		Microsoft::WRL::ComPtr<ID3D11Device> device,
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> context,
		std::wstring filePath);
	~Sky();

	// Variable
	DirectX::XMFLOAT4 colorTint;

	// Functions
	void Draw(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context, std::shared_ptr<Camera> camera,
		Microsoft::WRL::ComPtr<ID3D11RasterizerState> _rasterizerState);

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetSkyTexture();

private:
	// Variables
	Microsoft::WRL::ComPtr<ID3D11SamplerState> sampleState;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> skyTexture;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> stencilState;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState;

	std::shared_ptr<Mesh> mesh;
	std::shared_ptr<SimplePixelShader> ps;
	std::shared_ptr<SimpleVertexShader> vs;

	// Helper for creating a cubemap from 6 individual textures
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> CreateCubemap(
		Microsoft::WRL::ComPtr<ID3D11Device> device,
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> context,
		const wchar_t* right,
		const wchar_t* left,
		const wchar_t* up,
		const wchar_t* down,
		const wchar_t* front,
		const wchar_t* back);
};

