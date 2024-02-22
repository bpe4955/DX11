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
XMFLOAT4 uiColor(0.4f, 0.6f, 0.75f, 1.0f); // Default Cornflower Blue
bool demoWindowVisible = false;
bool isFullscreen = false;

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
	// Helper methods for loading shaders, creating some basic
	// geometry to draw and some simple camera matrices.
	//  - You'll be expanding and/or replacing these later
	LoadShaders();
	CreateGeometry();
	
	// Set initial graphics API state
	//  - These settings persist until we change them
	//  - Some of these, like the primitive topology & input layout, probably won't change
	//  - Others, like setting shaders, will need to be moved elsewhere later
	{
		// Tell the input assembler (IA) stage of the pipeline what kind of
		// geometric primitives (points, lines or triangles) we want to draw.  
		// Essentially: "What kind of shape should the GPU draw with our vertices?"
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	}

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
		(float)this->windowWidth, (float)this->windowHeight, XMFLOAT3(0.0f, 0.0f, -1.0f)));
	cameras.push_back(std::make_shared<Camera>(
		(float)this->windowWidth, (float)this->windowHeight, XMFLOAT3(0.0f, 0.0f, -10.0f)));
	cameras[1]->UpdateProjMatrix(false, (float)windowWidth, (float)windowHeight);
	cameras[1]->SetMouseSens(0.005f);
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
	ps = std::make_shared<SimplePixelShader>(device, context, FixPath(L"PixelShader.cso").c_str());
	vs = std::make_shared<SimpleVertexShader>(device, context, FixPath(L"VertexShader.cso").c_str());
}



// --------------------------------------------------------
// Creates the geometry we're going to draw 
// --------------------------------------------------------
void Game::CreateGeometry()
{
	// Create some temporary variables to represent colors
	// - Not necessary, just makes things more readable
	XMFLOAT4 red	= XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4 green	= XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	XMFLOAT4 blue	= XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
	XMFLOAT4 teal = XMFLOAT4(0.0f, 0.75f, 0.5f, 1.0f);

	Vertex vertices[] =
	{
		{ XMFLOAT3(+0.0f, +0.5f, +0.0f), red },
		{ XMFLOAT3(+0.5f, -0.5f, +0.0f), blue },
		{ XMFLOAT3(-0.5f, -0.5f, +0.0f), green },
	};

	unsigned int indices[] = { 0, 1, 2 };

	meshes.push_back(std::make_shared<Mesh>(&vertices[0], 3, &indices[0], 3, context, device));

	Vertex vertices1[] =
	{
		{ XMFLOAT3(+0.45f, +0.5f, +0.0f), blue }, // top left 0
		{ XMFLOAT3(+0.85f, +0.5f, +0.0f), blue }, // top right 1 
		{ XMFLOAT3(+0.45f, +0.1f, +0.0f), green }, // bottom left 2
		{ XMFLOAT3(+0.85f, +0.1f, +0.0f), green }, // bottom right 3
	};

	unsigned int indices1[] = { 0, 1, 2, 1, 3, 2 };
	
	meshes.push_back(std::make_shared<Mesh>(&vertices1[0], 4, &indices1[0], 6, context, device));

	// Centered around -0.50, +0.25
	// Radius 0.25
	Vertex vertices2[] =
	{
		{ XMFLOAT3(-0.65f, +0.375f, +0.0f), blue }, // top left ex 
		{ XMFLOAT3(-0.5f, +0.50f, +0.0f), blue }, // top 
		{ XMFLOAT3(-0.35f, +0.375f, +0.0f), blue }, // top right ex 
		{ XMFLOAT3(-0.375f, +0.125f, +0.0f), blue }, // bottom right ex 
		{ XMFLOAT3(-0.625f, +0.125f, +0.0f), blue }, // bottom left ex 
		{ XMFLOAT3(-0.55f, +0.375f, +0.0f), teal }, // top left int
		{ XMFLOAT3(-0.45f, +0.375f, +0.0f), teal }, // top right int
		{ XMFLOAT3(-0.425f, +0.25f, +0.0f), teal }, // bottom right int 
		{ XMFLOAT3(-0.5f, +0.175f, +0.0f), teal }, // bottom 
		{ XMFLOAT3(-0.575f, +0.25f, +0.0f), teal }, // bottom left int 
		{ XMFLOAT3(-0.50f, +0.25f, +0.0f), green }, // bottom left int 
	};

	unsigned int indices2[] = { 1, 6, 5, 2, 7, 6, 3, 8, 7, 4, 9, 8, 0, 5, 9, 10, 8, 9, 10, 9, 5, 10, 5, 6, 10, 6, 7, 10, 7, 8  };

	meshes.push_back(std::make_shared<Mesh>(&vertices2[0], 11, &indices2[0], 10*3, context, device));

	for (size_t i = 0; i < meshes.size(); i++)
	{
		entities.push_back(Entity(meshes[i]));
		entities.push_back(Entity(meshes[i]));
		entities[i * 2 + 1].GetTransform()->SetPosition(XMFLOAT3(-0.5f, -0.5f, 0));
	}
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
	UpdateUI(deltaTime);

	// Update Camera
	cameras[cameraIndex]->Update(deltaTime);

	//Move Entities
	for (size_t i = 0; i < entities.size(); i+=2)
	{
		entities[i].GetTransform()->Rotate(0, 0, deltaTime);
	}
	for (size_t i = 1; i < entities.size(); i += 2)
	{
		entities[i].GetTransform()->SetPosition((float)sin(totalTime), 0, 0);
	}

	// Example input checking: Quit if the escape key is pressed
	if (Input::GetInstance().KeyDown(VK_ESCAPE))
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
	for (size_t i = 0; i < entities.size(); i++)
	{
		entities[i].Draw(context, vs, ps, cameras[cameraIndex]);
	}

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
	if(demoWindowVisible) { ImGui::ShowDemoWindow(); }

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
		if(fps > 60) { ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Framerate: %f fps", fps); }
		else if (fps > 30) { ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Framerate: %f fps", fps); }
		else { ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Framerate: %f fps", fps); }
		ImGui::Text("Frame Count: %d", ImGui::GetFrameCount());
		ImGui::Text("Window Resolution: %dx%d", windowWidth, windowHeight);
		ImGui::ColorEdit4("Background Color", &uiColor.x);
		ImGui::Checkbox("ImGui Demo Window Visibility", &demoWindowVisible);

		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Additional Elements"))
	{
		if (ImGui::Button(isFullscreen ? "Windowed" : "Fullscreen")) {
			isFullscreen = !isFullscreen;
			swapChain->SetFullscreenState(isFullscreen, NULL); 
		}

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
			float fov = camPtr->GetFov()*RADTODEG;
			if (ImGui::DragFloat("FOV Scale ", &fov, 0.1f, XM_PIDIV4*RADTODEG, XM_PIDIV2*RADTODEG, "%.2f"))
			{
				camPtr->SetFov(fov/RADTODEG);
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
			ImGui::Text("Mesh %i: %i triangle(s), %i indices", i, meshes[i]->GetIndexCount()/3, meshes[i]->GetVertexCount());
		}

		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Scene Entities"))
	{
		for (int i = 0; i < entities.size(); i++)
		{
			char buf[128];
			sprintf_s(buf, "Entity %i",i);
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

		//ImGui::ColorEdit4("Color Tint", &cbColor.x);

		ImGui::TreePop();
	}

	ImGui::End();
}
