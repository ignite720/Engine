#pragma once

#include <vector>

class World
{
public:

	std::vector<class ActorSystem*> actorSystems;
	char name[64];
};

extern World currentWorld;

World* GetWorld();
