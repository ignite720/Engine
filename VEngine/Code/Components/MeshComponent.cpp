#include "MeshComponent.h"
#include "Asset/FBXImporter.h"
#include "Render/RenderUtils.h"
#include "Render/ShaderSystem.h"
#include "Render/Material.h"

//Vertex and index buffers linked to a mesh filename to copy over to new PSOs
std::unordered_map<std::string, MeshBuffers> existingMeshBuffers;

MeshComponent::MeshComponent(const std::string filename_, 
	const std::string textureFilename_,
	const std::string shaderFilename_)
{
	meshComponentData.filename = filename_;

	data = new MeshDataProxy();
	pso = new PipelineStateObject();

	material = new Material(textureFilename_, shaderFilename_);
}

void MeshComponent::Tick(float deltaTime)
{
}

void MeshComponent::Create()
{
	//Material's create needs to be called here to deal with serialisation
	material->Create();

	//Import mesh (set up bounding box in here too so you don't need to re-create bounds)
	fbxImporter.Import(meshComponentData.filename.c_str(), data);

	//Setup bounds
	BoundingOrientedBox::CreateFromPoints(boundingBox, data->vertices->size(),
		&data->vertices->at(0).pos, sizeof(Vertex));

	//Setup pipeline objects
	auto psoIt = existingMeshBuffers.find(meshComponentData.filename);
	if (psoIt == existingMeshBuffers.end())
	{
		pso->vertexBuffer = new Buffer();
		pso->indexBuffer = new Buffer();
		pso->vertexBuffer->data = RenderUtils::CreateVertexBuffer(data);
		pso->indexBuffer->data = RenderUtils::CreateIndexBuffer(data);
	}
	else
	{
		pso->vertexBuffer = psoIt->second.vertexBuffer;
		pso->indexBuffer = psoIt->second.indexBuffer;
	}

	pso->Create();

	MeshBuffers meshBuffers = {};
	meshBuffers.vertexBuffer = pso->vertexBuffer;
	meshBuffers.indexBuffer = pso->indexBuffer;
	existingMeshBuffers[meshComponentData.filename] = meshBuffers;
}

static void ReassignMesh(void* data)
{

}

Properties MeshComponent::GetProps()
{
	Properties props("MeshComponent");
	props.Add("Mesh", &meshComponentData);
	props.Merge(material->GetProps());
	return props;
}
