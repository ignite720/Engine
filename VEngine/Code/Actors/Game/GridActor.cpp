#include "GridActor.h"
#include "Components/MeshComponent.h"
#include "Components/WidgetComponent.h"

GridActor::GridActor()
{
	mesh = MeshComponent::system.Add(this);
	rootComponent = mesh;

	widget = WidgetComponent::system.Add(this);
}

Properties GridActor::GetProps()
{
	auto props = Actor::GetProps();
	props.title = "GridActor";
	props.Add("Destruct", &isDestructible);
	props.Add("Health", &health);
	return props;
}
