#include "FileSystem.h"
#include "World.h"
#include "Serialiser.h"
#include "Actors/IActorSystem.h"
#include "Actors/ActorSystemCache.h"
#include "WorldEditor.h"
#include "Editor/DebugMenu.h"
#include "Editor/Editor.h"
#include "Commands/CommandSystem.h"
#include "Core.h"
#include "GameUtils.h"
#include "Camera.h"
#include "Actors/Player.h"

FileSystem fileSystem;

void FileSystem::WriteAllActorSystems()
{
	auto lastOf = world.worldFilename.find_last_of("/\\");
	std::string str = world.worldFilename.substr(lastOf + 1);
	std::string file = "WorldMaps/" + str;

	Serialiser s(file, OpenMode::Out);

	for (IActorSystem* actorSystem : world.activeActorSystems)
	{
		actorSystem->Serialise(s);
	}

	debugMenu.AddNotification(L"World saved.");
}

void FileSystem::LoadWorld(std::string worldName)
{
	world.worldFilename = worldName;

	std::string path = "WorldMaps/" + worldName;

	world.Cleanup();

	Deserialiser d(path, OpenMode::In);

	while (!d.is.eof())
	{
		std::string actorSystemName;
		d.is >> actorSystemName;

		if (actorSystemName.empty())
		{
			break;
		}

		size_t numActorsToSpawn = 0;
		d.is >> numActorsToSpawn;

		auto asIt = actorSystemCache.nameToSystemMap->find(actorSystemName);
		if (asIt == actorSystemCache.nameToSystemMap->end())
		{
			continue;
		}

		IActorSystem* actorSystem = asIt->second;

		for (int i = 0; i < numActorsToSpawn; i++)
		{
			actorSystem->SpawnActor(Transform());
		}

		actorSystem->Deserialise(d);
	}

	world.Start();

	//Deselect any existing actors, because TransformGizmo will stay at previous positions.
	worldEditor.pickedActor = nullptr;

	//Clear all commands
	commandSystem.commands.clear();

	//Set player camera on world change as active if in-gameplay
	if (Core::gameplayOn)
	{
		activeCamera = GameUtils::GetPlayer()->camera;
	}

	editor->UpdateWorldList();
	debugMenu.AddNotification(L"World loaded.");
}

void FileSystem::ReloadCurrentWorld()
{
	LoadWorld(world.worldFilename);
}
