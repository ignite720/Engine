#pragma once
#include <DirectXMath.h>

using namespace DirectX;

struct GridNode
{
	GridNode()
	{
		xIndex = 0;
		yIndex = 0;
	}

	GridNode(int x, int y, int instancedMeshIndex_)
	{
		xIndex = x;
		yIndex = y;
		instancedMeshIndex = instancedMeshIndex_;

		worldPosition = XMFLOAT3((float)x, (float)0.f, (float)y);
	}

	bool Equals(int x, int y)
	{
		return (x == xIndex) && (y == yIndex);
	}

	bool Equals(GridNode* node)
	{
		return (node->xIndex == xIndex) && (node->yIndex == yIndex);
	}

	float GetFCost()
	{
		return gCost + hCost;
	}

	void ResetValues()
	{
		gCost = 0.f;
		hCost = 0.f;
		parentNode = nullptr;
		closed = false;
		preview = false;
		SetColour(normalColour);
	}

	//These functions also sets the nodes variables
	void Hide();
	void Show();

	//These ones only care about the visual display of the node
	void DisplayHide();
	void DisplayShow();

	void SetColour(XMFLOAT4 newColour);

	GridNode* parentNode = nullptr;
	XMFLOAT3 worldPosition;

	//COLOURS
	inline static XMFLOAT4 normalColour = XMFLOAT4(0.07f, 0.27f, 0.89f, 0.4f);
	inline static XMFLOAT4 previewColour = XMFLOAT4(0.89f, 0.07f, 0.07f, 0.4f);

	float gCost = 0.f; //Distance from start node
	float hCost = 0.f; //Distance to end node
	int xIndex = 0;
	int yIndex = 0;
	uint32_t instancedMeshIndex = 0;
	bool closed = false;
	bool active = true;
	bool preview = false; //If the node is to show preview movements, ignores lerp
};
