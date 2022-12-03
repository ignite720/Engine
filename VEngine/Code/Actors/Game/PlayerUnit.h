#pragma once

#include "../Actor.h"
#include "../ActorSystem.h"

struct MeshComponent;
struct PlayerActionBarWidget;
struct GuardWidget;
struct GridNode;

//Base class for all player controllable units.
//Should never be instantiated, no need for ACTOR_SYSTEM.
class PlayerUnit : public Actor
{
public:
	virtual void Tick(float deltaTime) override;

	void ControllerInput(float deltaTime);

	void CheckNextMoveNode(XMVECTOR previousPos);
	GridNode* GetCurrentNode();

protected:
	void ExpendActionPoints(int num);
	void PreviewMovementNodesDuringBattle();

	bool CheckIfMovementAndRotationStopped();
	void MovementInput(float deltaTime);
	void RotationInput(float deltaTime);

public:
	XMVECTOR nextPos;
	XMVECTOR nextRot;

	PlayerActionBarWidget* actionBarWidget = nullptr;
	GuardWidget* guardWidget = nullptr;

	MeshComponent* mesh = nullptr;

	int attackPoints = 1;
	int actionPoints = 10;
	int healthPoints = 3;

	float moveSpeed = 5.f;
	float rotSpeed = 3.5f;

	int xIndex = -1;
	int yIndex = -1;

	bool ableToGuard = false;
	bool guarding = false;
};
