#include "MeshComponent.h"
#include "Asset/FBXImporter.h"
#include "Render/RenderUtils.h"
#include "Render/ShaderSystem.h"
#include "Render/Material.h"
#include "WorldEditor.h"
#include "Editor/Editor.h"

//Vertex and index buffers linked to a mesh filename to copy over to new PSOs
std::unordered_map<std::string, MeshBuffers> existingMeshBuffers;

MeshComponent::MeshComponent(const std::string filename_, 
	const std::string textureFilename_,
	const std::string shaderFilename_)
{
	meshDataProxy = new MeshDataProxy();
	pso = new PipelineStateObject();

	meshComponentData.filename = filename_;

	material = new Material(textureFilename_, shaderFilename_);
}

void MeshComponent::Tick(float deltaTime)
{
}

void MeshComponent::Create()
{
	//Set 'this' so mesh widgets can access this component.
	meshComponentData.meshComponent = this;

	//Material's create needs to be called here to deal with serialisation
	material->Create();

	//Import mesh (set up bounding box in here too so you don't need to re-create bounds)
	fbxImporter.Import(meshComponentData.filename.c_str(), meshDataProxy);

	//Setup bounds
	BoundingOrientedBox::CreateFromPoints(boundingBox, meshDataProxy->vertices->size(),
		&meshDataProxy->vertices->at(0).pos, sizeof(Vertex));

	//Setup pipeline objects
	auto psoIt = existingMeshBuffers.find(meshComponentData.filename);
	if (psoIt == existingMeshBuffers.end())
	{
		pso->vertexBuffer = new Buffer();
		pso->indexBuffer = new Buffer();
		pso->vertexBuffer->data = RenderUtils::CreateVertexBuffer(meshDataProxy);
		pso->indexBuffer->data = RenderUtils::CreateIndexBuffer(meshDataProxy);
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

void MeshComponent::Destroy()
{
	delete material;
	delete meshDataProxy;
	delete pso;
}

static void ReassignMesh(void* data)
{
	auto meshData = (MeshComponentData*)data;
	meshData->meshComponent->Create();
}

Properties MeshComponent::GetProps()
{
	Properties props("MeshComponent");
	props.Add("Mesh", &meshComponentData).change = ReassignMesh;
	props.Merge(material->GetProps());
	return props;
}
