#include "Widget.h"
#include <WICTextureLoader.h>
#include "Input.h"
#include "Render/Renderer.h"
#include "Editor/Editor.h"
#include "Render/SpriteSystem.h"

void Widget::Tick(float deltaTime)
{
}

void Widget::Start()
{
}

void Widget::AddToViewport()
{
	uiSystem.AddWidget(this);
}

void Widget::RemoveFromViewport()
{
	uiSystem.RemoveWidget(this);
}

void Widget::GetScreenSpaceCoords(int& sx, int& sy)
{
	//What you need to do here it take the actor's position after it's been multiplied 
	//by the MVP matrix on the CPU side of things, divide it by the W component 
	//and multiply it out by the viewport.
	//REF:http://www.windows-tech.info/5/a80747e145dd9062.php

	const float f1 = pos.m128_f32[0] / pos.m128_f32[3];
	const float f2 = pos.m128_f32[1] / pos.m128_f32[3];

	sx = ((f1 * 0.5f) + 0.5) * renderer.viewport.Width;
	sy = ((f2 * -0.5f) + 0.5) * renderer.viewport.Height;
}

void Widget::Text(const std::wstring& text, D2D1_RECT_F layout, TextAlign align,
	D2D1_COLOR_F color, float opacity)
{
	DWRITE_TEXT_ALIGNMENT endAlignment{};

	switch (align)
	{
	case TextAlign::Center: 
		endAlignment = DWRITE_TEXT_ALIGNMENT_CENTER;
		break;

	case TextAlign::Justified:
		endAlignment = DWRITE_TEXT_ALIGNMENT_JUSTIFIED;
		break;

	case TextAlign::Leading:
		endAlignment = DWRITE_TEXT_ALIGNMENT_LEADING;
		break;

	case TextAlign::Trailing:
		endAlignment = DWRITE_TEXT_ALIGNMENT_TRAILING;
		break;
	}

	uiSystem.textFormat->SetTextAlignment(endAlignment);

	uiSystem.brushText->SetColor(color);
	uiSystem.brushText->SetOpacity(opacity);

	uiSystem.d2dRenderTarget->DrawText(text.c_str(), text.size(),
		uiSystem.textFormat, layout, uiSystem.brushText);
}

bool Widget::Button(const std::wstring& text, D2D1_RECT_F layout, float lineWidth,
	TextAlign textAlign, D2D1_COLOR_F textColor, float textOpacity)
{
	uiSystem.d2dRenderTarget->DrawRectangle(layout, uiSystem.brushShapes, lineWidth);
	Text(text, layout, textAlign, textColor, textOpacity);

	if (Input::GetMouseLeftUp())
	{
		if (editor->viewportMouseX > layout.left && editor->viewportMouseX < layout.right)
		{
			if (editor->viewportMouseY > layout.top && editor->viewportMouseY < layout.bottom)
			{
				return true;
			}
		}
	}

	return false;
}

void Widget::Image(const std::string& filename, D2D1_RECT_F layout)
{
	Sprite sprite = {};
	sprite.textureFilename = filename;
	sprite.dstRect = { (long)layout.left, (long)layout.top, (long)layout.right, (long)layout.bottom };
	sprite.srcRect = { 0, 0, (long)layout.right, (long)layout.bottom };

	spriteSystem.CreateScreenSprite(sprite);
}

void Widget::Image(const std::string& filename, int x, int y, int w, int h)
{
	Sprite sprite = {};
	sprite.textureFilename = filename;
	sprite.dstRect = { x, y, x + w, y + h };
	sprite.srcRect = { 0, 0, w, h };

	spriteSystem.CreateScreenSprite(sprite);
}

void Widget::Rect(D2D1_RECT_F layout)
{
	uiSystem.d2dRenderTarget->DrawRectangle(layout, uiSystem.brushShapes);
}

void Widget::FillRect(D2D1_RECT_F layout, D2D1_COLOR_F color, float opacity)
{
	uiSystem.brushShapesAlpha->SetColor(color);
	uiSystem.brushShapesAlpha->SetOpacity(opacity);

	uiSystem.d2dRenderTarget->FillRectangle(layout, uiSystem.brushShapesAlpha);
}

D2D1_RECT_F Widget::AlignLayout(float w, float h, Align align)
{
	float vw = renderer.GetViewportWidth();
	float vh = renderer.GetViewportHeight();

	switch (align)
	{
	case Align::Center: 
		vw /= 2.f;
		vh /= 2.f;
		break;

	case Align::Right:
		vw *= 0.75;
		vh /= 2.f;
		break;

	case Align::Left:
		vw *= 0.25f;
		vh /= 2.f;
		break;

	case Align::Top:
		vw /= 2.f;
		vh *= 0.25f;
		break;

	case Align::Bottom:
		vw /= 2.f;
		vh *= 0.75f;
		break;

	case Align::TopLeft:
		vw *= 0.25f;
		vh *= 0.25f;
		break;

	case Align::TopRight:
		vw *= 0.75f;
		vh *= 0.25f;
		break;

	case Align::BottomLeft:
		vw *= 0.25f;
		vh *= 0.75f;
		break;

	case Align::BottomRight:
		vw *= 0.75f;
		vh *= 0.75f;
		break;
	}

	D2D1_RECT_F rect = { vw - w, vh - h, vw + w, vh + h };

	if (rect.left < 0.f) rect.left = 0.f;
	if (rect.top < 0.f) rect.top = 0.f;
	if (rect.right > renderer.GetViewportWidth()) rect.right = vw;
	if (rect.bottom > renderer.GetViewportHeight()) rect.bottom = vh;

	return rect;
}

D2D1_RECT_F Widget::PercentAlignLayout(float left, float top, float right, float bottom)
{
	float vw = renderer.GetViewportWidth();
	float vh = renderer.GetViewportHeight();

	float endLeft = vw * left;
	float endTop = vh * top;
	float endRight = vw * right;
	float endBottom = vh * bottom;
	D2D1_RECT_F rect = { endLeft, endTop, endRight, endBottom };

	if (rect.left < 0.f) rect.left = 0.f;
	if (rect.top < 0.f) rect.top = 0.f;
	if (rect.right > renderer.GetViewportWidth()) rect.right = vw;
	if (rect.bottom > renderer.GetViewportHeight()) rect.bottom = vh;

	return rect;
}

D2D1_RECT_F Widget::CenterLayoutOnScreenSpaceCoords(float w, float h, float sx, float sy)
{
	D2D1_RECT_F rect = {sx - w, sy - h, sx + w, sy + h};

	float vw = renderer.GetViewportWidth();
	float vh = renderer.GetViewportHeight();

	if (rect.left < 0.f) rect.left = 0.f;
	if (rect.top < 0.f) rect.top = 0.f;
	if (rect.right > vw) rect.right = vw;
	if (rect.bottom > vh) rect.bottom = vh;

	return rect;
}
