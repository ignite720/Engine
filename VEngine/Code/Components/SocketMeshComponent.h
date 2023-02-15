#pragma once

#include "Components/MeshComponent.h"
#include "Animation/AnimationValues.h"

class SkeletalMeshComponent;

//@Todo: need to figrue out how to handle deleting SocketMeshes that are linked to SkeletalMeshes.
//If they're on the same actor, it's fine. But if they're seperate from each other, socket won't delete.

//Mesh that attaches to a Skeleton Joint. Updates its own transform using the Joint its attached to.
class SocketMeshComponent : public MeshComponent
{
public:
	COMPONENT_SYSTEM(SocketMeshComponent);

	SocketMeshComponent() {}
	SocketMeshComponent(int jointIndex_, std::string meshFilename, std::string textureFilename);

	void Create() override;

	void LinkToSkeletalMeshComponent(SkeletalMeshComponent* skeletalMesh);
	void SetTransformFromLinkedSkeletonJoint();

private:
	SkeletalMeshComponent* linkedSkeletalMesh = nullptr;

	JointIndex jointIndex = INVALID_JOINT_INDEX;
};
