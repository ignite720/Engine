#include "vpch.h"
#include "MeshComponent.h"
#include "Core/VMath.h"
#include "Core/Camera.h"
#include "Actors/Game/Player.h"
#include "Asset/FBXLoader.h"
#include "Asset/AssetSystem.h"
#include "Gameplay/GameUtils.h"
#include "Render/RenderUtils.h"
#include "Render/ShaderSystem.h"
#include "Render/Material.h"
#include "Render/MaterialSystem.h"
#include "Render/Renderer.h"
#include "Render/TextureSystem.h"
#include "Render/VertexShader.h"
#include "Render/PixelShader.h"
#include "Physics/PhysicsSystem.h"

//Vertex and index buffers linked to a mesh filename to copy over to new PSOs
std::unordered_map<std::string, std::unique_ptr<MeshBuffers>> existingMeshBuffers;

void MeshComponent::ResetMeshBuffers()
{
	existingMeshBuffers.clear();
}

std::vector<MeshComponent*> MeshComponent::SortMeshComponentsByDistanceToCamera()
{
	XMVECTOR cameraPos = activeCamera->GetWorldPositionV();

	std::multimap<float, MeshComponent*> meshDistanceMap;
	for (auto& mesh : system.GetComponents())
	{
		const float distance = XMVector3Length(cameraPos - mesh->GetWorldPositionV()).m128_f32[0];
		meshDistanceMap.emplace(distance, mesh.get());
	}

	std::vector<MeshComponent*> sortedMeshes;
	for (auto& meshPair : meshDistanceMap)
	{
		sortedMeshes.emplace_back(meshPair.second);
	}
	std::reverse(sortedMeshes.begin(), sortedMeshes.end());

	return sortedMeshes;
}

MeshComponent::MeshComponent()
{
	material = MaterialSystem::CreateMaterial("test.png", ShaderItems::Default);
}

MeshComponent::MeshComponent(const std::string filename_,
	const std::string textureFilename_,
	ShaderItem* shaderItem)
{
	meshComponentData.filename = filename_;
	material = MaterialSystem::CreateMaterial(textureFilename_, shaderItem);
}

void MeshComponent::Tick(float deltaTime)
{
	if (!isStatic)
	{
		PhysicsSystem::GetTransformFromPhysicsActor(this);
	}
}

void MeshComponent::Create()
{
	//Set 'this' so mesh widgets can access this component.
	meshComponentData.meshComponent = this;

	//Material's create needs to be called here to deal with serialisation
	material->Create();

	meshDataProxy = AssetSystem::ReadVMeshAssetFromFile(meshComponentData.filename);

	//Setup bounds
	BoundingOrientedBox::CreateFromBoundingBox(boundingBox, *meshDataProxy.boundingBox);

	//Setup pipeline objects
	auto psoIt = existingMeshBuffers.find(meshComponentData.filename);
	if (psoIt == existingMeshBuffers.end())
	{
		existingMeshBuffers[meshComponentData.filename] = std::make_unique<MeshBuffers>();
		auto& meshBuffers = existingMeshBuffers[meshComponentData.filename];

		meshBuffers->vertexBuffer.data = RenderUtils::CreateVertexBuffer(meshDataProxy);

		pso.vertexBuffer = &meshBuffers->vertexBuffer;
	}
	else
	{
		pso.vertexBuffer = &psoIt->second->vertexBuffer;
	}
}

void MeshComponent::Destroy()
{
	//Erase physics actor
	PhysicsSystem::ReleasePhysicsActor(this);

	material->Destroy();
	material = nullptr;
}

static void ReassignMesh(void* data)
{
	auto meshData = (MeshComponentData*)data;
	meshData->meshComponent->Create();
}

Properties MeshComponent::GetProps()
{
	auto props = __super::GetProps();
	props.title = "MeshComponent";

	props.Add("Mesh", &meshComponentData).change = ReassignMesh;
	props.Add("Casts Shadow", &castsShadow);
	props.Add("Static", &isStatic);
	props.Add("Trans. Occlude", &transparentOcclude);

	auto materialProps = material->GetProps();
	props.Merge(materialProps);

	return props;
}

void MeshComponent::SplitMeshCreate()
{
	meshComponentData.meshComponent = this;

	material->Create();

	auto buffer = new Buffer();
	buffer->data = RenderUtils::CreateVertexBuffer(meshDataProxy);
	pso.vertexBuffer = buffer;

	//Make sure bounds setup is before physics actor creation
	BoundingBox bb;
	BoundingBox::CreateFromPoints(bb, meshDataProxy.vertices->size(),
		&meshDataProxy.vertices->at(0).pos, sizeof(Vertex));
	BoundingOrientedBox::CreateFromBoundingBox(boundingBox, bb);

	//@Todo: owner being set as null on CreatePhysicsActor() won't explode the program,
	//but it will cause problems if you want to use raycasts via PhysX.
	PhysicsSystem::CreateConvexPhysicsMesh(this, GetOwner());
}

void MeshComponent::SetMeshFilename(std::string_view meshFilename)
{
	meshComponentData.filename = meshFilename;
}

void MeshComponent::SetRastState(const std::string newRastStateName)
{
	//If calling before Material::Create() is called, rastStateName is the one.
	material->rastStateValue.SetValue(newRastStateName);
	//This set isn't necessary with the above, but fine for after Material::Create() called.
	material->rastState = Renderer::GetRastState(newRastStateName);
}

void MeshComponent::SetBlendState(const std::string newBlendState)
{
	material->blendState = Renderer::GetBlendState(newBlendState);
}

void MeshComponent::SetTexture(const std::string newTextureName)
{
	material->texture = TextureSystem::FindTexture2D(newTextureName);
	material->textureData.filename = newTextureName;
}

void MeshComponent::SetShaderFilenames(ShaderItem* shaderItem)
{
	material->shaderData.shaderItemName = shaderItem->GetName();
}

void MeshComponent::SetAmbientColour(const XMFLOAT4 ambientColour)
{
	material->materialShaderData.ambient = ambientColour;
}

XMFLOAT4 MeshComponent::GetAmbientColour()
{
	return material->materialShaderData.ambient;
}

void MeshComponent::SetUseTexture(bool useTexture)
{
	material->materialShaderData.useTexture = useTexture;
}

Buffer* MeshComponent::GetVertexBuffer() const 
{
	return pso.vertexBuffer;
}

std::vector<XMFLOAT3> MeshComponent::GetAllVertexPositions()
{
	std::vector<XMFLOAT3> positions;
	for (auto& vertex : *meshDataProxy.vertices)
	{
		positions.emplace_back(vertex.pos);
	}
	return positions;
}
