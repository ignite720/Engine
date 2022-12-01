#include "vpch.h"
#include "GridMapPicker.h"
#include "Input.h"
#include "Actors/Game/GridActor.h"
#include "Components/CameraComponent.h"
#include "UI/Game/GridMapPickerSelectionInfoWidget.h"
#include "Physics/Raycast.h"

GridMapPicker::GridMapPicker()
{
	SetEmptyRootComponent();
	
	//Camera setup
	camera = CreateComponent(CameraComponent(), "Camera");
	camera->targetActor = this;
	camera->SetPosition(1.75f, 1.75f, -2.75f);
	rootComponent->AddChild(camera);

	//Selection Widget setup
	gridMapPickerSelectionInfoWidget = CreateWidget<GridMapPickerSelectionInfoWidget>();
	gridMapPickerSelectionInfoWidget->AddToViewport();
}

void GridMapPicker::Start()
{
}

void GridMapPicker::Tick(float deltaTime)
{
	if (Input::GetMouseLeftUp())
	{
		Ray ray(this);
		if (RaycastFromScreen(ray))
		{
			auto gridActor = dynamic_cast<GridActor*>(ray.hitActor);
			if (gridActor)
			{
				SetPosition(gridActor->GetPosition());

				gridMapPickerSelectionInfoWidget->selectedGridActor = gridActor;
			}
		}
	}
}

Properties GridMapPicker::GetProps()
{
	return __super::GetProps();
}
