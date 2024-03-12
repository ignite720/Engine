#pragma once

#include "Actor.h"
#include "ActorSystem.h"

class CameraComponent;

class CameraActor : public Actor
{
public:
	ACTOR_SYSTEM(CameraActor);

	CameraActor();
	Properties GetProps() override;

private:
	CameraComponent* camera = nullptr;
};
