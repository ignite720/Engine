#pragma once
#include "../Actor.h"
#include "../ActorSystem.h"

struct BoxTriggerComponent;

//Actor to trigger quick player dialogue when player overlaps with trigger.
struct PlayerDialogueTrigger : Actor
{
	BoxTriggerComponent* trigger = nullptr;

	//Text to show on overlap
	std::wstring playerThoughtText;

private:
	bool alreadyActivated = false;

public:
	PlayerDialogueTrigger();
	virtual void Start() override;
	virtual void Tick(float deltaTime) override;
	virtual Properties GetProps() override;
};
