#include "InstanceMeshComponent.h"
#include "Render/RenderUtils.h"
#include "Render/Material.h"

InstanceMeshComponent::InstanceMeshComponent(uint32_t meshInstanceRenderCount_,
	const std::string filename,
	const std::string textureFilename)
	: MeshComponent(filename, textureFilename, "InstanceShader.hlsl")
{
	meshInstanceRenderCount = meshInstanceRenderCount_;

	instanceData.reserve(meshInstanceRenderCount);
}

InstanceMeshComponent::~InstanceMeshComponent()
{
	if(structuredBuffer) structuredBuffer->Release();
	if(srv) srv->Release();
}

void InstanceMeshComponent::Create()
{
	MeshComponent::Create();

	instanceData.push_back(InstanceData());

	//Setup shader buffers
	structuredBuffer = RenderUtils::CreateStructuredBuffer(sizeof(InstanceData) * meshInstanceRenderCount,
		sizeof(InstanceData), instanceData.data());

	srv = RenderUtils::CreateSRVForMeshInstance(structuredBuffer, meshInstanceRenderCount);
}

void InstanceMeshComponent::SetInstanceCount(uint32_t count)
{
	meshInstanceRenderCount = count;
}

uint32_t InstanceMeshComponent::GetInstanceCount()
{
	return meshInstanceRenderCount;
}

void InstanceMeshComponent::SetInstanceData(std::vector<InstanceData>& instanceData_)
{
	instanceData.clear();
	instanceData = instanceData_;
}

Properties InstanceMeshComponent::GetProps()
{
	Properties props = MeshComponent::GetProps();
	props.title = "InstanceMeshComponent";
	return props;
}
