#include "vpch.h"
#include "SpatialComponent.h"

void SpatialComponent::AddChild(SpatialComponent* component)
{
	assert(component != this);
	assert(component);
	component->parent = this;
	children.emplace_back(component);
}

void SpatialComponent::RemoveChild(SpatialComponent* component)
{
	for (int i = 0; i < children.size(); i++)
	{
		if (children[i] == component)
		{
			children.erase(children.begin() + i);
			return;
		}
	}

	throw new std::exception("child not found");
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

Properties SpatialComponent::GetProps()
{
	auto props = __super::GetProps();
	props.title = "SpatialComponent";
	props.Add(" Pos", &transform.position);
	props.Add(" Rot", &transform.rotation);
	props.Add(" Scale", &transform.scale);
	return props;
}

XMFLOAT3 SpatialComponent::GetLocalPosition()
{
	return transform.position;
}

XMVECTOR SpatialComponent::GetWorldPositionV()
{
	return GetWorldMatrix().r[3];
}

XMVECTOR SpatialComponent::GetLocalPositionV()
{
	XMVECTOR position = XMLoadFloat3(&transform.position);
	return position;
}

void SpatialComponent::SetLocalPosition(float x, float y, float z)
{
	transform.position = XMFLOAT3(x, y, z);
}

void SpatialComponent::SetLocalPosition(XMFLOAT3 newPosition)
{
	transform.position = newPosition;
}

void SpatialComponent::SetLocalPosition(XMVECTOR newPosition)
{
	XMStoreFloat3(&transform.position, newPosition);
}

void SpatialComponent::SetWorldPosition(XMVECTOR position)
{
	XMVECTOR relativePosition = position;

	if (parent)
	{
		XMVECTOR parentPosition = parent->GetLocalPositionV();
		relativePosition = XMVectorSubtract(relativePosition, parentPosition);
	}

	SetLocalPosition(relativePosition);
}

void SpatialComponent::AddLocalPosition(XMVECTOR offset)
{
	SetLocalPosition(GetLocalPositionV() + offset);
}

void SpatialComponent::AddWorldPosition(XMVECTOR offset)
{
	SetWorldPosition(GetWorldPositionV() + offset);
}

XMFLOAT3 SpatialComponent::GetLocalScale()
{
	return transform.scale;
}

XMVECTOR SpatialComponent::GetLocalScaleV()
{
	XMVECTOR scale = XMLoadFloat3(&transform.scale);
	return scale;
}

void SpatialComponent::SetLocalScale(float uniformScale)
{
	transform.scale = XMFLOAT3(uniformScale, uniformScale, uniformScale);
}

void SpatialComponent::SetLocalScale(float x, float y, float z)
{
	transform.scale = XMFLOAT3(x, y, z);
}

void SpatialComponent::SetLocalScale(XMFLOAT3 newScale)
{
	transform.scale = newScale;
}

void SpatialComponent::SetLocalScale(XMVECTOR newScale)
{
	XMStoreFloat3(&transform.scale, newScale);
}

void SpatialComponent::SetWorldScale(float uniformScale)
{
	SetWorldScale(XMVectorSet(uniformScale, uniformScale, uniformScale, 0.f));
}

void SpatialComponent::SetWorldScale(XMVECTOR scale)
{
	XMVECTOR relativeScale = scale;

	if (parent)
	{
		XMVECTOR parentScale = parent->GetLocalScaleV();
		relativeScale = XMVectorMultiply(relativeScale, parentScale);
	}

	SetLocalScale(relativeScale);
}

XMVECTOR SpatialComponent::GetLocalRotationV()
{
	XMVECTOR rotation = XMLoadFloat4(&transform.rotation);
	return rotation;
}

void SpatialComponent::SetLocalRotation(float x, float y, float z, float w)
{
	transform.rotation = XMFLOAT4(x, y, z, w);
}

XMFLOAT4 SpatialComponent::GetLocalRotation()
{
	return transform.rotation;
}

void SpatialComponent::SetLocalRotation(XMFLOAT4 newRotation)
{
	transform.rotation = newRotation;
}

void SpatialComponent::SetLocalRotation(XMVECTOR newRotation)
{
	XMStoreFloat4(&transform.rotation, newRotation);
}

XMFLOAT3 SpatialComponent::GetForwardVector()
{
	XMFLOAT3 forward;
	XMStoreFloat3(&forward, XMVector3Normalize(GetWorldMatrix().r[2]));
	return forward;
}

XMVECTOR SpatialComponent::GetForwardVectorV()
{
	return XMVector3Normalize(GetWorldMatrix().r[2]);
}

XMFLOAT3 SpatialComponent::GetRightVector()
{
	XMFLOAT3 right;
	XMStoreFloat3(&right, XMVector3Normalize(GetWorldMatrix().r[0]));
	return right;
}

XMVECTOR SpatialComponent::GetRightVectorV()
{
	return XMVector3Normalize(GetWorldMatrix().r[0]);
}

void SpatialComponent::SetWorldRotation(XMVECTOR newRotation)
{
	XMVECTOR relativeRotation = newRotation;

	if (parent)
	{
		//Looks like relative rotations are inversed with quaternions. ParentQuat(-1) * newRot;
		XMVECTOR parentRot = XMQuaternionInverse(parent->GetLocalRotationV());
		relativeRotation = XMQuaternionMultiply(parentRot, newRotation);
	}

	SetLocalRotation(relativeRotation);
}

void SpatialComponent::AddLocalRotation(XMVECTOR vector, float angle)
{
	auto newRotation = 
		XMQuaternionMultiply(GetLocalRotationV(),
			DirectX::XMQuaternionRotationAxis(vector, XMConvertToRadians(angle)));
	SetLocalRotation(newRotation);
}

XMFLOAT3 SpatialComponent::GetUpVector()
{
	XMFLOAT3 up;
	XMStoreFloat3(&up, XMVector3Normalize(GetWorldMatrix().r[1]));
	return up;
}

XMVECTOR SpatialComponent::GetUpVectorV()
{
	return XMVector3Normalize(GetWorldMatrix().r[1]);
}
