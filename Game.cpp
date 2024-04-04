#include "Game.h"
#include "Vertex.h"
#include "Input.h"
#include "PathHelpers.h"
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"

// Needed for a helper function to load pre-compiled shader files
#pragma comment(lib, "d3dcompiler.lib")
#include <d3dcompiler.h>

// For the DirectX Math library
using namespace DirectX;

//Variables
const float RADTODEG = 57.2958f;
XMFLOAT4 uiColor(1.0f, 1.0f, 1.0f, 1.0f); // Default Cornflower Blue
bool demoWindowVisible = false;
bool isFullscreen = false;
float BRIGHTNESS = 0.1f;
XMFLOAT3 ambientColor = XMFLOAT3(uiColor.x * BRIGHTNESS ,uiColor.y * BRIGHTNESS,uiColor.z * BRIGHTNESS) ;

// --------------------------------------------------------
// Constructor
//
// DXCore (base class) constructor will set up underlying fields.
// Direct3D itself, and our window, are not ready at this point!
//
// hInstance - the application's OS-level handle (unique ID)
// --------------------------------------------------------
Game::Game(HINSTANCE hInstance)
	: DXCore(
		hInstance,			// The application's handle
		L"DirectX Game",	// Text for the window's title bar (as a wide-character string)
		1280,				// Width of the window's client area
		720,				// Height of the window's client area
		false,				// Sync the framerate to the monitor refresh? (lock framerate)
		true)				// Show extra stats (fps) in title bar?
{
#if defined(DEBUG) || defined(_DEBUG)
	// Do we want a console window?  Probably only in debug mode
	CreateConsoleWindow(500, 120, 32, 120);
	printf("Console window created successfully.  Feel free to printf() here.\n");
#endif
	cameraIndex = 0;
	spotLight = {};
}

// --------------------------------------------------------
// Destructor - Clean up anything our game has created:
//  - Delete all objects manually created within this class
//  - Release() all Direct3D objects created within this class
// --------------------------------------------------------
Game::~Game()
{
	// Call delete or delete[] on any objects or arrays you've
	// created using new or new[] within this class
	// - Note: this is unnecessary if using smart pointers

	// Call Release() on any Direct3D objects made within this class
	// - Note: this is unnecessary for D3D objects stored in ComPtrs

	// ImGui clean up
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

// --------------------------------------------------------
// Called once per program, after Direct3D and the window
// are initialized but before the game loop.
// --------------------------------------------------------
void Game::Init()
{
	LoadShaders();
	CreateMaterials();
	CreateGeometry();
	CreateLights();

	// Set initial graphics API state
	// Tell the input assembler (IA) stage of the pipeline what kind of
	// geometric primitives (points, lines or triangles) we want to draw.  
	// Essentially: "What kind of shape should the GPU draw with our vertices?"
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Initialize ImGui itself & platform/renderer backends
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui_ImplWin32_Init(hWnd);
		ImGui_ImplDX11_Init(device.Get(), context.Get());
		// Pick a style (uncomment one of these 3)
		ImGui::StyleColorsDark();
	}

	// Create Cameras
	cameraIndex = 0;
	cameras.push_back(std::make_shared<Camera>(
		(float)this->windowWidth, (float)this->windowHeight, XMFLOAT3(0.0f, 0.0f, -10.0f)));
	cameras.push_back(std::make_shared<Camera>(
		(float)this->windowWidth, (float)this->windowHeight, XMFLOAT3(0.0f, 0.0f, -10.0f)));
	cameras[1]->UpdateProjMatrix(false, (float)windowWidth, (float)windowHeight);
	cameras[1]->SetMouseSens(0.005f);
	// Rasterizer state
	D3D11_RASTERIZER_DESC rd = {};
	rd.CullMode = D3D11_CULL_NONE;
	rd.FillMode = D3D11_FILL_SOLID;
	device->CreateRasterizerState(&rd, rastState.GetAddressOf());

	context->RSSetState(rastState.Get());

}

// --------------------------------------------------------
// Loads shaders from compiled shader object (.cso) files
// and also created the Input Layout that describes our 
// vertex data to the rendering pipeline. 
// - Input Layout creation is done here because it must 
//    be verified against vertex shader byte code
// - We'll have that byte code already loaded below
// --------------------------------------------------------
void Game::LoadShaders()
{
	//ps = std::make_shared<SimplePixelShader>(device, context, FixPath(L"PixelShader.cso").c_str());
	ps2 = std::make_shared<SimplePixelShader>(device, context, FixPath(L"CustomPS.cso").c_str());
	vs = std::make_shared<SimpleVertexShader>(device, context, FixPath(L"VertexShader.cso").c_str());
}


