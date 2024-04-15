#include "vpch.h"
#include "VerticalMoveGridActor.h"
#include "Core/VMath.h"

void VerticalMoveGridActor::Create()
{
	__super::Create();

	canBeRotatedRollZAxis = false;
	canBeRotatedPitchXAxis = false;
}

void VerticalMoveGridActor::OnLinkMoveForward()
{
	__super::OnLinkMoveForward();

	ResetNextPos();
	AddNextPosition(VMath::GlobalUpVector());
}

void VerticalMoveGridActor::OnLinkMoveBack()
{
	__super::OnLinkMoveBack();

	ResetNextPos();
	AddNextPosition(-VMath::GlobalUpVector());
}

void VerticalMoveGridActor::OnLinkMoveLeft()
{
	__super::OnLinkMoveLeft();

	ResetNextPos();
}

void VerticalMoveGridActor::OnLinkMoveRight()
{
	__super::OnLinkMoveRight();

	ResetNextPos();
}
