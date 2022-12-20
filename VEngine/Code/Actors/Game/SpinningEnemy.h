#pragma once

#include "Enemy.h"

//A spinning everlasting gobstopper enemy.
class SpinningEnemy : public Enemy
{
public:
	ACTOR_SYSTEM(SpinningEnemy);

	void Create() override;
	void Start() override;
	void Tick(float deltaTime) override;
	Properties GetProps() override;

private:
	XMVECTOR nextRot = XMVectorSet(0.f, 0.f, 0.f, 1.f);
	float spinTimer = 0.f;
};
