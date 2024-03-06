#include "vpch.h"
#include "RenderUtils.h"
#include "Texture2D.h"
#include "PipelineObjects.h"
#include "Core/Debug.h"
#include "Core/VString.h"
#include "MeshDataProxy.h"
#include "Renderer.h"

namespace RenderUtils
{
	void CreateDefaultBuffer(uint64_t byteWidth, uint32_t bindFlags, const void* initData, Microsoft::WRL::ComPtr<ID3D11Buffer>& outputBuffer)
	{
		D3D11_BUFFER_DESC desc = {};
		desc.ByteWidth = byteWidth;
		desc.BindFlags = bindFlags;
		desc.Usage = D3D11_USAGE_DEFAULT;

		D3D11_SUBRESOURCE_DATA data = {};
		data.pSysMem = initData;

		HR(Renderer::GetDevice().CreateBuffer(&desc, &data, outputBuffer.ReleaseAndGetAddressOf()));

		SetResourceName(outputBuffer.Get(), "default_buffer_" + std::to_string(GenerateUID()));
	}

	void CreateDynamicBuffer(uint64_t byteWidth, uint32_t bindFlags, const void* initData, Microsoft::WRL::ComPtr<ID3D11Buffer>& outputBuffer)
	{
		D3D11_BUFFER_DESC desc = {};
		desc.ByteWidth = RenderUtils::CalcBufferByteSize(byteWidth);
		desc.BindFlags = bindFlags;
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		D3D11_SUBRESOURCE_DATA data = {};
		data.pSysMem = initData;

		HR(Renderer::GetDevice().CreateBuffer(&desc, &data, outputBuffer.ReleaseAndGetAddressOf()));

		SetResourceName(outputBuffer.Get(), "dynamic_buffer_" + std::to_string(GenerateUID()));
	}

	void CreateVertexBuffer(MeshDataProxy& meshData, Microsoft::WRL::ComPtr<ID3D11Buffer>& outputBuffer)
	{
		CreateDefaultBuffer(meshData.GetVerticesByteWidth(),
			D3D11_BIND_VERTEX_BUFFER, meshData.vertices.data(), outputBuffer);
	}

	void CreateSRVForMeshInstance(ID3D11Buffer* structuredBuffer, uint32_t numBufferElements, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& outputSrv)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
		srvDesc.BufferEx.NumElements = numBufferElements;

		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;
		HR(Renderer::GetDevice().CreateShaderResourceView(structuredBuffer, &srvDesc, outputSrv.ReleaseAndGetAddressOf()));

		SetResourceName(outputSrv.Get(), "srv_" + std::to_string(GenerateUID()));
	}

	void CreateStructuredBuffer(uint32_t byteWidth, uint32_t byteStride, const void* initData, Microsoft::WRL::ComPtr<ID3D11Buffer>& outputBuffer)
	{
		D3D11_BUFFER_DESC desc = {};
		desc.ByteWidth = byteWidth;
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.StructureByteStride = byteStride;
		desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		D3D11_SUBRESOURCE_DATA data = {};
		data.pSysMem = initData;

		HR(Renderer::GetDevice().CreateBuffer(&desc, &data, outputBuffer.ReleaseAndGetAddressOf()));

		SetResourceName(outputBuffer.Get(), "structured_buffer_" + std::to_string(GenerateUID()));
	}

	void CreateSampler(Sampler& sampler)
	{
		D3D11_SAMPLER_DESC sampDesc = {};
		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;

		HR(Renderer::GetDevice().CreateSamplerState(&sampDesc, sampler.data.ReleaseAndGetAddressOf()));

		SetResourceName(sampler.data.Get(), "sampler_" + std::to_string(GenerateUID()));
	}

	void SetResourceName(ID3D11DeviceChild* resource, std::string name)
	{
		//@Todo: _DEBUG
		HR(resource->SetPrivateData(WKPDID_D3DDebugObjectName, name.size(), name.c_str()));
	}

	UINT CalcBufferByteSize(UINT byteSize)
	{
		return (byteSize + 255) & ~255;
	}
}
