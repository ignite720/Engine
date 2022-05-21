#pragma once
#include <string>

struct ShaderItem;
struct ID3D11Buffer;
struct ID3D11Resource;
struct ID3D11ShaderResourceView;
struct ID3D11SamplerState;
struct ID3D11BlendState;
struct ID3D11RasterizerState;

struct Buffer
{
	ID3D11Buffer* data = nullptr;
};

struct Texture2D
{
	std::string filename;

	ID3D11Resource* data = nullptr;
	ID3D11ShaderResourceView* srv = nullptr;

	uint32_t width = 0;
	uint32_t height = 0;

	Texture2D() {}
	Texture2D(std::string filename_);
	~Texture2D();
};

struct ShaderResourceView
{
	ID3D11ShaderResourceView* data = nullptr;
};

struct Sampler
{
	ID3D11SamplerState* data = nullptr;

	Sampler(ID3D11SamplerState* data_);
};

struct RastState
{
	ID3D11RasterizerState* data = nullptr;
	std::string name;

	RastState() {}
	RastState(std::string name_, ID3D11RasterizerState* data_);
};

struct BlendState
{
	ID3D11BlendState* data = nullptr;
	std::string name;

	BlendState() {}
	BlendState(std::string name_, ID3D11BlendState* data_);
};

struct MeshBuffers
{
	Buffer* vertexBuffer = nullptr;
	Buffer* indexBuffer = nullptr;
};

struct PipelineStateObject
{
	Buffer* vertexBuffer = nullptr;
	Buffer* indexBuffer = nullptr;

	PipelineStateObject();
	void Create();
	void SetVertexBuffer(Buffer* vertexBuffer);
	void SetIndexBuffer(Buffer* indexBuffer);
};
