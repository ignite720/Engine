#pragma once

#include <DirectXMath.h>

using namespace DirectX;

struct Ray
{
	XMVECTOR origin;
	XMVECTOR direction;

	int actorIndex = 0;
};

void DrawRayDebug(XMVECTOR rayOrigin, XMVECTOR rayDir, float distance, class ID3D11Buffer* debugBuffer, class DXUtil* dx);
bool Raycast(Ray& ray, int sx, int sy, class Camera* camera, class ActorSystem* actorSystem);
