#include "vpch.h"
#include "Player.h"
#include "Input.h"
#include "VMath.h"
#include "Actors/Game/Enemy.h"
#include "Actors/Game/LevelInstance.h"
#include "Components/MeshComponent.h"
#include "Components/CameraComponent.h"
#include "Gameplay/GameUtils.h"
#include "Physics/Raycast.h"
#include "UI/Game/ComboBarWidget.h"
#include "UI/UISystem.h"

Player::Player()
{
	mesh = CreateComponent(MeshComponent("cube.fbx", "test.png"), "Mesh");
	rootComponent = mesh;

	camera = CreateComponent(CameraComponent(), "Camera");
	rootComponent->AddChild(camera);
}

void Player::Create()
{
	camera->SetPosition(2.f, 1.5f, -4.f);
}

void Player::Start()
{
	__super::Start();

	comboBarWidget = UISystem::CreateWidget<ComboBarWidget>();
	comboBarWidget->AddToViewport();

	auto cameraFocusPoint = GetPositionV() + GetForwardVectorV() * 3.f;
	camera->SetWorldRotation(VMath::LookAtRotation(cameraFocusPoint, camera->GetWorldPositionV()));

	nextPos = GetPositionV();
	nextRot = GetRotationV();
}

void Player::End()
{
	previousHitTransparentActors.clear();
}

void Player::Tick(float deltaTime)
{
	__super::Tick(deltaTime);

	MakeOccludingMeshBetweenCameraAndPlayerTransparent();

	Shoot();
	BladeSwipe();

	MovementInput();
	RotationInput();

	SetPosition(VMath::VectorConstantLerp(GetPositionV(), nextPos, deltaTime, movementSpeed));
	SetRotation(VMath::QuatConstantLerp(GetRotationV(), nextRot, deltaTime, rotationSpeed));
}

Properties Player::GetProps()
{
    auto props = __super::GetProps();
	props.title = "Player";
	props.Add("All Range Mode", &allRangeModeActive);
	return props;
}

//Note: Default blend state needs to already be set for the mesh.
void Player::MakeOccludingMeshBetweenCameraAndPlayerTransparent()
{
	auto SetActorAlpha = [](Actor* actor, float alpha) {
		auto mesh = actor->GetFirstComponentOfTypeAllowNull<MeshComponent>();
		if (mesh && mesh->transparentOcclude)
		{
			auto ambientColour = mesh->GetAmbientColour();
			ambientColour.w = alpha;
			mesh->SetAmbientColour(ambientColour);
		}
	};

	const float transparentValue = 0.35f;
	const float solidValue = 1.f;

	HitResult hitResult(this);
	if (OrientedBoxCast(hitResult, camera->GetWorldPositionV(), GetPositionV(), XMFLOAT2(0.5f, 0.5f), true))
	{
		std::vector<Actor*> ableActors;

		for (auto actor : previousHitTransparentActors)
		{
			SetActorAlpha(actor, solidValue);
		}

		for (auto actor : hitResult.hitActors)
		{
			if (actor->CanBeTransparentlyOccluded())
			{
				ableActors.push_back(actor);
				SetActorAlpha(actor, transparentValue);
			}
		}

		previousHitTransparentActors = ableActors;
	}
	else
	{
		for(auto actor : previousHitTransparentActors)
		{
			SetActorAlpha(actor, solidValue);
		}

		previousHitTransparentActors.clear();
	}
}

void Player::MovementInput()
{
	if (!CheckMovementAndRotationHaveStopped()) return;

	auto previousPos = nextPos;

	if (Input::GetKeyHeld(Keys::W))
	{
		nextPos = GetPositionV() + GetForwardVectorV();
		if (CheckForObstacle()) nextPos = previousPos;
	}
	else if (Input::GetKeyHeld(Keys::S))
	{
		nextPos = GetPositionV() - GetForwardVectorV();
		if (CheckForObstacle()) nextPos = previousPos;
	}
	else if (Input::GetKeyHeld(Keys::A))
	{
		nextPos = GetPositionV() - GetRightVectorV();
		if (CheckForObstacle()) nextPos = previousPos;
	}
	else if (Input::GetKeyHeld(Keys::D))
	{
		nextPos = GetPositionV() + GetRightVectorV();
		if (CheckForObstacle()) nextPos = previousPos;
	}
	else if (Input::GetKeyHeld(Keys::E))
	{
		nextPos = GetPositionV() + GetUpVectorV();
		if (CheckForObstacle()) nextPos = previousPos;
	}
	else if (Input::GetKeyHeld(Keys::Q))
	{
		nextPos = GetPositionV() - GetUpVectorV();
		if (CheckForObstacle()) nextPos = previousPos;
	}

	if (!CheckPlayerWithinLevelBounds())
	{
		nextPos = previousPos;
	}
}

