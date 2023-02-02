#pragma once

#include <vector>
#include <DirectXMath.h>
#include "Physics/CollisionLayers.h"

class IActorSystem;
struct CameraComponent;
class Actor;
struct SpatialComponent;

using namespace DirectX;

struct HitResult
{
	XMVECTOR origin = XMVectorSet(0.f, 0.f, 0.f, 1.f);
	XMVECTOR direction = XMVectorSet(0.f, 0.f, 0.f, 0.f);
	
	//Position the ray has hit in world
	XMFLOAT3 hitPos = XMFLOAT3(0.f, 0.f, 0.f);
	XMFLOAT3 normal = XMFLOAT3(0.f, 0.f, 0.f);
	XMFLOAT2 uv = XMFLOAT2(0.f, 0.f);

	//List of actors to ignore when cast
	std::vector<Actor*> actorsToIgnore;

	std::vector<SpatialComponent*> componentsToIgnore;

	std::vector<Actor*> hitActors;
	Actor* hitActor = nullptr;

	std::vector<SpatialComponent*> hitComponents;
	SpatialComponent* hitComponent = nullptr;

	//Layer to ignore on raycast
	CollisionLayers ignoreLayer = CollisionLayers::None;

	//Cut the raycast off at this point
	float range = 0.f;

	//Output distance from ray origin and hit point
	float hitDistance = 0.f;

	bool bHit = false;

	HitResult() {}
	HitResult(Actor* actorToIgnore)
	{
		actorsToIgnore.emplace_back(actorToIgnore);
	}

	void AddActorsToIgnore(std::vector<Actor*>& actors);

	XMVECTOR GetHitPosV() { return XMLoadFloat3(&hitPos); }
};

bool Raycast(HitResult& hitResult, XMVECTOR origin, XMVECTOR direction, float range, bool fromScreen = false);
bool Raycast(HitResult& hitResult, XMVECTOR origin, XMVECTOR end);
bool RaycastTriangleIntersect(HitResult& hitResult);
bool RaycastFromScreen(HitResult& hitResult);
void DrawDebugLine(XMVECTOR start, XMVECTOR end);

bool OrientedBoxCast(HitResult& hitResult, XMVECTOR origin, XMVECTOR end, XMFLOAT2 extents, bool drawDebug);
bool SimpleBoxCast(XMFLOAT3 center, XMFLOAT3 extents, HitResult& hitResult);
