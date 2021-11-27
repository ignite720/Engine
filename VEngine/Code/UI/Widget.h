#pragma once
#include <string>
#include <d2d1_1.h>
#include <unordered_map>
#include <DirectXMath.h>

using namespace DirectX;

//I can never get the D2D1_RECT_F things right
//left = The x-coordinate of the upper-left corner of the rectangle
//top = The y-coordinate of the upper-left corner of the rectangle.
//right = The x-coordinate of the lower-right corner of the rectangle
//bottom = The y-coordinate of the lower-right corner of the rectangle.

//Base widget class for in-game UI.
struct Widget
{
	virtual void Tick(float deltaTime);
	virtual void Start();
	void AddToViewport();
	void RemoveFromViewport();
	void GetScreenSpaceCoords(int& sx, int& sy);

	void Text(const std::wstring& text, D2D1_RECT_F layout);
	bool Button(const std::wstring& text, D2D1_RECT_F layout, float lineWidth = 1.0f);
	void Image(const std::string& filename, int x, int y, int w, int h);
	void Rect(D2D1_RECT_F layout);

	//Rect layout alignment functions
	static D2D1_RECT_F AlignLayoutCenter(float w, float h);

	XMVECTOR pos;
	std::wstring displayText;

	bool render = true;
};
