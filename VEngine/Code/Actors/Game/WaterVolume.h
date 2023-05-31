#pragma once

#include "../Actor.h"
#include "../ActorSystem.h"

class BoxTriggerComponent;
class MeshComponent;

//A plane denoting the water's surface and a BoxTrigger encapsulating the body of water.
class WaterVolume : public Actor
{
public:
	ACTOR_SYSTEM(WaterVolume);

	WaterVolume();
	void Create() override;
	void Tick(float deltaTime) override;
	Properties GetProps() override;

	bool Contains(XMVECTOR point);

private:
	MeshComponent* waterSurface = nullptr;
	BoxTriggerComponent* waterVolumeTrigger = nullptr;

	float yPointToRaiseTo = 0.f;
};