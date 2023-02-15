#include "vpch.h"
#include "World.h"
#include "VMath.h"
#include "FileSystem.h"
#include "Profile.h"
#include "Core.h"
#include "Timer.h"
#include "Log.h"
#include "Asset/FBXLoader.h"
#include "Actors/MeshActor.h"
#include "Actors/Game/Player.h"
#include "Actors/Game/LevelInstance.h"
#include "Actors/DirectionalLightActor.h"
#include "Actors/ActorSystemCache.h"
#include "Components/ComponentSystemCache.h"
#include "Components/IComponentSystem.h"
#include "Components/MeshComponent.h"
#include "Render/TextureSystem.h"
#include "Render/MaterialSystem.h"
#include "Render/Material.h"
#include "Audio/AudioSystem.h"
#include "Render/SpriteSystem.h"
#include "UI/UISystem.h"
#include "Gameplay/GameInstance.h"
#include "Gameplay/GameUtils.h"
#include "Physics/PhysicsSystem.h"

std::string World::worldFilename;

std::map<UID, Actor*> World::actorUIDMap;
std::map<std::string, Actor*> World::actorNameMap;

std::vector<IActorSystem*> World::activeActorSystems;
std::vector<IComponentSystem*> World::activeComponentSystems;

void World::Init()
{
	for (auto actorSystem : ActorSystemCache::Get().GetAllSystems())
	{
		activeActorSystems.emplace_back(actorSystem);
	}

	for (auto componentSystem : ComponentSystemCache::Get().GetAllSystems())
	{
		activeComponentSystems.emplace_back(componentSystem);
	}

	//Load starting map
	FileSystem::LoadWorld(GameInstance::startingMap);
}

void World::Start()
{
	//@Todo: creating all materials here isn't working. All meshes are creating materials twice on world loads,
	//once here then again in MeshComponent::Create()
	MaterialSystem::CreateAllMaterials();
	TextureSystem::CreateAllTextures();

	for (auto actorSystem : activeActorSystems)
	{
		actorSystem->Init();
	}

	for (auto componentSystem : activeComponentSystems)
	{
		componentSystem->Init();
	}

	for (auto actorSystem : activeActorSystems)
	{
		actorSystem->PostInit();
	}

	if (Core::gameplayOn)
	{
		StartAllComponents();
		WakeAndStartAllActors();

		GameUtils::LoadGameInstanceData();
	}
}

void World::WakeAndStartAllActors()
{
	auto actors = GetAllActorsInWorld();

	for (auto actor : actors)
	{
		actor->Awake();
	}

	for (auto actor : actors)
	{
		actor->Start();
	}

	for (auto actor : actors)
	{
		actor->LateStart();
	}
}

void World::StartAllComponents()
{
	auto components = GetAllComponentsInWorld();

	for (auto component : components)
	{
		component->Start();
	}
}

void World::EndAllActors()
{
	for (auto actor : GetAllActorsInWorld())
	{
		actor->End();
	}
}

void World::CreateDefaultMapActors()
{
	auto player = Player::system.Add();
	player->Create();
	player->CreateAllComponents();

	auto levelInstance = LevelInstance::system.Add();
	levelInstance->SetPosition(XMFLOAT3(2.f, 4.f, 2.f));
	levelInstance->SetExtents(5.f, 5.f, 5.f);

	auto dlight = DirectionalLightActor::system.Add();
	//Set light pointing down because shadows looks nice.
	dlight->SetRotation(
		VMath::LookAtRotation(dlight->GetPositionV() - VMath::GlobalUpVector(), dlight->GetPositionV()));
	dlight->SetPosition(XMVectorSet(0.f, 5.f, 0.f, 1.f));

	//Set a ground plane to work off
	MeshActor::spawnMeshFilename = "node.vmesh";
	auto mesh = MeshActor::system.Add();
	mesh->SetPosition(XMFLOAT3(2.f, -0.5f, 2.f));
	mesh->SetScale(XMVectorSet(5.f, 1.f, 5.f, 1.0f));
	mesh->Create();
	mesh->CreateAllComponents();

	MeshActor::spawnMeshFilename.clear();

	editor->UpdateWorldList();
}

std::vector<IActorSystem*> World::GetLayerActorSystems()
{
	std::vector<IActorSystem*> actorSystems;

	for (auto actorSystem : activeActorSystems)
	{
		if (actorSystem->GetName() == "EntranceTrigger" ||
			actorSystem->GetName() == "MeshActor")
		{
			continue;
		}

		actorSystems.emplace_back(actorSystem);
	}

	return actorSystems;
}

