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
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> testRastState;

	// Lights
	Light spotLight;
	std::vector<Light> lights;

	// Store data for entities
	std::vector<std::shared_ptr<Mesh>> meshes;
	std::vector<std::shared_ptr<Material>> materials;
	std::vector<Entity> entities;

	// Simple Shaders
	std::shared_ptr<SimpleVertexShader> vs;
	std::shared_ptr<SimplePixelShader> ps;
	std::shared_ptr<SimplePixelShader> ps2;


	DirectX::XMFLOAT3 MouseRayCast();

};

