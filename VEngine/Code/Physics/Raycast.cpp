#include "vpch.h"
#include "Raycast.h"
#include <limits>
#include "Core/Camera.h"
#include "Render/Renderer.h"
#include "Editor/Editor.h"
#include "Core/VMath.h"
#include "Actors/Actor.h"
#include "Components/MeshComponent.h"
#include "Core/World.h"

bool IsIgnoredActor(Actor* actor, HitResult& hitResult)
{
	for (auto actorToIgnore : hitResult.actorsToIgnore)
	{
		if (actor == actorToIgnore)
		{
			return true;
		}
	}

	return false;
}

bool IsIgnoredSpatialComponent(SpatialComponent* component, HitResult& hitResult)
{
	for (auto componentToIgnore : hitResult.componentsToIgnore)
	{
		if (component == componentToIgnore)
		{
			return true;
		}
	}

	return false;
}

bool Raycast(HitResult& hitResult, XMVECTOR origin, XMVECTOR direction, float range, bool fromScreen)
{
	hitResult.origin = origin;
	hitResult.direction = direction;

	//Calculate raycast from camera coords into world
	if (fromScreen)
	{
		XMVECTOR invViewDet = XMMatrixDeterminant(activeCamera->GetViewMatrix());
		XMMATRIX invView = XMMatrixInverse(&invViewDet, activeCamera->GetViewMatrix());
		XMVECTOR invDet = XMMatrixDeterminant(XMMatrixIdentity());
		XMMATRIX invModel = XMMatrixInverse(&invDet, XMMatrixIdentity());
		XMMATRIX toLocal = XMMatrixMultiply(invView, invModel);

		hitResult.origin = XMVector3TransformCoord(hitResult.origin, toLocal);
		hitResult.direction = XMVector3TransformNormal(hitResult.direction, toLocal);
		hitResult.direction = XMVector3Normalize(hitResult.direction);
	}

	std::vector<float> distances;

	bool bRayHit = false;
	hitResult.hitActors.clear();
	hitResult.hitComponents.clear();

	auto actorsInWorld = World::GetAllActorsInWorld();
	for (auto actor : actorsInWorld)
	{
		if (!actor->IsActive())
		{
			continue;
		}

		if (IsIgnoredActor(actor, hitResult))
		{
			continue;
		}

		//Iterate over actor's mesh components
		for (auto mesh : actor->GetComponentsOfType<MeshComponent>())
		{
			if (!mesh->IsActive())
			{
				continue;
			}

			//Collision layer checks
			if (mesh->layer == CollisionLayers::None ||
				mesh->layer == hitResult.ignoreLayer)
			{
				continue;
			}

			if (IsIgnoredSpatialComponent(mesh, hitResult))
			{
				continue;
			}

			BoundingOrientedBox boundingBox = VMath::GetBoundingBoxInWorld(mesh);

			if (boundingBox.Intersects(hitResult.origin, hitResult.direction, hitResult.hitDistance))
			{
				distances.emplace_back(hitResult.hitDistance);
				hitResult.hitComponents.emplace_back(mesh);
				bRayHit = true;
			}
		}
	}

	float nearestDistance = std::numeric_limits<float>::max();

	if (!bRayHit)
	{
		return false;
	}

	if (RaycastTriangleIntersect(hitResult))
	{
		for (int i = 0; i < distances.size(); i++)
		{
			//Skip hit if more than range or less than 0
			if (distances[i] > range || distances[i] < 0.f)
			{
				continue;
			}

			if (distances[i] < nearestDistance)
			{
				nearestDistance = distances[i];
			}
		}
	}

	if (nearestDistance > range)
	{
		return false;
	}

	if (hitResult.hitActor)
	{
		XMVECTOR hitPos = origin + (direction * hitResult.hitDistance);
		XMStoreFloat3(&hitResult.hitPos, hitPos);
		return true;
	}

	return false;
}

