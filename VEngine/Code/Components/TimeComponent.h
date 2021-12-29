#pragma once
#include "Component.h"
#include "ComponentSystem.h"

//Holds time of day game data for actors.
struct TimeComponent : Component
{	
	COMPONENT_SYSTEM(TimeComponent);

	int activeBeginHour = 0;
	int activeEndHour = 0;

	//Check if need the actor to be always active (usually for inanimate game objects)
	bool alwaysActive = true;

	TimeComponent();
	virtual Properties GetProps() override;
	bool CheckIfActiveAtCurrentTime();
};
