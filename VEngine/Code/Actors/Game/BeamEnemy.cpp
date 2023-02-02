#include "vpch.h"
#include "BeamEnemy.h"
#include "Core/VMath.h"
#include "Actors/Game/Player.h"
#include "Components/MeshComponent.h"
#include "Particle/Polyboard.h"
#include "Physics/Raycast.h"

void BeamEnemy::Create()
{
	auto mesh = CreateComponent(MeshComponent("turret.vmesh", "test.png"), "Mesh");
	rootComponent->AddChild(mesh);

	beam = CreateComponent(Polyboard(), "Beam");
	beam->GenerateVertices();
	rootComponent->AddChild(beam);
}

void BeamEnemy::Tick(float deltaTime)
{
	auto pos = GetPositionV();

	auto mesh = GetComponent<MeshComponent>("Mesh");
	if (mesh)
	{
		XMStoreFloat3(&beam->startPoint, pos);
		XMStoreFloat3(&beam->endPoint, pos + GetForwardVectorV() * beamDistance);
	}

	HitResult result(this);
	if (Raycast(result, pos, pos + GetForwardVectorV() * beamDistance))
	{
		auto player = dynamic_cast<Player*>(result.hitActor);
		if (player)
		{
			player->InflictDamage(1.f);
		}
	}

	//Here for stationary BeamEnemies
	if (!VMath::Float3IsZero(rotateDirection))
	{
		AddRotation(XMLoadFloat3(&rotateDirection), rotateSpeed * deltaTime);
	}
}

Properties BeamEnemy::GetProps()
{
	auto props = __super::GetProps();
	props.title = "BeamEnemy";
	props.Add("Rotate Speed", &rotateSpeed);
	props.Add("Rotate Direction", &rotateDirection);
	return props;
}
