#include "vpch.h"
#include "GameInstance.h"

std::string GameInstance::startingMap = "jailbreak.vmap";
std::string GameInstance::previousMapMovedFrom = startingMap;

//Used when continuing from game save files
std::string GameInstance::mapToLoadOnContinue;

bool GameInstance::useGameSaves = false;

//Global properties

Properties GameInstance::GetGlobalProps()
{
	Properties props("GameInstance");
	return props;
}
