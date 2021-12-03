#pragma once
#include "../Actor.h"
#include "../ActorSystem.h"
#include <string>

struct BoxTriggerComponent;
struct InteractWidget;

struct EntranceTrigger : Actor
{
	ACTOR_SYSTEM(EntranceTrigger)

	BoxTriggerComponent* trigger = nullptr;

	InteractWidget* interactWidget = nullptr;

	std::string levelToMoveTo;
	std::string openText;

	bool isEntraceActive = true;

	EntranceTrigger();
	virtual void Start() override;
	virtual void Tick(float deltaTime) override;
	virtual Properties GetProps() override;
};