void Player::RotationInput()
{
	if (Input::GetKeyHeld(Keys::Shift) && Input::GetKeyUp(Keys::Left))
	{
		nextRot = XMQuaternionMultiply(GetRotationV(), DirectX::XMQuaternionRotationAxis(GetForwardVectorV(), XMConvertToRadians(90.f)));
	}
	else if (Input::GetKeyHeld(Keys::Shift) && Input::GetKeyUp(Keys::Right))
	{
		nextRot = XMQuaternionMultiply(GetRotationV(), DirectX::XMQuaternionRotationAxis(GetForwardVectorV(), XMConvertToRadians(-90.f)));
	}
	else if (Input::GetKeyHeld(Keys::Shift) && Input::GetKeyUp(Keys::Up))
	{
		nextRot = XMQuaternionMultiply(GetRotationV(), DirectX::XMQuaternionRotationAxis(GetRightVectorV(), XMConvertToRadians(-90.f)));
	}
	else if (Input::GetKeyHeld(Keys::Shift) && Input::GetKeyUp(Keys::Down))
	{
		nextRot = XMQuaternionMultiply(GetRotationV(), DirectX::XMQuaternionRotationAxis(GetRightVectorV(), XMConvertToRadians(90.f)));
	}
	else if (Input::GetKeyUp(Keys::Right))
	{
		nextRot = XMQuaternionMultiply(GetRotationV(), DirectX::XMQuaternionRotationAxis(GetUpVectorV(), XMConvertToRadians(90.f)));
	}
	else if (Input::GetKeyUp(Keys::Left))
	{
		nextRot = XMQuaternionMultiply(GetRotationV(), DirectX::XMQuaternionRotationAxis(GetUpVectorV(), XMConvertToRadians(-90.f)));
	}
}

bool Player::CheckMovementAndRotationHaveStopped()
{
	return XMVector4Equal(GetPositionV(), nextPos) && XMQuaternionEqual(GetRotationV(), nextRot);
}

bool Player::CheckPlayerWithinLevelBounds()
{
	return LevelInstance::system.GetFirstActor()->CheckIfPointInsideLevelSize(nextPos);
}

bool Player::CheckForObstacle()
{
	HitResult hitResult(this);
	return Raycast(hitResult, GetPositionV(), nextPos);
}

void Player::Shoot()
{
	if (!CheckMovementAndRotationHaveStopped()) return;

	if (Input::GetKeyDown(Keys::Up))
	{
		HitResult hitResult(this);
		if (Raycast(hitResult, GetPositionV(), GetForwardVectorV(), 1000.f))
		{
			auto enemy = dynamic_cast<Enemy*>(hitResult.hitActor);
			if (enemy)
			{
				comboBarWidget->IncreaseScoreAndCombo();

				auto mesh = dynamic_cast<MeshComponent*>(hitResult.hitComponent);
				if (mesh)
				{
					GameUtils::SpawnSpriteSheet("Sprites/explosion.png", mesh->GetWorldPositionV(), false, 4, 4);
					mesh->Remove();
				}

				if (enemy->CheckIfAllTaggedMeshesAreDestroyed())
				{
					enemy->Destroy();
				}
			}
		}
	}
}

void Player::BladeSwipe()
{
	if (!CheckMovementAndRotationHaveStopped()) return;

	if (Input::GetKeyDown(Keys::Down))
	{
		//Line up 5 origins alongside player's right axis
		std::vector<XMVECTOR> rayOrigins;
		rayOrigins.push_back(GetPositionV() - GetRightVectorV() * 2.f);
		rayOrigins.push_back(GetPositionV() - GetRightVectorV());
		rayOrigins.push_back(GetPositionV());
		rayOrigins.push_back(GetPositionV() + GetRightVectorV());
		rayOrigins.push_back(GetPositionV() + GetRightVectorV() * 2.f);

		//Can't destroy components/actors in an inner Raycast loop. Keep them and destroy later down.
		std::vector<MeshComponent*> hitMeshComponents;
		std::set<Enemy*> hitEnemies;

		for (auto& rayOrigin : rayOrigins)
		{
			HitResult hitResult(this);
			if (Raycast(hitResult, rayOrigin, GetForwardVectorV(), 1000.f))
			{
				auto enemy = dynamic_cast<Enemy*>(hitResult.hitActor);
				if (enemy)
				{
					hitEnemies.emplace(enemy);

					comboBarWidget->IncreaseScoreAndCombo();
					
					auto mesh = dynamic_cast<MeshComponent*>(hitResult.hitComponent);
					if (mesh)
					{
						hitMeshComponents.push_back(mesh);
					}
				}
			}
		}

		for (auto hitMesh : hitMeshComponents)
		{
			GameUtils::SpawnSpriteSheet("Sprites/blade_slash.png", hitMesh->GetWorldPositionV(), false, 3, 5);
			hitMesh->Remove();
		}

		for (auto hitEnemy : hitEnemies)
		{
			if (hitEnemy->CheckIfAllTaggedMeshesAreDestroyed())
			{
				hitEnemy->Destroy();
			}
		}
	}
}
