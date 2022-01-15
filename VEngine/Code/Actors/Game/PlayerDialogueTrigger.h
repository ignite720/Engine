#pragma once
#include "../Actor.h"
#include "../ActorSystem.h"

struct BoxTriggerComponent;

//Actor to trigger quick player dialogue when player overlaps with trigger.
struct PlayerDialogueTrigger : Actor
{
	ACTOR_SYSTEM(PlayerDialogueTrigger);

	BoxTriggerComponent* trigger = nullptr;

	//Text to show on overlap
	std::wstring playerThoughtText;

	//Memory that can optionally activate player dialogue
	std::string memoryToActivate;

private:
	bool alreadyActivated = false;

public:
	PlayerDialogueTrigger();
	virtual void Start() override;
	virtual void Tick(float deltaTime) override;
	virtual Properties GetProps() override;
};
