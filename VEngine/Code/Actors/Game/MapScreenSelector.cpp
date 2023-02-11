#include "vpch.h"
#include "MapScreenSelector.h"
#include "Components/CameraComponent.h"
#include "Core/Input.h"
#include "Core/VMath.h"

MapScreenSelector::MapScreenSelector()
{
	SetEmptyRootComponent();
}

void MapScreenSelector::Create()
{
	camera = CreateComponent<CameraComponent>("Camera");
	rootComponent->AddChild(camera);

	camera->SetLocalPosition(0.f, 3.f, -3.f);
	camera->SetWorldRotation(VMath::LookAtRotation(GetPositionV(), camera->GetWorldPositionV()));
}

void MapScreenSelector::Start()
{
	camera->SetAsActiveCamera();
}

void MapScreenSelector::Tick(float deltaTime)
{
	MovementInput(deltaTime);
	RotationInput(deltaTime);
}

Properties MapScreenSelector::GetProps()
{
	auto props = __super::GetProps();
	props.title = GetTypeName();
	return props;
}

void MapScreenSelector::MovementInput(float deltaTime)
{
	const float moveSpeed = 7.5f * deltaTime;

	if (Input::GetKeyHeld(Keys::W))
	{
		AddPositionV(GetForwardVectorV() * moveSpeed);
	}
	else if (Input::GetKeyHeld(Keys::S))
	{
		AddPositionV(GetForwardVectorV() * -moveSpeed);
	}

	if (Input::GetKeyHeld(Keys::A))
	{
		AddPositionV(GetRightVectorV() * -moveSpeed);
	}
	else if (Input::GetKeyHeld(Keys::D))
	{
		AddPositionV(GetRightVectorV() * moveSpeed);
	}
}

void MapScreenSelector::RotationInput(float deltaTime)
{
	const float angle = 100.f * deltaTime;

	if (Input::GetKeyHeld(Keys::Right))
	{
		AddRotation(GetUpVectorV(), angle);
	}
	else if (Input::GetKeyHeld(Keys::Left))
	{
		AddRotation(GetUpVectorV(), -angle);
	}
}