void Game::CreateMaterials()
{
	// Create Sampler State
	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState;
	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP; 
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;		
	samplerDesc.MaxAnisotropy = 16;		// Can make this a "Graphics Setting"
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	device->CreateSamplerState(&samplerDesc, samplerState.GetAddressOf());
	
	// Load Textures
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> tilesTex;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> tilesSpec;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> crackedMask;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cushionTex;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cushionNormal;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> rockTex;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> rockNormal;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> questTex;

	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/tiles.png").c_str(), nullptr, tilesTex.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/tiles_specular.png").c_str(), nullptr, tilesSpec.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/MaskCracked.jpg").c_str(), nullptr, crackedMask.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/cushion.png").c_str(), nullptr, cushionTex.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/cushion_normals.png").c_str(), nullptr, cushionNormal.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/rock.png").c_str(), nullptr, rockTex.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/rock_normals.png").c_str(), nullptr, rockNormal.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/Brian_Quest64.png").c_str(), nullptr, questTex.GetAddressOf());

	// Create Materials
	materials.push_back(std::make_shared<Material>(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), 0.1f, vs, ps2));
	materials.back().get()->AddSampler("Sampler", samplerState);
	materials.back().get()->AddTextureSRV("SurfaceTexture", rockTex);
	materials.back().get()->AddTextureSRV("NormalMap", rockNormal);
	materials.push_back(std::make_shared<Material>(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), 0.9f, vs, ps2));
	materials.back().get()->AddSampler("Sampler", samplerState);
	materials.back().get()->AddTextureSRV("SurfaceTexture", cushionTex);
	materials.back().get()->AddTextureSRV("NormalMap", cushionNormal);
	//materials.push_back(std::make_shared<Material>(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), 0.2f, vs, ps2));
	//materials.push_back(std::make_shared<Material>(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), 0.8f, vs, ps));
	//materials.push_back(std::make_shared<Material>(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), 0.2f, vs, ps));
	materials.push_back(std::make_shared<Material>(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), 1.0f, vs, ps2));
	materials.back().get()->AddSampler("Sampler", samplerState);
	materials.back().get()->AddTextureSRV("SurfaceTexture", questTex);

	// Create SkyBox
	meshes.push_back(std::make_shared<Mesh>(FixPath(L"../../Assets/Models/cube.obj").c_str(), context, device));
	skyBox = std::make_unique<Sky>(meshes.back(),
		samplerState, device, context,
		FixPath(L"../../Assets/Skies/Planet/").c_str());
	skyBox->colorTint = uiColor;
}

void Game::CreateLights()
{
	spotLight.Type = LIGHT_TYPE_SPOT;
	spotLight.Direction = XMFLOAT3(0.0f, -1.0f, 0.0f);
	spotLight.Range = 5.0f;
	spotLight.Position = XMFLOAT3(1.25f, 3.0f, 0.0f);
	spotLight.Intensity = 0.2f;
	spotLight.Color = XMFLOAT3(1.0f, 1.0f, 1.0f);
	spotLight.SpotFalloff = 1.0f;
	lights.push_back(Light{});
	lights[lights.size()-1].Type = LIGHT_TYPE_DIR;
	lights[lights.size()-1].Direction = XMFLOAT3(1.0f, -1.0f, 0.0f); // Affects top and right of objects
	lights[lights.size()-1].Color = XMFLOAT3(0.2f, 0.2f, 1.0f); // Blue
	lights[lights.size()-1].Intensity = 0.1f;
	lights.push_back(Light{});
	lights[lights.size()-1].Type = LIGHT_TYPE_DIR;
	lights[lights.size()-1].Direction = XMFLOAT3(0.0f, 1.0f, 0.0f); // Affects bottom of objects
	lights[lights.size()-1].Color = XMFLOAT3(0.2f, 1.0f, 0.2f); // Green
	lights[lights.size()-1].Intensity = 0.2f;
	lights.push_back(Light{});
	lights[lights.size()-1].Type = LIGHT_TYPE_DIR;
	lights[lights.size()-1].Direction = XMFLOAT3(-1.0f, -1.0f, 0.0f); // Affects top and left of objects
	lights[lights.size()-1].Color = XMFLOAT3(1.0f, 0.2f, 0.2f); // Red
	lights[lights.size()-1].Intensity = 0.3f;
	lights.push_back(Light{});
	lights[lights.size()-1].Type = LIGHT_TYPE_POINT;
	lights[lights.size()-1].Range = 5.0f;
	lights[lights.size()-1].Position = XMFLOAT3(4.0f, 2.5f, -2.0f); // Located behind the cube / creature
	lights[lights.size()-1].Color = XMFLOAT3(1.0f, 1.0f, 0.2f); // Yellow
	lights[lights.size()-1].Intensity = 0.5f;
	lights.push_back(Light{});
	lights[lights.size()-1].Type = LIGHT_TYPE_POINT;
	lights[lights.size()-1].Range = 3.0f;
	lights[lights.size()-1].Position = XMFLOAT3(9.0f, -0.5f, 1.65f); // Located in front of yoshi
	lights[lights.size()-1].Color = XMFLOAT3(1.0f, 0.2f, 1.0f); // Magenta
	lights[lights.size()-1].Intensity = 0.7f;

	//ps->SetData("lights",
	//	&lights[0],
	//	sizeof(Light)*MAX_NUM_LIGHTS);
	ps2->SetData("lights",
		&lights[0],
		sizeof(Light) * MAX_NUM_LIGHTS);

	int numLights = (int)lights.size();
	//ps->SetData("numLights",
	//	&numLights,
	//	sizeof(int));
	ps2->SetData("numLights",
		&numLights,
		sizeof(int));
}

