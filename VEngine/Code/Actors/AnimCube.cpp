#include "vpch.h"
#include "AnimCube.h"
#include "Components/SkeletalMeshComponent.h"

AnimCube::AnimCube()
{
	skeletalMesh = CreateComponent("Skeleton", SkeletalMeshComponent("spin_cubes.vmesh", "test.png"));
	rootComponent = skeletalMesh;
}

void AnimCube::PostCreate()
{
	skeletalMesh->LoadAnimation("spin_cubes@spin.vanim");
}

void AnimCube::Start()
{
	skeletalMesh->PlayAnimation("spin");
}

Properties AnimCube::GetProps()
{
	return __super::GetProps();
}
