#pragma once

#include "DXCore.h"
#include <DirectXMath.h>
#include <wrl/client.h> // Used for ComPtr - a smart pointer for COM objects
#include <memory>
#include "Mesh.h"
#include <vector>
#include "BuffStructs.h"
#include "Entity.h"
#include "Camera.h"
#include "SimpleShader.h"
#include "Material.h"
#include "Light.h"
#include "WICTextureLoader.h"
#include "Sky.h"
#include "ShadowLight.h"


class Game 
	: public DXCore
{

public:
	Game(HINSTANCE hInstance);
	~Game();

	// Overridden setup and game loop methods, which
	// will be called automatically
	void Init();
	void OnResize();
	void Update(float deltaTime, float totalTime);
	void Draw(float deltaTime, float totalTime);

private:
	// Initialization helper methods - feel free to customize, combine, remove, etc.
	void LoadShaders(); 
	void CreateMaterials();
	void CreateGeometry();
	void CreateLights();
	void UpdateUI(float deltaTime);
	void BuildUI();

	// Camera
	std::vector<std::shared_ptr<Camera>> cameras;
	int cameraIndex;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rastState;

	// Lights
	Light spotLight;
	std::vector<Light> lights;
	std::vector<ShadowLight> shadowLights;

	// Store data for entities
	std::vector<std::shared_ptr<Mesh>> meshes;
	std::vector<std::shared_ptr<Material>> materials;
	std::vector<std::shared_ptr<Material>> transparentMaterials;
	std::vector<Entity> entities;
	std::vector<Entity> transparentEntities;
	std::unique_ptr<Sky> skyBox;

	// Simple Shaders
	std::shared_ptr<SimpleVertexShader> vs;
	//std::shared_ptr<SimplePixelShader> ps;
	std::shared_ptr<SimplePixelShader> customPS;

	// Blending
	Microsoft::WRL::ComPtr<ID3D11BlendState> blendState;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> cullBackRastState;

	// Post Processing
	// Resources that are shared among all post processes
	Microsoft::WRL::ComPtr<ID3D11SamplerState> ppSampler;
	std::shared_ptr<SimpleVertexShader> ppVS;
	// Resources that are tied to a particular post process
	std::shared_ptr<SimplePixelShader> ppPS;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> ppRTV; // For rendering
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> ppSRV; // For sampling
	int blurStrength;

	// Helper Functions
	void PostProcessSetup();
	void ResetPostProcess();
	DirectX::XMFLOAT3 MouseRayCast();

};

