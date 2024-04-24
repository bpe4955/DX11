#pragma once
#include "Light.h"
#include "DXCore.h"
#include <vector>
#include "Entity.h"

class ShadowLight
{
public:
	/// <summary>
	/// Shadow Light for a directional Light
	/// </summary>
	/// <param name="_direction">Light's Direction</param>
	/// <param name="_intensity">Light's Intensity</param>
	/// <param name="_color">Light's Color</param>
	ShadowLight(DirectX::XMFLOAT3 _direction, float _intensity, DirectX::XMFLOAT3 _color, 
		Microsoft::WRL::ComPtr<ID3D11Device> _device, Microsoft::WRL::ComPtr<ID3D11DeviceContext> _context);
	/// <summary>
	/// Shadow Light for a spot light
	/// </summary>
	/// <param name="_direction">Light's Direction</param>
	/// <param name="_range">Light's Range</param>
	/// <param name="_position">Light's Position</param>
	/// <param name="_intensity">Light's Intensity</param>
	/// <param name="_color">Light's Color</param>
	/// <param name="_spotFalloff">Light's Falloff</param>
	/// <param name="_fov">Light's Fov</param>
	ShadowLight(DirectX::XMFLOAT3 _direction, float _range, DirectX::XMFLOAT3 _position,
		float _intensity, DirectX::XMFLOAT3 _color, float _spotFalloff, float _fov,
		Microsoft::WRL::ComPtr<ID3D11Device> _device, Microsoft::WRL::ComPtr<ID3D11DeviceContext> _context);
	/// <summary>
	/// Shadow light for a given light
	/// </summary>
	/// <param name="_light">Light to cast shadows</param>
	ShadowLight(Light _light, Microsoft::WRL::ComPtr<ID3D11Device> _device, Microsoft::WRL::ComPtr<ID3D11DeviceContext> _context);
	~ShadowLight();

	// Getters and Setters
	//static void SetDevice(Microsoft::WRL::ComPtr<ID3D11Device> _device);
	//static void SetBackBufferRTV(Microsoft::WRL::ComPtr<ID3D11RenderTargetView> _backBufferRTV);
	//static void SetDepthBufferDSV(Microsoft::WRL::ComPtr<ID3D11DepthStencilView> _depthBufferDSV);
	//static void SetContext(Microsoft::WRL::ComPtr<ID3D11DeviceContext> _context);
	void SetLightProjectionSize(float _lightProjectionSize);
	static void SetWindowSize(unsigned int* _windowWidth, unsigned int* _windowHeight);
	void SetFov(float _fov);
	void SetDirection(DirectX::XMFLOAT3 _direction);
	void SetPosition(DirectX::XMFLOAT3 _position);

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetShadowSRV();
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> GetShadowDSV();
	int GetType();
	DirectX::XMFLOAT3 GetDirection();
	DirectX::XMFLOAT3 GetPosition();

	// Public Functions
	void Update(std::vector<Entity> entities, Microsoft::WRL::ComPtr<ID3D11RenderTargetView> _backBufferRTV,
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> _depthBufferDSV);

private:
	/// <summary>
	/// Private Constructor
	/// </summary>
	void Init();
	Light light;
	// Shadow Map Data
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> shadowDSV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shadowSRV;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> shadowSampler;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> shadowRasterizer;
	int shadowMapResolution;
	// Light Matrices
	DirectX::XMFLOAT4X4 shadowViewMatrix;
	DirectX::XMFLOAT4X4 shadowProjectionMatrix;
	bool lightViewDirty;
	bool lightProjectionDirty;
	float lightProjectionSize;
	
	// Shaders
	std::shared_ptr<SimpleVertexShader> shadowVS;

	// Game Data
	Microsoft::WRL::ComPtr<ID3D11Device> device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
	static unsigned int* windowWidth;
	static unsigned int* windowHeight;
	//static Microsoft::WRL::ComPtr<ID3D11RenderTargetView> backBufferRTV;
	//static Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthBufferDSV;

	// Helper Functions
	void CreateShadowMapData();
	void UpdateProjectionMatrix();
	void UpdateViewMatrix();

	void Render(std::vector<Entity> entities, Microsoft::WRL::ComPtr<ID3D11RenderTargetView> _backBufferRTV,
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> _depthBufferDSV);
};

