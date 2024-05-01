#include "ShadowLight.h"

// Static Variables
//Microsoft::WRL::ComPtr<ID3D11Device> ShadowLight::device;
//Microsoft::WRL::ComPtr<ID3D11DeviceContext> ShadowLight::context;
//std::shared_ptr<SimpleVertexShader> ShadowLight::shadowVS;
unsigned int* ShadowLight::windowWidth;
unsigned int* ShadowLight::windowHeight;
//Microsoft::WRL::ComPtr<ID3D11RenderTargetView> ShadowLight::backBufferRTV;
//Microsoft::WRL::ComPtr<ID3D11DepthStencilView> ShadowLight::depthBufferDSV;

// Construction
ShadowLight::ShadowLight(DirectX::XMFLOAT3 _direction, float _intensity, DirectX::XMFLOAT3 _color, Microsoft::WRL::ComPtr<ID3D11Device> _device, Microsoft::WRL::ComPtr<ID3D11DeviceContext> _context) :
	device(_device),
	context(_context)
{
	// Create Light Struct
	light = {};
	light.Type = LIGHT_TYPE_DIR;
	light.Direction = _direction;
	light.Intensity = _intensity;
	light.Color = _color;

	Init();
}
ShadowLight::ShadowLight(DirectX::XMFLOAT3 _direction, float _range, DirectX::XMFLOAT3 _position, float _intensity, DirectX::XMFLOAT3 _color, float _spotFalloff, float _fov, Microsoft::WRL::ComPtr<ID3D11Device> _device, Microsoft::WRL::ComPtr<ID3D11DeviceContext> _context) :
	device(_device),
	context(_context)
{
	// Create Light Struct
	light = {};
	light.Type = LIGHT_TYPE_SPOT;
	light.Direction = _direction;
	light.Range = _range;
	light.Position = _position;
	light.Intensity = _intensity;
	light.Color = _color;
	light.SpotFalloff = _spotFalloff;
	light.Fov = _fov;

	Init();
}
ShadowLight::ShadowLight(Light _light, Microsoft::WRL::ComPtr<ID3D11Device> _device, Microsoft::WRL::ComPtr<ID3D11DeviceContext> _context) :
	device(_device),
	context(_context)
{
	// Create Light Struct
	light = _light;

	Init();
}
ShadowLight::~ShadowLight() {}
void ShadowLight::Init()
{
	CreateShadowMapData();
	// Create Vertex Shader
	shadowVS = std::make_shared<SimpleVertexShader>(device, context, FixPath(L"ShadowShader.cso").c_str()); 
	// Create matricies
	lightProjectionSize = 20.0f;
	lightProjectionDirty = true;
	lightViewDirty = true;
	UpdateProjectionMatrix();
	UpdateViewMatrix();
}

// Getters and Setters
//void ShadowLight::SetDevice(Microsoft::WRL::ComPtr<ID3D11Device> _device) { device = _device; }
//void ShadowLight::SetBackBufferRTV(Microsoft::WRL::ComPtr<ID3D11RenderTargetView> _backBufferRTV) { backBufferRTV = _backBufferRTV; }
//void ShadowLight::SetDepthBufferDSV(Microsoft::WRL::ComPtr<ID3D11DepthStencilView> _depthBufferDSV) { depthBufferDSV = _depthBufferDSV; }
//void ShadowLight::SetContext(Microsoft::WRL::ComPtr<ID3D11DeviceContext> _context) { context = _context; }
void ShadowLight::SetLightProjectionSize(float _lightProjectionSize)
{
	lightProjectionSize = _lightProjectionSize;
	lightProjectionDirty = true;
}
void ShadowLight::SetWindowSize(unsigned int* _windowWidth, unsigned int* _windowHeight )
{ 
	windowWidth = _windowWidth;
	windowHeight = _windowHeight;
}
void ShadowLight::SetFov(float _fov)
{
	if (light.Type != LIGHT_TYPE_SPOT) { return; }
	light.Fov = _fov;
	lightProjectionDirty = true;
}
void ShadowLight::SetDirection(DirectX::XMFLOAT3 _direction)
{
	light.Direction = _direction;
	lightViewDirty = true;
}
void ShadowLight::SetPosition(DirectX::XMFLOAT3 _position)
{
	if (light.Type != LIGHT_TYPE_SPOT) { return; }
	light.Position = _position;
	lightViewDirty = true;
}

Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> ShadowLight::GetShadowSRV() { return shadowSRV; }
DirectX::XMFLOAT4X4 ShadowLight::GetShadowViewMatrix() { return shadowViewMatrix; }
DirectX::XMFLOAT4X4 ShadowLight::GetShadowProjectionMatrix() { return shadowProjectionMatrix; }
Microsoft::WRL::ComPtr<ID3D11SamplerState> ShadowLight::GetShadowSampler() { return shadowSampler; }
int ShadowLight::GetType() { return light.Type; }
DirectX::XMFLOAT3 ShadowLight::GetDirection() { return light.Direction; }
DirectX::XMFLOAT3 ShadowLight::GetPosition() { return light.Position; }

