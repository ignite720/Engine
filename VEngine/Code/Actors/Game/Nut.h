#pragma once

#include "GridActor.h"

//Metallic nut, not the squirrel nut.
class Nut : public GridActor
{
public:
	ACTOR_SYSTEM(Nut);

	void Create() override;

	void OnLinkRotateLeft() override;
	void OnLinkRotateRight() override;

private:
	void CheckIfConnectedToBolt(const XMVECTOR moveDirection);
};