bool Raycast(HitResult& hitResult, XMVECTOR origin, XMVECTOR end)
{
	XMVECTOR direction = XMVector3Normalize(end - origin);
	float range = XMVector3Length(end - origin).m128_f32[0] + 0.1f;
	return Raycast(hitResult, origin, direction, range, false);
}

//@Todo: there's a performance problem with this function. It just takes too long with large meshes if they're hit.
bool RaycastTriangleIntersect(HitResult& hitResult)
{
	std::vector<HitResult> hitResults;

	for (auto component : hitResult.hitComponents)
	{
		XMMATRIX model = component->GetWorldMatrix();

		auto mesh = dynamic_cast<MeshComponent*>(component);
		if(mesh)
		{
			//This is for dealing with DestructibleMesh's meshproxy nulls
			if (mesh->meshDataProxy.vertices == nullptr)
			{
				continue;
			}

			for (int i = 0; i < mesh->meshDataProxy.vertices->size() / 3; i++)
			{
				const int index0 = i * 3;
				const int index1 = i * 3 + 1;
				const int index2 = i * 3 + 2;

				XMVECTOR v0 = XMLoadFloat3(&mesh->meshDataProxy.vertices->at(index0).pos);
				v0 = XMVector3TransformCoord(v0, model);

				XMVECTOR v1 = XMLoadFloat3(&mesh->meshDataProxy.vertices->at(index1).pos);
				v1 = XMVector3TransformCoord(v1, model);

				XMVECTOR v2 = XMLoadFloat3(&mesh->meshDataProxy.vertices->at(index2).pos);
				v2 = XMVector3TransformCoord(v2, model);

				HitResult tempHitResult = {};
				tempHitResult = hitResult;

				if (DirectX::TriangleTests::Intersects(hitResult.origin, hitResult.direction, v0, v1, v2, tempHitResult.hitDistance))
				{
					//Get normal for triangle
					XMVECTOR normal = XMVectorZero();
					normal += XMLoadFloat3(&mesh->meshDataProxy.vertices->at(index0).normal);
					normal += XMLoadFloat3(&mesh->meshDataProxy.vertices->at(index1).normal);
					normal += XMLoadFloat3(&mesh->meshDataProxy.vertices->at(index2).normal);

					normal = XMVector3Normalize(normal);
					XMStoreFloat3(&tempHitResult.normal, normal);

					//Get hit UV
					XMVECTOR hitPosition = hitResult.origin + (hitResult.direction * tempHitResult.hitDistance);
					float hitU, hitV;
					VMath::TriangleXYZToUV(mesh->meshDataProxy.vertices->at(index0),
						mesh->meshDataProxy.vertices->at(index1),
						mesh->meshDataProxy.vertices->at(index2), hitPosition, hitU, hitV);
					tempHitResult.uv = XMFLOAT2(hitU, hitV);

					//Set hit component and actor
					tempHitResult.hitComponent = mesh;
					tempHitResult.hitActor = World::GetActorByUID(mesh->GetOwnerUID());

					hitResults.emplace_back(tempHitResult);
				}
			}
		}
	}

	//Get all hit actors
	std::vector<Actor*> hitActors;
	for (auto& hitResult : hitResults)
	{
		hitActors.emplace_back(hitResult.hitActor);
	}

	//Set nearest hit actor
	float lowestDistance = std::numeric_limits<float>::max();
	int rayIndex = -1;
	for (int i = 0; i < hitResults.size(); i++)
	{
		if (hitResults[i].hitDistance < lowestDistance)
		{
			lowestDistance = hitResults[i].hitDistance;
			rayIndex = i;
		}
	}

	if (rayIndex > -1)
	{
		hitResult = hitResults[rayIndex];
		hitResult.hitActors = hitActors;
		return true;
	}

	return false;
}

