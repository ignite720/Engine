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
	enum class TextAlign
	{
		Center,
		Justified,
		Leading,
		Trailing
	};

	enum class Align
	{
		Center,
		Right,
		Left,
		Bottom,
		Top,
		TopLeft,
		TopRight,
		BottomLeft,
		BottomRight,
	};

	virtual void Tick(float deltaTime);
	virtual void Start();

	void AddToViewport();
	void RemoveFromViewport();
	void GetScreenSpaceCoords(int& sx, int& sy);

	void Text(const std::wstring& text, D2D1_RECT_F layout, TextAlign align = TextAlign::Center,
		D2D1_COLOR_F color = { 0.f, 0.f, 0.f, 1.f }, float opacity = 1.0f);

	bool Button(const std::wstring& text, D2D1_RECT_F layout, float lineWidth = 1.0f);

	void Image(const std::string& filename, D2D1_RECT_F layout);
	void Image(const std::string& filename, int x, int y, int w, int h);

	void Rect(D2D1_RECT_F layout);
	void FillRect(D2D1_RECT_F layout, D2D1_COLOR_F color = {0.5f, 0.5f, 0.5f, 1.f}, float opacity = 1.0f);

	D2D1_RECT_F AlignLayout(float w, float h, Align align);
	D2D1_RECT_F CenterLayoutOnScreenSpaceCoords(float w, float h, float sx, float sy);

	//pos is the widget's position in world space to be mapped to screen space.
	XMVECTOR pos;

	bool render = true;
};
