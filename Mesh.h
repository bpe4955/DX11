#pragma once
#include <wrl/client.h>
#include <d3d11.h>
#include "Vertex.h"
#include "PathHelpers.h"
#include <string>

class Mesh
{
public:
	Mesh(Vertex* vertices, int _vertexCount,
		unsigned int* indices, int _indexCount,
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> _context,
		Microsoft::WRL::ComPtr<ID3D11Device> _device);
	Mesh(std::wstring relativeFilePath,
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> _context,
		Microsoft::WRL::ComPtr<ID3D11Device> _device);
	Mesh(std::string relativeFilePath,
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> _context,
		Microsoft::WRL::ComPtr<ID3D11Device> _device);
	Mesh(const char* relativeFilePath,
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> _context,
		Microsoft::WRL::ComPtr<ID3D11Device> _device);
	~Mesh();
	/// <summary>
	/// Create the Vertex and Index buffers for the mesh
	/// </summary>
	/// <param name="vertices">The mesh's vertices</param>
	/// <param name="indices">The mesh's indices</param>
	/// <param name="_device">The ID3D11Device we are creating buffers with</param>
	void CreateBuffers(Vertex* vertices, unsigned int* indices, Microsoft::WRL::ComPtr<ID3D11Device> _device);
	void LoadModelAssimp(std::string relativeFilePath);
	void LoadModelGiven(std::string relativeFilePath);
	/// <summary>
	/// Returns the Vertex Buffer ComPtr
	/// </summary>
	/// <returns>The Vertex Buffer ComPtr</returns>
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetVertexBuffer();
	/// <summary>
	/// Returns the Index Buffer ComPtr
	/// </summary>
	/// <returns>The Index Buffer ComPtr</returns>
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetIndexBuffer();
	/// <summary>
	/// Returns the number of indices this mesh contains
	/// </summary>
	/// <returns>The number of indices this mesh contains</returns>
	int GetIndexCount();
	/// <summary>
	/// Returns the number of vertices this mesh contains
	/// </summary>
	/// <returns>The number of vertices this mesh contains</returns>
	int GetVertexCount();
	/// <summary>
	/// Activates the buffers and draws the correct number of indices
	/// </summary>
	void Draw();

private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	int vertexCount;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
	int indexCount;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
	Microsoft::WRL::ComPtr<ID3D11Device> device;

};