// --------------------------------------------------------
// Creates the geometry we're going to draw 
// --------------------------------------------------------
void Game::CreateGeometry()
{
	//meshes.push_back(std::make_shared<Mesh>(FixPath(L"../../Assets/Models/cheburashka.obj").c_str(), context, device));
	meshes.push_back(std::make_shared<Mesh>(FixPath(L"../../Assets/Models/sphere.obj").c_str(), context, device));
	meshes.push_back(std::make_shared<Mesh>(FixPath(L"../../Assets/Models/N square.obj").c_str(), context, device));
	meshes.push_back(std::make_shared<Mesh>(FixPath(L"../../Assets/Models/yoshi.obj").c_str(), context, device));
	meshes.push_back(std::make_shared<Mesh>(FixPath(L"../../Assets/Models/cheburashka.obj").c_str(), context, device));
	meshes.push_back(std::make_shared<Mesh>(FixPath(L"../../Assets/Models/Brian_Quest64.obj").c_str(), context, device));
	//meshes.push_back(std::make_shared<Mesh>(FixPath(L"../../Assets/Models/flower.dae").c_str(), context, device));
	//meshes.push_back(std::make_shared<Mesh>(FixPath(L"../../Assets/Models/Pikachu (Gigantamax).dae").c_str(), context, device));
	//meshes.push_back(std::make_shared<Mesh>(FixPath(L"../../Assets/Models/Pikachu (Gigantamax).fbx").c_str(), context, device));
	//meshes.push_back(std::make_shared<Mesh>(FixPath(L"../../Assets/Models/cylinder.obj").c_str(), context, device));
	//meshes.push_back(std::make_shared<Mesh>(FixPath(L"../../Assets/Models/helix.obj").c_str(), context, device));
	//meshes.push_back(std::make_shared<Mesh>(FixPath(L"../../Assets/Models/torus.obj").c_str(), context, device));
	//meshes.push_back(std::make_shared<Mesh>(FixPath(L"../../Assets/Models/quad.obj").c_str(), context, device));
	
	

	// Create Entities
	for (size_t i = 0; i < meshes.size(); i++)
	{
		//entities.push_back(Entity(meshes[i], materials[i % 2]));
		//entities[i * 2].GetTransform()->SetPosition(XMFLOAT3(-3.0f*(1+i), 0.0f, 0.0f));
		//entities.push_back(Entity(meshes[i], materials[i%2 + 2]));
		//entities[i * 2 + 1].GetTransform()->SetPosition(XMFLOAT3(3.0f * (1 + i), 0.0f, 0.0f));
		entities.push_back(Entity(meshes[i], materials[1]));
		entities[i].GetTransform()->SetPosition(XMFLOAT3(-3.0f * (i), 0.0f, 0.0f));
	}
	entities[0].SetMaterial(materials[0]);
	entities.back().SetMaterial(materials.back());
}


