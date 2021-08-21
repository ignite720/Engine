#include "MeshComponent.h"
#include "FBXImporter.h"
#include "Render/Renderer.h"

std::unordered_map<std::string, PipelineStateObject*> existingPiplineStateObjects;

MeshComponent::MeshComponent(const char* filename_)
{
	filename = filename_;
}

void MeshComponent::Create()
{
	//Import mesh
	fbxImporter.Import(filename.c_str(), data);

	//Setup bounds
	BoundingOrientedBox::CreateFromPoints(boundingBox, data.vertices.size(), &data.vertices[0].pos, sizeof(Vertex));

	//Setup pipeline objects
	auto psoIt = existingPiplineStateObjects.find(filename);
	if (psoIt == existingPiplineStateObjects.end())
	{
		pso->vertexBuffer.data = renderer.CreateVertexBuffer(data);
		pso->indexBuffer.data = renderer.CreateIndexBuffer(data);
		pso->sampler.data = renderer.CreateSampler();
	}
	else
	{
		pso = psoIt->second;
	}

	existingPiplineStateObjects[filename] = pso;
}

Properties MeshComponent::GetProperties()
{
	return Properties();
}