// Public Functions
void ShadowLight::Update(std::vector<Entity> entities, std::vector<Entity> transparentEntities,
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> _backBufferRTV,
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> _depthBufferDSV,
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> _rasterizerState)
{
	if (lightProjectionDirty) { UpdateProjectionMatrix(); }
	if (lightViewDirty) { UpdateViewMatrix(); }

	Render(entities, transparentEntities, _backBufferRTV, _depthBufferDSV, _rasterizerState);
}



// Private Helper Functions

/// <summary>
/// Create Depth Stencil view and Shader Resource View
/// </summary>
void ShadowLight::CreateShadowMapData()	
{
	shadowMapResolution = 1024;
	// Create the actual texture that will be the shadow map
	D3D11_TEXTURE2D_DESC shadowDesc = {};
	shadowDesc.Width = shadowMapResolution; // Ideally a power of 2 (like 1024)
	shadowDesc.Height = shadowMapResolution; // Ideally a power of 2 (like 1024)
	shadowDesc.ArraySize = 1;
	shadowDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	shadowDesc.CPUAccessFlags = 0;
	shadowDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	shadowDesc.MipLevels = 1;
	shadowDesc.MiscFlags = 0;
	shadowDesc.SampleDesc.Count = 1;
	shadowDesc.SampleDesc.Quality = 0;
	shadowDesc.Usage = D3D11_USAGE_DEFAULT;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> shadowTexture;
	device->CreateTexture2D(&shadowDesc, 0, shadowTexture.GetAddressOf());

	// Create the depth/stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC shadowDSDesc = {};
	shadowDSDesc.Format = DXGI_FORMAT_D32_FLOAT;
	shadowDSDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	shadowDSDesc.Texture2D.MipSlice = 0;
	device->CreateDepthStencilView(
		shadowTexture.Get(),
		&shadowDSDesc,
		shadowDSV.GetAddressOf());

	// Create the SRV for the shadow map
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	device->CreateShaderResourceView(
		shadowTexture.Get(),
		&srvDesc,
		shadowSRV.GetAddressOf());

	// Create the special "comparison" sampler state for shadows
	D3D11_SAMPLER_DESC shadowSampDesc = {};
	shadowSampDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR; // COMPARISON filter!
	shadowSampDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
	shadowSampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.BorderColor[0] = 1.0f;
	shadowSampDesc.BorderColor[1] = 1.0f;
	shadowSampDesc.BorderColor[2] = 1.0f;
	shadowSampDesc.BorderColor[3] = 1.0f;
	device->CreateSamplerState(&shadowSampDesc, &shadowSampler);

	// Create a rasterizer state
	D3D11_RASTERIZER_DESC shadowRastDesc = {};
	shadowRastDesc.FillMode = D3D11_FILL_SOLID;
	shadowRastDesc.CullMode = D3D11_CULL_BACK;
	shadowRastDesc.DepthClipEnable = true;
	shadowRastDesc.DepthBias = 1000; // Multiplied by (smallest possible positive value storable in the depth buffer)
	shadowRastDesc.DepthBiasClamp = 0.0f;
	shadowRastDesc.SlopeScaledDepthBias = 1.0f;
	device->CreateRasterizerState(&shadowRastDesc, &shadowRasterizer);
}
/// <summary>
/// Update the light projection matrix
/// </summary>
void ShadowLight::UpdateProjectionMatrix()
{
	lightProjectionDirty = false;
	switch (light.Type)
	{
	case (LIGHT_TYPE_POINT):
		printf("Cannot have Point Shadow Light");
		return;
	case(LIGHT_TYPE_DIR):
		// Only needs to be updated if lightProjectionSize changes
		DirectX::XMMATRIX lightProjection = DirectX::XMMatrixOrthographicLH(
			lightProjectionSize,
			lightProjectionSize,
			0.1f,
			100.0f);
		XMStoreFloat4x4(&shadowProjectionMatrix, lightProjection);
		break;
	case(LIGHT_TYPE_SPOT):
		// Only needs to be updated if light.Fov changes
		lightProjection = DirectX::XMMatrixPerspectiveFovLH(
			light.Fov,
			1.0f,
			0.01f,
			100.0f);
		XMStoreFloat4x4(&shadowProjectionMatrix, lightProjection);
		break;
	}
}
/// <summary>
/// Update the light view matrix
/// </summary>
void ShadowLight::UpdateViewMatrix()
{
	lightViewDirty = false;
	DirectX::XMVECTOR lightDirection;
	DirectX::XMVECTOR lightPosition;
	switch (light.Type)
	{
	case (LIGHT_TYPE_POINT):
		printf("Cannot have Point Shadow Light");
		return;
	case(LIGHT_TYPE_DIR):
		// Only needs to update if direction changes
		lightDirection = DirectX::XMLoadFloat3(&light.Direction);
		DirectX::XMMATRIX lightView = DirectX::XMMatrixLookToLH(
			DirectX::XMVectorScale(lightDirection, -7.5f), // Position: "Backing up" 20 units from origin
			lightDirection, // Direction: light's direction
			DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)); // Up: World up vector (Y axis)
		XMStoreFloat4x4(&shadowViewMatrix, lightView);
		break;
	case(LIGHT_TYPE_SPOT):
		// Only needs to update if direction or position changes
		lightDirection = DirectX::XMLoadFloat3(&light.Direction);
		lightPosition = DirectX::XMLoadFloat3(&light.Position);
		lightView = DirectX::XMMatrixLookToLH(
			lightPosition, // Position: "Backing up" 20 units from origin
			lightDirection, // Direction: light's direction
			DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)); // Up: World up vector (Y axis)
		XMStoreFloat4x4(&shadowViewMatrix, lightView);
		break;
	}
}

