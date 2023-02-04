#include "vpch.h"
#include "MeshSplitActor.h"
#include "Components/MeshComponent.h"
#include "Core/Input.h"
#include "Core/MeshSplitter.h"

MeshSplitActor::MeshSplitActor()
{
	mesh = CreateComponent(MeshComponent("ico_sphere.vmesh", "test.png"), "Mesh");
	rootComponent = mesh;
}

void MeshSplitActor::Tick(float deltaTime)
{
	if (Input::GetKeyUp(Keys::Enter))
	{
		MeshSplitter::SplitMeshViaPlane(*mesh, mesh0Verts, mesh1Verts);

		mesh->Remove();

		{
			auto mesh0 = MeshComponent::system.Add("SplitMesh0", this, MeshComponent(), false);
			mesh0->SetRastState(RastStates::noBackCull);
			mesh0->meshDataProxy.vertices = &mesh0Verts;
			mesh0->SplitMeshCreate();
		}

		{		
			auto mesh1 = MeshComponent::system.Add("SplitMesh1", this, MeshComponent(), false);
			mesh1->SetRastState(RastStates::noBackCull);
			mesh1->meshDataProxy.vertices = &mesh1Verts;
			mesh1->SplitMeshCreate();
		}
	}
}