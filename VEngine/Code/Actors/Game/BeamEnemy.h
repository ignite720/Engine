#pragma once

#include "Enemy.h"

class Polyboard;

//Enemy that emits a beam, moving like the fireball poles from Mario Bros.
class BeamEnemy : public Enemy
{
public:
	ACTOR_SYSTEM(BeamEnemy);

	void Create() override;
	void Start() override;
	void Tick(float deltaTime) override;
	Properties GetProps() override;

private:
	Polyboard* beam = nullptr;

	XMVECTOR rotateDirection = XMVectorSet(0.f, 0.f, 0.f, 1.f);
	float rotateSpeed = 33.f;
	float beamDistance = 10.f;
};