bool RaycastFromScreen(HitResult& hitResult)
{
	const int sx = editor->viewportMouseX;
	const int sy = editor->viewportMouseY;

	CameraComponent* camera = activeCamera;

	XMMATRIX proj = camera->GetProjectionMatrix();

	const float vx = (2.f * sx / Renderer::GetViewportWidth() - 1.0f) / proj.r[0].m128_f32[0];
	const float vy = (-2.f * sy / Renderer::GetViewportHeight() + 1.0f) / proj.r[1].m128_f32[1];

	hitResult.origin = XMVectorSet(0.f, 0.f, 0.f, 1.f);
	hitResult.direction = XMVectorSet(vx, vy, 1.f, 0.f);

	constexpr float range = std::numeric_limits<float>::max();

	return Raycast(hitResult, hitResult.origin, hitResult.direction, range, true);
}

void DrawDebugLine(XMVECTOR start, XMVECTOR end)
{
	Line line;
	XMStoreFloat3(&line.p1, start);
	XMStoreFloat3(&line.p2, end);

	Renderer::AddDebugLine(line);
}

bool OrientedBoxCast(HitResult& hitResult, XMVECTOR origin, XMVECTOR end, XMFLOAT2 extents, bool drawDebug)
{
	XMVECTOR orientationV = VMath::LookAtRotation(end, origin);

	XMFLOAT4 orientation{};
	XMStoreFloat4(&orientation, orientationV);

	XMVECTOR centerV = VMath::CalcMidPoint(origin, end);
	XMFLOAT3 center{};
	XMStoreFloat3(&center, centerV);

	XMFLOAT3 extentsFloat3 = { extents.x, extents.y, 0.f };
	extentsFloat3.z = XMVector3Length(end - origin).m128_f32[0] / 2.f;

	DirectX::BoundingOrientedBox boundingOrientedBox(center, extentsFloat3, orientation);

	if (drawDebug)
	{
		Renderer::ClearBounds();
		Renderer::AddDebugDrawOrientedBox(boundingOrientedBox);
	}

	for (auto actor : World::GetAllActorsInWorld())
	{
		if (IsIgnoredActor(actor, hitResult))
		{
			continue;
		}

		for (auto mesh : actor->GetComponentsOfType<MeshComponent>())
		{
			auto meshBoundsInWorld = VMath::GetBoundingBoxInWorld(mesh);
			if (boundingOrientedBox.Intersects(meshBoundsInWorld))
			{
				hitResult.hitComponents.emplace_back(mesh);
				hitResult.hitActors.emplace_back(actor);
			}
		}
	}

	return hitResult.hitActors.size();
}

//Note: doesn't set HitResult::hitActor.
bool SimpleBoxCast(XMVECTOR center, XMFLOAT3 extents, HitResult& hit, bool drawDebug)
{
	XMFLOAT3 centerFloat3;
	XMStoreFloat3(&centerFloat3, center);
	DirectX::BoundingBox boundingBox(centerFloat3, extents);

	if (drawDebug)
	{
		DirectX::BoundingOrientedBox orientedBox;
		DirectX::BoundingOrientedBox::CreateFromBoundingBox(orientedBox, boundingBox);
		Renderer::ClearBounds();
		Renderer::AddDebugDrawOrientedBox(orientedBox);
	}

	for (auto actor : World::GetAllActorsInWorld())
	{
		if (IsIgnoredActor(actor, hit))
		{
			continue;
		}

		for (auto mesh : actor->GetComponentsOfType<MeshComponent>())
		{
			BoundingOrientedBox meshWorldBounds = VMath::GetBoundingBoxInWorld(mesh);

			if (boundingBox.Intersects(meshWorldBounds))
			{
				hit.hitComponents.emplace_back(mesh);
				hit.hitActors.emplace_back(actor);
			}
		}
	}

	return hit.hitActors.size();
}

void HitResult::AddActorsToIgnore(std::vector<Actor*>& actors)
{
	actorsToIgnore.reserve(actorsToIgnore.size() + actors.size());

	for (auto actor : actors)
	{
		actorsToIgnore.emplace_back(actor);
	}
}