// --------------------------------------------------------
// Handle resizing to match the new window size.
//  - DXCore needs to resize the back buffer
//  - Eventually, we'll want to update our 3D camera
// --------------------------------------------------------
void Game::OnResize()
{
	// Handle base-level DX resize stuff
	DXCore::OnResize();
	for (size_t i = 0; i < cameras.size(); i++)
	{
		cameras[i]->UpdateProjMatrix((float)windowWidth, (float)windowHeight);
	}
}

// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	Input& input = Input::GetInstance();

	UpdateUI(deltaTime);

	// Update Camera
	cameras[cameraIndex]->Update(deltaTime);

	//Move Entities
	for (size_t i = 0; i < entities.size(); i ++)
	{
		//entities[i].GetTransform()->SetPosition((float)sin(totalTime) - i * 2.5f, 0, 0);
	}


	//Move SpotLight
	spotLight.Position = cameras[cameraIndex]->GetPosition();
	XMFLOAT3 mouseDir = MouseRayCast();
	spotLight.Direction = mouseDir;

	ps2->SetData("spotLight",
		&spotLight,
		sizeof(Light));

	// Example input checking: Quit if the escape key is pressed
	if (input.KeyDown(VK_ESCAPE))
		Quit();
}

// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	// Frame START
	// - These things should happen ONCE PER FRAME
	// - At the beginning of Game::Draw() before drawing *anything*
	{
		// Clear the back buffer (erases what's on the screen)
		const float bgColor[4] = { uiColor.x, uiColor.y, uiColor.z, uiColor.w }; // Edited in custom debug menu
		context->ClearRenderTargetView(backBufferRTV.Get(), bgColor);

		// Clear the depth buffer (resets per-pixel occlusion information)
		context->ClearDepthStencilView(depthBufferDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	}

	// DRAW entities
	// Loop through shaders and set universal data
	//if (ps->HasVariable("totalTime")) { ps->SetFloat("totalTime", totalTime); }
	if (ps2->HasVariable("totalTime")) { ps2->SetFloat("totalTime", totalTime); }
	XMFLOAT3 ambientColor = XMFLOAT3(uiColor.x * BRIGHTNESS, uiColor.y * BRIGHTNESS, uiColor.z * BRIGHTNESS);
	//if (ps->HasVariable("ambient")) { ps->SetFloat3("ambient", ambientColor); }
	if (ps2->HasVariable("ambient")) { ps2->SetFloat3("ambient", ambientColor); }
		
	for (size_t i = 0; i < entities.size(); i++)
	{
		entities[i].Draw(context, cameras[cameraIndex]);
	}

	// DRAW Skybox
	skyBox->colorTint = uiColor;
	skyBox->Draw(context, cameras[cameraIndex], rastState);

	// DRAW ImGUI
	ImGui::Render(); // Turns this frame’s UI into renderable triangles
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData()); // Draws it to the screen


	// Frame END
	// - These should happen exactly ONCE PER FRAME
	// - At the very end of the frame (after drawing *everything*)
	{
		// Present the back buffer to the user
		//  - Puts the results of what we've drawn onto the window
		//  - Without this, the user never sees anything
		bool vsyncNecessary = vsync || !deviceSupportsTearing || isFullscreen;
		swapChain->Present(
			vsyncNecessary ? 1 : 0,
			vsyncNecessary ? 0 : DXGI_PRESENT_ALLOW_TEARING);

		// Must re-bind buffers after presenting, as they become unbound
		context->OMSetRenderTargets(1, backBufferRTV.GetAddressOf(), depthBufferDSV.Get());
	}
}

//Helper Functions

/// <summary>
/// Helper for update to deal with ImGui
/// </summary>
void Game::UpdateUI(float deltaTime)
{
	// Feed fresh data to ImGui
	ImGuiIO& io = ImGui::GetIO();
	io.DeltaTime = deltaTime;
	io.DisplaySize.x = (float)this->windowWidth;
	io.DisplaySize.y = (float)this->windowHeight;
	// Reset the frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	// Determine new input capture
	Input& input = Input::GetInstance();
	input.SetKeyboardCapture(io.WantCaptureKeyboard);
	input.SetMouseCapture(io.WantCaptureMouse);
	// Show the demo window
	if (demoWindowVisible) { ImGui::ShowDemoWindow(); }

	BuildUI();
}

