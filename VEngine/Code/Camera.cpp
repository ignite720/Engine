#include "Camera.h"
#include "Input.h"
#include "WorldEditor.h"
#include "Actors/Actor.h"
#include "Editor/Editor.h"
#include "Math.h"

CameraComponent editorCamera(XMFLOAT3(0.f, 2.f, -5.f), true);
CameraComponent* activeCamera;

CameraComponent::CameraComponent(XMFLOAT3 startPos, bool isEditorCamera)
{
	focusPoint = XMVectorSet(0.f, 0.f, 0.f, 1.f);

	transform.position = startPos;
	UpdateTransform();

	editorCamera = isEditorCamera;
}

XMMATRIX CameraComponent::GetViewMatrix()
{
	XMVECTOR right = transform.world.r[0];
	XMVECTOR up = transform.world.r[1];
	XMVECTOR forward = transform.world.r[2];
	XMVECTOR position = transform.world.r[3];

	forward = XMVector3Normalize(forward);
	up = XMVector3Normalize(XMVector3Cross(forward, right));
	right = XMVector3Cross(up, forward);

	float x = XMVectorGetX(XMVector3Dot(position, right));
	float y = XMVectorGetX(XMVector3Dot(position, up));
	float z = XMVectorGetX(XMVector3Dot(position, forward));

	XMMATRIX view;

	view.r[0].m128_f32[0] = right.m128_f32[0];
	view.r[1].m128_f32[0] = right.m128_f32[1];
	view.r[2].m128_f32[0] = right.m128_f32[2];
	view.r[3].m128_f32[0] = -x;

	view.r[0].m128_f32[1] = up.m128_f32[0];
	view.r[1].m128_f32[1] = up.m128_f32[1];
	view.r[2].m128_f32[1] = up.m128_f32[2];
	view.r[3].m128_f32[1] = -y;

	view.r[0].m128_f32[2] = forward.m128_f32[0];
	view.r[1].m128_f32[2] = forward.m128_f32[1];
	view.r[2].m128_f32[2] = forward.m128_f32[2];
	view.r[3].m128_f32[2] = -z;

	view.r[0].m128_f32[3] = 0.0f;
	view.r[1].m128_f32[3] = 0.0f;
	view.r[2].m128_f32[3] = 0.0f;
	view.r[3].m128_f32[3] = 1.0f;

	return view;
}

void CameraComponent::Pitch(float angle)
{
	XMVECTOR& right = transform.world.r[0];
	XMVECTOR& up = transform.world.r[1];
	XMVECTOR& forward = transform.world.r[2];

	XMMATRIX r = XMMatrixRotationAxis(right, angle);
	up = XMVector3TransformNormal(up, r);
	forward = XMVector3TransformNormal(forward, r);
}

void CameraComponent::RotateY(float angle)
{
	XMVECTOR& right = transform.world.r[0];
	XMVECTOR& up = transform.world.r[1];
	XMVECTOR& forward = transform.world.r[2];

	XMMATRIX r = XMMatrixRotationY(angle);
	up = XMVector3TransformNormal(up, r);
	right = XMVector3TransformNormal(right, r);
	forward = XMVector3TransformNormal(forward, r);
}

void CameraComponent::MouseMove(int x, int y)
{
	static XMINT2 lastMousePos;

	if (Input::GetAsyncKey(Keys::RightMouse))
	{
		float dx = XMConvertToRadians(0.25f * (float)(x - lastMousePos.x));
		float dy = XMConvertToRadians(0.25f * (float)(y - lastMousePos.y));

		Pitch(dy);
		RotateY(dx);
	}

	lastMousePos.x = x;
	lastMousePos.y = y;
}

void CameraComponent::Move(float d, XMVECTOR axis)
{
	XMVECTOR s = XMVectorReplicate(d);
	XMVECTOR& position = transform.world.r[3];
	position = XMVectorMultiplyAdd(s, axis, position);
}

void CameraComponent::ZoomTo(Actor* actor)
{
	XMVECTOR& position = transform.world.r[3];
	XMVECTOR& forward = transform.world.r[2];

	//Trace the camera down the line its pointing towards the actor
	XMFLOAT3 actorPos = actor->GetPosition();
	XMVECTOR actorPosVec = XMLoadFloat3(&actorPos);
	XMVECTOR zoomPos = actorPosVec - (forward * 5.f);
	position = zoomPos;
}

void CameraComponent::Tick(double deltaTime)
{
	XMVECTOR right = transform.world.r[0];
	XMVECTOR up = transform.world.r[1];
	XMVECTOR forward = transform.world.r[2];

	if (editorCamera)
	{
		MouseMove(editor->viewportMouseX, editor->viewportMouseY);

		//WASD MOVEMENT
		//if (!gConsole.bConsoleActive)
		{
			if (Input::GetAsyncKey(Keys::RightMouse))
			{
				const float moveSpeed = 7.5f * deltaTime;

				if (Input::GetAsyncKey(Keys::W))
				{
					Move(moveSpeed, forward);
				}
				if (Input::GetAsyncKey(Keys::S))
				{
					Move(-moveSpeed, forward);
				}
				if (Input::GetAsyncKey(Keys::D))
				{
					Move(moveSpeed, right);
				}
				if (Input::GetAsyncKey(Keys::A))
				{
					Move(-moveSpeed, right);
				}
				if (Input::GetAsyncKey(Keys::Q))
				{
					Move(-moveSpeed, up);
				}
				if (Input::GetAsyncKey(Keys::E))
				{
					Move(moveSpeed, up);
				}
			}

			//Zoom onto selected actor
			if (Input::GetKeyUp(Keys::F))
			{
				if (worldEditor.pickedActor)
				{
					ZoomTo(worldEditor.pickedActor);
				}
			}

			//MOUSE WHEEL ZOOM
			const float zoomSpeed = 55.f * deltaTime;

			if (Input::mouseWheelUp)
			{
				Move(zoomSpeed, forward);
			}
			else if (Input::mouseWheelDown)
			{
				Move(-zoomSpeed, forward);
			}
		}
	}
}

void CameraComponent::Create()
{
}

Properties CameraComponent::GetProps()
{
	return Properties();
}

//void Camera::FrustumCullTest(ActorSystem& system)
//{
//	for (int i = 0; i < system.actors.size(); i++)
//	{
//		XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);

//		XMMATRIX world = system.actors[i]->GetTransformationMatrix();
//		XMMATRIX invWorld = XMMatrixInverse(&XMMatrixDeterminant(world), world);

//		XMMATRIX viewToLocal = XMMatrixMultiply(invView, invWorld);

//		BoundingFrustum frustum, localSpaceFrustum;
//		BoundingFrustum::CreateFromMatrix(frustum, proj);
//		frustum.Transform(localSpaceFrustum, viewToLocal);

//		system.boundingBox.Center = system.actors[i]->GetPositionFloat3();
//		system.boundingBox.Extents = system.actors[i]->GetScale();

//		if (localSpaceFrustum.Contains(system.boundingBox) == DirectX::DISJOINT)
//		{
//			system.actors[i]->bRender = false;
//		}
//		else
//		{
//			system.actors[i]->bRender = true;
//		}
//	}
//}
