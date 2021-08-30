#include "Actor.h"
#include "Components/SpatialComponent.h"

Actor::Actor()
{
}

XMMATRIX Actor::GetWorldMatrix()
{
	XMMATRIX parentWorld = XMMatrixIdentity();

	if (parent)
	{
		parentWorld = parent->GetWorldMatrix();
	}

	UpdateTransform(parentWorld);

	return rootComponent->transform.world;
}

void Actor::UpdateTransform(XMMATRIX parentWorld)
{
	XMMATRIX world = GetTransformMatrix() * parentWorld;

	for (Actor* child : children)
	{
		child->UpdateTransform(world);
	}

	rootComponent->transform.world = world;
}

XMMATRIX Actor::GetTransformMatrix()
{
	XMVECTOR rotationOffset = XMVectorSet(0.f, 0.f, 0.f, 1.f);

	if (parent)
	{
		rotationOffset = parent->rootComponent->transform.world.r[3];
	}

	return rootComponent->transform.GetAffineRotationOrigin(rotationOffset);
}

XMFLOAT3 Actor::GetPosition()
{
	return XMFLOAT3(rootComponent->transform.position);
}

XMFLOAT3 Actor::GetScale()
{
	return XMFLOAT3(rootComponent->transform.scale);
}

XMFLOAT4 Actor::GetRotation()
{
	return XMFLOAT4(rootComponent->transform.rotation);
}

void Actor::SetPosition(XMVECTOR position)
{
	XMStoreFloat3(&rootComponent->transform.position, position);
}

void Actor::SetPosition(XMFLOAT3 position)
{
	rootComponent->transform.position = position;
}

void Actor::SetScale(XMVECTOR scale)
{
	XMStoreFloat3(&rootComponent->transform.scale, scale);
}

void Actor::SetRotation(XMVECTOR rotation)
{
	XMStoreFloat4(&rootComponent->transform.rotation, rotation);
}

void Actor::SetTransform(Transform transform)
{
	rootComponent->transform = transform;
}

Transform Actor::GetTransform()
{
	return rootComponent->transform;
}

XMFLOAT3 Actor::GetForwardVector()
{
	XMFLOAT3 forward;
	XMStoreFloat3(&forward, rootComponent->transform.world.r[2]);
	return forward;
}

XMFLOAT3 Actor::GetRightVector()
{
	XMFLOAT3 right;
	XMStoreFloat3(&right, rootComponent->transform.world.r[0]);
	return right;
}

XMFLOAT3 Actor::GetUpVector()
{
	XMFLOAT3 up;
	XMStoreFloat3(&up, rootComponent->transform.world.r[1]);
	return up;
}

Properties Actor::GetProps()
{
	Properties props("Actor");

	props.Add("Position", &rootComponent->transform.position);
	props.Add("Scale", &rootComponent->transform.scale);
	props.Add("Rotation", &rootComponent->transform.rotation);

	props.Add("Name", &name);

	//This was how to join maps. Keeping here for reference.
	/*for (Component* component : components)
	{
		Properties componentProps = component->GetProps();
		props.dataMap.insert(componentProps.dataMap.begin(), componentProps.dataMap.end());
		props.typeMap.insert(componentProps.typeMap.begin(), componentProps.typeMap.end());
	}*/

	return props;
}

void Actor::Tick(double deltaTime)
{
}

void Actor::Destroy()
{
}
