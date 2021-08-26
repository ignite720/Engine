#pragma once
#include "Components/SpatialComponent.h"
#include "Components/ComponentSystem.h"
#include <DirectXMath.h>

using namespace DirectX;

struct Actor;

struct CameraComponent : SpatialComponent
{
	COMPONENT_SYSTEM(CameraComponent)

	XMMATRIX proj;
	XMVECTOR focusPoint;
	bool editorCamera = false;

	CameraComponent(XMFLOAT3 startPos, bool isEditorCamera);
	XMMATRIX GetViewMatrix();
	void Pitch(float angle);
	void RotateY(float angle);
	void MouseMove(int x, int y);
	void Move(float d, XMVECTOR axis);
	void ZoomTo(Actor* actor);

	virtual void Tick(double deltaTime) override;
	virtual void Create() override;
	virtual Properties GetProps() override;
	//void FrustumCullTest(ActorSystem& system);
};

extern CameraComponent editorCamera;
extern CameraComponent* activeCamera;
