#include "vpch.h"
#include "GameUtils.h"
#include <filesystem>
#include "Actors/Game/Player.h"
#include "Audio/AudioSystem.h"
#include "Core/World.h"
#include "Core/FileSystem.h"
#include "GameInstance.h"
#include "Core/Camera.h"
#include "Components/CameraComponent.h"
#include "UI/UISystem.h"
#include "UI/ScreenFadeWidget.h"
#include "Core/Input.h"
#include "Core/Log.h"
#include "Particle/SpriteSheet.h"
#include "Asset/AssetFileExtensions.h"

namespace GameUtils
{
	std::string levelToMoveTo;

	void SetActiveCameraTarget(Actor* newTarget)
	{
		activeCamera->targetActor = newTarget;
	}

	void CameraShake(float shake)
	{
		activeCamera->shakeLevel = shake;
	}

	SpriteSheet* SpawnSpriteSheet(std::string textureFilename, XMVECTOR position, bool loop, int numRows, int numColumns)
	{
		auto spriteSheet = SpriteSheet::system.Add("SpriteSheet", nullptr, SpriteSheet(), false);

		spriteSheet->SetLocalPosition(position);
		spriteSheet->textureData.filename = textureFilename;
		spriteSheet->loopAnimation = loop;
		spriteSheet->numSheetRows = numRows;
		spriteSheet->numSheetColumns = numColumns;

		spriteSheet->Create();

		return spriteSheet;
	}

	void PlayAudioOneShot(const std::string audioFilename)
	{
		AudioSystem::PlayAudio(audioFilename);
	}

	void SaveGameWorldState()
	{
		//The deal with save files here is the the .vmap file is the original state of the world seen in-editor
		//and the .sav version of it is based on in-game player saves. So if a .sav version exists, that is loaded
		//instead of the .vmap file, during gameplay.

		auto firstOf = World::worldFilename.find_first_of(".");
		std::string str = World::worldFilename.substr(0, firstOf);
		std::string file = str += AssetFileExtensions::gameSave;

		World::worldFilename = file;
		FileSystem::SerialiseAllSystems();
	}

	void LoadWorld(std::string worldName)
	{
		std::string path = "WorldMaps/" + worldName;

		if (GameInstance::useGameSaves)
		{
			path = "GameSaves/" + worldName;
		}

		GameUtils::SaveGameInstanceData();

		FileSystem::LoadWorld(worldName);
	}

	void SaveGameInstanceData()
	{
		if (GameInstance::useGameSaves)
		{
			Properties instanceProps = GameInstance::GetInstanceSaveData();
			Serialiser s(gameInstanceSaveFile, OpenMode::Out);
			s.Serialise(instanceProps);
		}
	}

	void LoadGameInstanceData()
	{
		if (GameInstance::useGameSaves)
		{
			Properties instanceProps = GameInstance::GetInstanceSaveData();
			Deserialiser d(gameInstanceSaveFile, OpenMode::In);
			d.Deserialise(instanceProps);
		}
	}

	void LoadWorldAndMoveToEntranceTrigger()
	{
		if (levelToMoveTo.empty())
		{
			return;
		}

		LoadWorld(levelToMoveTo);

		GameInstance::previousMapMovedFrom = levelToMoveTo;

		Input::blockInput = false;
	}

	void DisablePlayer()
	{
		auto player = Player::system.GetFirstActor();
		player->SetActive(false);
	}

	void SetActiveCamera(CameraComponent* camera)
	{
		activeCamera = camera;
	}
}