void ShadowLight::Render(std::vector<Entity> entities, std::vector<Entity> transparentEntities,
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> _backBufferRTV,
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> _depthBufferDSV,
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> _rasterizerState)
{
	// Setup
	// Clear the Shadow Map
	context->ClearDepthStencilView(shadowDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	context->RSSetState(shadowRasterizer.Get());
	ID3D11RenderTargetView* nullRTV{}; // Set up the output merger stage
	context->OMSetRenderTargets(1, &nullRTV, shadowDSV.Get());
	//context->OMSetRenderTargets(0, 0, shadowDSV.Get());
	D3D11_VIEWPORT viewport = {}; // Change viewport
	viewport.Width = (float)shadowMapResolution;
	viewport.Height = (float)shadowMapResolution;
	viewport.MaxDepth = 1.0f;
	context->RSSetViewports(1, &viewport);
	// Set Shaders
	context->PSSetShader(0, 0, 0); // Deactivate the pixel shader
	shadowVS->SetShader();
	shadowVS->SetMatrix4x4("view", shadowViewMatrix);
	shadowVS->SetMatrix4x4("projection", shadowProjectionMatrix);
	// Entity Render Loop
	for (auto& e : entities)
	{
		shadowVS->SetMatrix4x4("world", e.GetTransform()->GetWorldMatrix());
		shadowVS->CopyAllBufferData();
		// Draw the mesh directly to avoid the entity's material
		// Note: Your code may differ significantly here!
		e.GetMesh()->Draw();
	}

	for (auto& e : transparentEntities)
	{
		shadowVS->SetMatrix4x4("world", e.GetTransform()->GetWorldMatrix());
		shadowVS->CopyAllBufferData();
		// Draw the mesh directly to avoid the entity's material
		// Note: Your code may differ significantly here!
		e.GetMesh()->Draw();
	}
	// Reset the pipline
	viewport.Width = (float)*windowWidth;
	viewport.Height = (float)*windowHeight;
	context->RSSetViewports(1, &viewport);
	context->OMSetRenderTargets(
		1,
		_backBufferRTV.GetAddressOf(),
		_depthBufferDSV.Get());
	context->RSSetState(_rasterizerState.Get());
}
