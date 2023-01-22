#include "vpch.h"
#include "DebugCamera.h"
#include "Components/MeshComponent.h"

DebugCamera::DebugCamera()
{
	mesh = new MeshComponent("camera.vmesh", "test.png");
	mesh->layer = CollisionLayers::Editor;
	mesh->Create();
}
