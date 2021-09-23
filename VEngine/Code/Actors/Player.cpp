#include "Player.h"
#include "Components/MeshComponent.h"
#include "Camera.h"
#include "Input.h"
#include "VMath.h"

#include "Physics/Raycast.h"
#include "Editor/Editor.h"

Player::Player()
{
	mesh = MeshComponent::system.Add(this, MeshComponent("cube.fbx", "wall.png"));
	rootComponent = mesh;

	camera = CameraComponent::system.Add(this, CameraComponent(XMFLOAT3(3.f, 3.f, -7.f), false));
	rootComponent->AddChild(camera);
}

void Player::Start()
{
	nextPos = GetPositionVector();

	nextRot = GetRotationVector();
}

void Player::Tick(double deltaTime)
{
	const float moveSpeed = 7.5f;
	SetPosition(VMath::VectorConstantLerp(GetPositionVector(), nextPos, deltaTime, moveSpeed));

	//if (VMath::VecEqual(GetPositionVector(), nextPos))
	if (XMVector4Equal(GetPositionVector(), nextPos))
	{
		if (Input::GetAsyncKey(Keys::W))
		{
			Ray ray;
			ray.actorsToIgnore.push_back(this);
			if (!Raycast(ray, GetPositionVector(), GetForwardVectorV(), 1.f))
			{
				editor->Log("hit");
				nextPos = GetForwardVectorV() + GetPositionVector();
			}
		}		
		if (Input::GetAsyncKey(Keys::S))
		{
			nextPos = -GetForwardVectorV() + GetPositionVector();
		}
		if (Input::GetAsyncKey(Keys::A))
		{
			nextPos = -GetRightVectorV() + GetPositionVector();
		}
		if (Input::GetAsyncKey(Keys::D))
		{
			nextPos = GetRightVectorV() + GetPositionVector();
		}
	}

	const float rotSpeed = 5.f;
	//currentRot = XMQuaternionSlerp(currentRot, nextRot, deltaTime * rotSpeed);
	SetRotation(VMath::QuatConstantLerp(GetRotationVector(), nextRot, deltaTime, rotSpeed));

	if (XMQuaternionEqual(GetRotationVector(), nextRot))
	{
		if (Input::GetKeyUp(Keys::Right))
		{
			float angle = XMConvertToRadians(90.f);
			nextRot = XMQuaternionMultiply(nextRot, DirectX::XMQuaternionRotationAxis(VMath::XMVectorUp(), angle));
		}		
		if (Input::GetKeyUp(Keys::Left))
		{
			float angle = XMConvertToRadians(-90.f);
			nextRot = XMQuaternionMultiply(nextRot, DirectX::XMQuaternionRotationAxis(VMath::XMVectorUp(), angle));
		}
	}
}

Properties Player::GetProps()
{
    return Actor::GetProps();
}
