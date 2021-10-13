#pragma once
#include "Actor.h"
#include "ActorSystem.h"

struct GridNode;

//Units are all moveable actors that can be placed in-world
struct Unit : Actor
{
	ACTOR_SYSTEM(Unit)

	int xIndex;
	int yIndex;

	Unit();
	virtual Properties GetProps() override;
	void MoveTo(GridNode& node);
};
