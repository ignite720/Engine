#pragma once

#include <DirectXMath.h>

using namespace DirectX;

class Transform
{
public:
	Transform();
	XMMATRIX GetAffine();
	XMMATRIX GetAffineRotationOrigin(XMVECTOR rotOrigin);
	void Decompose(XMMATRIX m);

	XMFLOAT3 position;
	XMFLOAT3 scale;
	XMFLOAT3 euler;
	XMFLOAT4 quatRotation;
};
