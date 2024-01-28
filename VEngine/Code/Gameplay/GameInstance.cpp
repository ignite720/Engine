#include "vpch.h"
#include "GameInstance.h"

std::string GameInstance::startingMap = "church_balcony_room.vmap";
std::string GameInstance::previousMapMovedFrom = startingMap;

bool GameInstance::useGameSaves = false;

//GAME SPECIFIC VARS
static std::string heldPlayerItem;

//Global properties
//...

//CHURCH
static bool churchSwordLock = false;
static bool churchStatueRotateLock = false;

//TOWN
static std::string townTimeOfDay = "Night";

//Used when continuing from game save files
static std::string mapToLoadOnContinue;
//...

Properties GameInstance::GetGlobalProps()
{
	Properties props("GameInstance");
	props.Add("ChurchSwordLock", &churchSwordLock);
	props.Add("ChurchStatueRotateLock", &churchStatueRotateLock);
	props.Add("MapToLoadOnContinue", &mapToLoadOnContinue);
	props.Add("TownTimeOfDay", &townTimeOfDay);
	return props;
}

std::string GameInstance::GetHeldPlayerItem()
{
	return heldPlayerItem;
}

void GameInstance::SetHeldPlayerItem(std::string_view heldItem)
{
	heldPlayerItem = heldItem;
}

void GameInstance::ClearHeldPlayerItem()
{
	heldPlayerItem.clear();
}

std::string GameInstance::GetContinueMapName()
{
	return mapToLoadOnContinue;
}

void GameInstance::SetContinueMapName(std::string_view name)
{
	mapToLoadOnContinue = name;
}
