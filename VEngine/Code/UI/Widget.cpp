#include "Widget.h"
#include <WICTextureLoader.h>
#include "UI/UISystem.h"
#include "Input.h"
#include "Render/Renderer.h"
#include "Editor/Editor.h"
#include "Render/SpriteSystem.h"

void Widget::Tick(float deltaTime)
{
	//Just for testing
	MapToScreenSpace();
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

void Widget::MapToScreenSpace()
{
	//What you need to do here it take the actor's position after it's been multiplied 
	//by the MVP matrix on the CPU side of things, divide it by the W component and multiply it out by the viewport.

	//REF:http://www.windows-tech.info/5/a80747e145dd9062.php

	float f1 = pos.m128_f32[0] / pos.m128_f32[3];
	float f2 = pos.m128_f32[1] / pos.m128_f32[3];

	int sx = ((f1 * 0.5f) + 0.5) * renderer.viewport.Width;
	int sy = ((f2 * -0.5f) + 0.5) * renderer.viewport.Height;

	Text(displayText, { (float)sx, (float)sy, (float)sx + 150, (float)sy + 150 });
}

void Widget::Text(const std::wstring& text, D2D1_RECT_F layout)
{
	uiSystem.d2dRenderTarget->DrawText(text.c_str(), text.size(),
		uiSystem.textFormat, layout, uiSystem.brushText);
}

//Make sure the buttons layout isn't backwards(bottom and top less than left and right)
//else the mouse check won't work.
bool Widget::Button(const std::wstring& text, D2D1_RECT_F layout, float lineWidth)
{
	uiSystem.d2dRenderTarget->DrawRectangle(layout, uiSystem.brushShapes, lineWidth);
	Text(text, layout);

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

void Widget::Image(const std::string& filename, int x, int y, int w, int h)
{
	Sprite sprite = {};
	sprite.textureFilename = filename;
	sprite.dstRect = { x, y, x + w, y + h };
	sprite.srcRect = { 0, 0, w, h };

	spriteSystem.CreateScreenSprite(sprite);
}
