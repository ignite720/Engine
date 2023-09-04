#include "vpch.h"
#include "WaterWheel.h"
#include "WaterVolume.h"

void WaterWheel::Create()
{
	__super::Create();
	SetMeshFilename("water_wheel.vmesh");
}

void WaterWheel::Start()
{
	__super::Start();
	waterVolume = World::GetActorByNameAndLogCast<WaterVolume>(waterVolumeName);
}

Properties WaterWheel::GetProps()
{
	auto props = __super::GetProps();
	props.title = GetTypeName();
	props.Add("Water Volume", &waterVolumeName);
	props.Add("Push Direction", &pushDirection);
	return props;
}

void WaterWheel::OnLinkRotate()
{
	__super::OnLinkRotate();

	if (waterVolume)
	{
		auto gridActors = waterVolume->GetAllGridActorsWithinVolume();
		for (auto gridActor : gridActors)
		{
			if (gridActor == this)
			{
				continue;
			}

			auto pushDirectionV = XMLoadFloat3(&pushDirection);
			auto nextPos = pushDirectionV + gridActor->GetPositionV();
			gridActor->SetNextPos(nextPos);
		}
	}
}
