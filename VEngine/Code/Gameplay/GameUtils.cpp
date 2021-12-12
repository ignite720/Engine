#include "GameUtils.h"
#include <filesystem>
#include "Actors/Game/Player.h"
#include "Actors/Game/Grid.h"
#include "Actors/Game/EntranceTrigger.h"
#include "Audio/AudioSystem.h"
#include "World.h"
#include "FileSystem.h"
#include "GameInstance.h"
#include "Camera.h"

namespace GameUtils
{
	Player* GetPlayer()
	{
		if (!Player::system.actors.empty())
		{
			return Player::system.actors[0];
		}

		return nullptr;
	}

	Grid* GetGrid()
	{
		if (!Grid::system.actors.empty())
		{
			return Grid::system.actors[0];
		}

		return nullptr;
	}

	void SetActiveCameraTarget(Actor* newTarget)
	{
		activeCamera->targetActor = newTarget;
	}

	void PlayAudio(const std::string audioFilename)
	{
		audioSystem.PlayAudio(audioFilename);
	}

	void SaveGameWorldState()
	{
		//The deal with save files here is the the .vmap file is the original state of the world seen in-editor
		//and the .sav version of it is based on in-game player saves. So if a .sav version exists, that is loaded
		//instead of the .vmap file, during gameplay.

		auto firstOf = world.worldFilename.find_first_of(".");
		std::string str = world.worldFilename.substr(0, firstOf);
		std::string file = str += ".vmap";

		world.worldFilename = file;
		fileSystem.WriteAllActorSystems();
	}

	void LoadWorld(std::string worldName)
	{
		std::string path = "WorldMaps/" + worldName;

		if (GameInstance::useGameSaves)
		{
			path = "GameSaves/" + worldName;
		}

		assert(std::filesystem::exists(path));
		fileSystem.LoadWorld(worldName);
	}

	void LoadWorldAndMoveToEntranceTrigger(std::string worldName)
	{
		LoadWorld(worldName);

		int matchingEntranceTriggerCount = 0;
		//Set player pos and rot at entrancetrigger in loaded world with same name as previous.
		auto entranceTriggers = world.GetAllActorsOfTypeInWorld<EntranceTrigger>();
		for (auto entrance : entranceTriggers)
		{
			if (entrance->levelToMoveTo == GameInstance::previousMapMovedFrom)
			{
				matchingEntranceTriggerCount++;

				auto player = GameUtils::GetPlayer();

				player->SetPosition(entrance->GetPosition());
				player->nextPos = player->GetPositionVector();

				player->SetRotation(entrance->GetRotationVector());
				player->nextRot = player->GetRotationVector();
			}
		}

		GameInstance::previousMapMovedFrom = worldName;

		assert(matchingEntranceTriggerCount < 2 && "Entrances with same name");
	}
}