/// <summary>
/// Build a custom Window for the UI
/// </summary>
void Game::BuildUI()
{
	char buf[128];
	sprintf_s(buf, "Custom Debug %c###CustomDebug", "|/-\\"[(int)(ImGui::GetTime() / 0.25f) & 3]);
	ImGui::Begin(buf, NULL, ImGuiWindowFlags_AlwaysAutoResize);

	ImGui::SetNextItemOpen(true, ImGuiCond_Once);
	if (ImGui::TreeNode("App Details"))
	{
		float fps = ImGui::GetIO().Framerate;
		if (fps > 60) { ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Framerate: %f fps", fps); }
		else if (fps > 30) { ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Framerate: %f fps", fps); }
		else { ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Framerate: %f fps", fps); }
		ImGui::Text("Frame Count: %d", ImGui::GetFrameCount());
		ImGui::Text("Window Resolution: %dx%d", windowWidth, windowHeight);
		ImGui::Checkbox("ImGui Demo Window Visibility", &demoWindowVisible);
		if (ImGui::Button(isFullscreen ? "Windowed" : "Fullscreen")) {
			isFullscreen = !isFullscreen;
			swapChain->SetFullscreenState(isFullscreen, NULL);
		}

		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Additional Elements"))
	{
		ImGui::Text("Font Scaling");
		ImGuiIO& io = ImGui::GetIO();
		const float MIN_SCALE = 0.5f;
		const float MAX_SCALE = 2.0f;
		static float window_scale = 1.0f;
		ImGui::PushItemWidth(ImGui::GetFontSize() * 8);
		if (ImGui::DragFloat("window scale", &window_scale, 0.005f, MIN_SCALE, MAX_SCALE, "%.2f", ImGuiSliderFlags_AlwaysClamp)) // Scale only this window
			ImGui::SetWindowFontScale(window_scale);
		ImGui::DragFloat("global scale", &io.FontGlobalScale, 0.005f, MIN_SCALE, MAX_SCALE, "%.2f", ImGuiSliderFlags_AlwaysClamp); // Scale everything

		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Lighting"))
	{
		ImGui::DragFloat("Brightness", &BRIGHTNESS, 0.0002f, 0.001f, 0.5f, "%.3f");
		ImGui::ColorEdit4("Background Color", &uiColor.x);

		if (ImGui::TreeNode("Scene Lights"))
		{
			ImGui::ColorEdit3("Spotlight Color", &spotLight.Color.x);
			for (int i = 0; i < lights.size(); i++)
			{
				char buf[128];
				sprintf_s(buf, "Light %i Color", i);
				// Edit color of each light
				if (ImGui::ColorEdit3(buf, &lights[i].Color.x))
				{
					//ps->SetData("lights",
					//	&lights[0],
					//	sizeof(Light) * MAX_NUM_LIGHTS);
					ps2->SetData("lights",
						&lights[0],
						sizeof(Light) * MAX_NUM_LIGHTS);
				}
			}
			ImGui::TreePop();
		}

		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Cameras"))
	{
		ImGui::RadioButton("Perspective", &cameraIndex, 0); ImGui::SameLine();
		ImGui::RadioButton("Orthographic", &cameraIndex, 1);
		std::shared_ptr<Camera> camPtr = cameras[cameraIndex];

		// Show Orthographic "zoom" slider
		if (cameraIndex == 1) {
			float orthoScale = 1 / camPtr->GetOrthoScale();
			if (ImGui::DragFloat("View Scale ", &orthoScale, 1.0f, 10.0f, 1500.0f, "%.0f"))
			{
				camPtr->SetOrthoScale(orthoScale);
				camPtr->UpdateProjMatrix((float)this->windowWidth, (float)this->windowHeight);
			}
		}
		// Show Perspective fov slider
		else {
			float fov = camPtr->GetFov() * RADTODEG;
			if (ImGui::DragFloat("FOV Scale ", &fov, 0.1f, XM_PIDIV4 * RADTODEG, XM_PIDIV2 * RADTODEG, "%.2f"))
			{
				camPtr->SetFov(fov / RADTODEG);
				camPtr->UpdateProjMatrix((float)this->windowWidth, (float)this->windowHeight);
			}
		}

		ImVec4 cameraUIColor = (ImVec4)ImColor::HSV(0.3f, 0.5f, 0.5f);
		ImGui::TextColored(cameraUIColor, "Camera Details:");
		{
			ImGui::TextColored(cameraUIColor, "Position: %0.2f, %0.2f, %0.2f",
				camPtr->GetPosition().x, camPtr->GetPosition().y, camPtr->GetPosition().z);
			ImGui::TextColored(cameraUIColor, "Rotation: %0.2f, %0.2f, %0.2f",
				camPtr->GetPitchYawRoll().x * RADTODEG, camPtr->GetPitchYawRoll().y * RADTODEG, camPtr->GetPitchYawRoll().z * RADTODEG);
			ImGui::TextColored(cameraUIColor, "Near Plane: %0.2f      Far Plane: %0.2f",
				camPtr->GetNearDist(), camPtr->GetFarDist());
			ImGui::TextColored(cameraUIColor, "Move Speed: %0.2f     Mouse Sens: %0.3f",
				camPtr->GetMoveSpeed(), camPtr->GetMouseSens());
		}


		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Meshes"))
	{
		for (size_t i = 0; i < meshes.size(); i++)
		{
			ImGui::Text("Mesh %i: %i triangle(s), %i indices", i, meshes[i]->GetIndexCount() / 3, meshes[i]->GetVertexCount());
		}

		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Scene Entities"))
	{
		for (int i = 0; i < entities.size(); i++)
		{
			char buf[128];
			sprintf_s(buf, "Entity %i", i);
			// Edit transform of each entity
			if (ImGui::TreeNode(buf))
			{
				Transform entityTransform = *entities[i].GetTransform();
				XMFLOAT3 position = entityTransform.GetPosition();
				XMFLOAT3 scale = entityTransform.GetScale();
				XMFLOAT3 rotation = entityTransform.GetPitchYawRoll();
				if (ImGui::DragFloat3("Position ", &position.x, 0.01f, -1.5f, 1.5f, "%.2f")) { entities[i].GetTransform()->SetPosition(position); }
				if (ImGui::DragFloat3("Scale ", &scale.x, 0.01f, 0.1f, 3.0f, "%.2f")) { entities[i].GetTransform()->SetScale(scale); }
				if (ImGui::DragFloat3("Rotation ", &rotation.x, 0.01f, 0.0f, 6.28f, "%.2f")) { entities[i].GetTransform()->SetRotation(rotation); }
				ImGui::TreePop();
			}
		}

		ImGui::TreePop();
	}

	ImGui::End();
}

/// <summary>
/// Gets an approximation of a direction vector to the mouse position.
/// Derived from this code on stackoverflow: https://stackoverflow.com/questions/71731722/correct-way-to-generate-3d-world-ray-from-2d-mouse-coordinates
/// </summary>
/// <returns>An approximated direction vector, derived from the mouse's screen position</returns>
DirectX::XMFLOAT3 Game::MouseRayCast()
{
	Input& input = Input::GetInstance();
	float xpos = (float)input.GetMouseX();
	float ypos = (float)input.GetMouseY();

	// converts a position from the 2d xpos, ypos to a normalized 3d direction
	float x = (2.0f * xpos) / windowWidth - 1.0f;
	float y = 1.0f - (2.0f * ypos) / windowHeight;
	float z = 1.0f;
	XMFLOAT3 ray_nds = XMFLOAT3(x, y, z);
	XMFLOAT4 ray_clip = XMFLOAT4(ray_nds.x, ray_nds.y, ray_nds.z, 1.0f);

	// eye space to clip we would multiply by projection so
	// clip space to eye space is the inverse projection
	XMFLOAT4X4 proj = cameras[cameraIndex]->GetProjMatrix();
	XMMATRIX invProj = XMMatrixInverse(nullptr, XMLoadFloat4x4(&proj));
	XMVECTOR rayEyeVec = XMVector4Transform(XMLoadFloat4(&ray_clip), invProj);

	// world space to eye space is usually multiply by view so
	// eye space to world space is inverse view
	XMFLOAT4X4 view = cameras[cameraIndex]->GetViewMatrix();
	XMMATRIX viewMatInv = XMMatrixInverse(nullptr, XMLoadFloat4x4(&view));

	// Convert float4 to float3 and normalize
	XMFLOAT4 inv_ray_wor;
	XMStoreFloat4(&inv_ray_wor, XMVector4Transform(rayEyeVec, viewMatInv));
	XMFLOAT3 ray_wor = XMFLOAT3(inv_ray_wor.x, inv_ray_wor.y, inv_ray_wor.z);
	XMVECTOR ray_wor_vec = XMVector3Normalize(XMLoadFloat3(&ray_wor));
	XMStoreFloat3(&ray_wor, ray_wor_vec);
	return ray_wor;
}
