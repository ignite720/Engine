#include "vpch.h"
#include "SteamValve.h"
#include "Gameplay/GameUtils.h"
#include "Particle/ParticleEmitter.h"
#include "Steam.h"

void SteamValve::Create()
{
	__super::Create();

	ignoreRotationValidCheck = true;
	canBeMovedInLink = false;

	rotateSpeed = 2.f;

	SetMeshFilename("gear.vmesh");
}

void SteamValve::Start()
{
	__super::Start();

	steam = World::GetActorByNameAndLogCast<Steam>(steamName);
	EnableDisableSteam();
}

Properties SteamValve::GetProps()
{
	auto props = __super::GetProps();
	props.title = GetTypeName();
	props.Add("Valve On", &isValveOn);
	props.Add("Steam", &steamName);
	return props;
}

void SteamValve::OnLinkRotate()
{
	__super::OnLinkRotate();

	isValveOn = !isValveOn;

	EnableDisableSteam();
}

void SteamValve::EnableDisableSteam()
{
	if (steam)
	{
		if (isValveOn)
		{
			steam->Enable();
		}
		else
		{
			steam->Disable();
		}
	}
}