void World::TickAllActorSystems(float deltaTime)
{
	Profile::Start();

	for (IActorSystem* actorSystem : activeActorSystems)
	{
		actorSystem->Tick(deltaTime);
	}

	Profile::End();
}

void World::TickAllComponentSystems(float deltaTime)
{
	Profile::Start();

	for (IComponentSystem* componentSystem : activeComponentSystems)
	{
		componentSystem->Tick(deltaTime);
	}

	Profile::End();
}

std::vector<Actor*> World::GetAllActorsInWorld()
{
	std::vector<Actor*> outActors;

	for (IActorSystem* actorSystem : activeActorSystems)
	{
		auto actors = actorSystem->GetActorsAsBaseClass();
		outActors.insert(outActors.end(), actors.begin(), actors.end());
	}

	return outActors;
}

Actor* World::GetActorByUID(UID uid)
{
	auto actorIt = actorUIDMap.find(uid);
	assert(actorIt != actorUIDMap.end());
	return actorIt->second;
}

Actor* World::GetActorByUIDAllowNull(UID uid)
{
	auto actorIt = actorUIDMap.find(uid);
	if (actorIt == actorUIDMap.end())
	{
		return nullptr;
	}

	return actorIt->second;
}

Actor* World::GetActorByName(std::string actorName)
{
	auto actorIt = actorNameMap.find(actorName);
	assert(actorIt != actorNameMap.end());
	return actorIt->second;
}

Actor* World::GetActorByNameAllowNull(std::string actorName)
{
	Actor* foundActor = nullptr;
	foundActor = actorNameMap[actorName];
	if (foundActor == nullptr)
	{
		Log("%s actor not found.", actorName.c_str());
	}
	return foundActor;
}

std::vector<Component*> World::GetAllComponentsInWorld()
{
	std::vector<Component*> outComponents;

	for (IComponentSystem* componentSystem : activeComponentSystems)
	{
		auto components = componentSystem->GetComponentsAsBaseClass();
		outComponents.insert(outComponents.end(), components.begin(), components.end());
	}

	return outComponents;
}

void World::AddActorToWorld(Actor* actor)
{
	std::string actorName = actor->GetName();
	UID actorUID = actor->GetUID();

	actorUIDMap.emplace(actorUID, actor);
	actorNameMap.emplace(actorName, actor);
}

void World::RemoveActorFromWorld(Actor* actor)
{
	actorUIDMap.erase(actor->GetUID());
	actorNameMap.erase(actor->GetName());
}

void World::RemoveActorFromWorld(UID actorUID)
{
	Actor* actor = actorUIDMap.find(actorUID)->second;
	actorNameMap.erase(actor->GetName());
	actorUIDMap.erase(actorUID);
}

void World::RemoveActorFromWorld(std::string actorName)
{
	Actor* actor = actorNameMap.find(actorName)->second;
	actorUIDMap.erase(actor->GetUID());
	actorNameMap.erase(actorName);
}

void World::ClearAllActorsFromWorld()
{
	actorUIDMap.clear();
	actorNameMap.clear();
}

bool World::CheckIfActorExistsInWorld(std::string actorName)
{
	return actorNameMap.find(actorName) != actorNameMap.end();
}

bool World::CheckIfActorExistsInWorld(UID actorUID)
{
	return actorUIDMap.find(actorUID) != actorUIDMap.end();
}

void World::Cleanup()
{
	actorUIDMap.clear();
	actorNameMap.clear();

	Timer::Cleanup();
	PhysicsSystem::Reset();
	AudioSystem::DeleteLoadedAudioAndChannels();
	TextureSystem::Cleanup();
	MaterialSystem::Cleanup();
	SpriteSystem::Reset();
	UISystem::Reset();

	//@Todo: want these functions here but debug mesh data (light point light mesh) which is static loses
	//its buffer and mesh data on load.
	//MeshComponent::ResetMeshBuffers();
	//FBXLoader::ClearExistingMeshData();
	//FBXLoader::ClearExistingSkeletonData();

	for (auto componentSystem : activeComponentSystems)
	{
		componentSystem->Cleanup();
	}

	for (auto actorSystem : activeActorSystems)
	{
		actorSystem->Cleanup();
	}
}
