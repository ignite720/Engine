#include "vpch.h"
#include "MinecartTrack.h"
#include "Components/MeshComponent.h"

void MinecartTrack::Create()
{
	canBeMovedInLink = false;

	mesh->SetMeshFilename("node.vmesh");
	mesh->SetTexture("minecart_track.jpg");
}