#include "SpatialComponent.h"

void SpatialComponent::AddChild(SpatialComponent* component)
{
	assert(component);
	component->parent = this;
	children.push_back(component);
}

XMMATRIX SpatialComponent::GetWorldMatrix()
{
	XMMATRIX parentWorld = XMMatrixIdentity();

	if (parent)
	{
		parentWorld = parent->GetWorldMatrix();
	}

	UpdateTransform(parentWorld);

	return transform.world;
}

void SpatialComponent::UpdateTransform(XMMATRIX parentWorld)
{
	XMMATRIX world = transform.GetAffine() * parentWorld;

	for (SpatialComponent* child : children)
	{
		child->UpdateTransform(world);
	}

	transform.world = world;
}

XMFLOAT3 SpatialComponent::GetPosition()
{
	return transform.position;
}

void SpatialComponent::SetPosition(XMFLOAT3 newPosition)
{
	transform.position = newPosition;
}

void SpatialComponent::SetPosition(XMVECTOR newPosition)
{
	XMStoreFloat3(&transform.position, newPosition);
}

XMFLOAT3 SpatialComponent::GetScale()
{
	return XMFLOAT3();
}

void SpatialComponent::SetScale(XMFLOAT3 newScale)
{
	transform.scale = newScale;
}

void SpatialComponent::SetScale(XMVECTOR newScale)
{
	XMStoreFloat3(&transform.scale, newScale);
}

XMFLOAT4 SpatialComponent::GetRotation()
{
	return transform.rotation;
}

void SpatialComponent::SetRotation(XMFLOAT4 newRotation)
{
	transform.rotation = newRotation;
}

void SpatialComponent::SetRotation(XMVECTOR newRotation)
{
	XMStoreFloat4(&transform.rotation, newRotation);
}

XMFLOAT3 SpatialComponent::GetForwardVector()
{
	XMFLOAT3 forward;
	XMStoreFloat3(&forward, transform.world.r[2]);
	return forward;
}

XMFLOAT3 SpatialComponent::GetRightVector()
{
	XMFLOAT3 right;
	XMStoreFloat3(&right, transform.world.r[0]);
	return right;
}

XMFLOAT3 SpatialComponent::GetUpVector()
{
	XMFLOAT3 up;
	XMStoreFloat3(&up, transform.world.r[1]);
	return up;
}
