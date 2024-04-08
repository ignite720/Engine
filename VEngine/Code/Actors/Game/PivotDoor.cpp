#include "vpch.h"
#include "PivotDoor.h"
#include "Components/MeshComponent.h"
#include "Render/RastStates.h"

PivotDoor::PivotDoor()
{
	SetEmptyRootComponent();
	rootComponent->AddChild(mesh);
}

void PivotDoor::Create()
{
	__super::Create();

	mesh->SetLocalPosition(0.5f, 0.25f, 0.f);
	mesh->SetLocalScale(0.5f, 0.75f, 1.f);
	mesh->SetMeshFilename("plane.vmesh");
	mesh->SetRastState(RastStates::noBackCull);

	linkRotateAudio = "door.wav";
	rotateSpeed = 2.5f;

	canBeMovedInLink = false;

	canBeRotatedRollZAxis = false;
	canBeRotatedPitchXAxis = false;
	canBeRotatedYawYAxis = true;
}

Properties PivotDoor::GetProps()
{
	auto props = __super::GetProps();
	props.title = GetTypeName();
	return props;
}
