#pragma once

#include "../PlayerUnit.h"
#include "Actors/ActorSystem.h"

class AttackUnit : public PlayerUnit
{
public:
	ACTOR_SYSTEM(AttackUnit);

	virtual Properties GetProps() override;
};
