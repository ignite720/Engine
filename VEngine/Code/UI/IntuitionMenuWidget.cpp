#include "IntuitionMenuWidget.h"
#include "Render/Renderer.h"
#include "Gameplay/GameUtils.h"
#include "Gameplay/Intuition.h"
#include "Actors/Game/Player.h"
#include "VString.h"

void IntuitionMenuWidget::Tick(float deltaTime)
{
	D2D1_RECT_F rect = CenterLayoutOnScreenSpaceCoords(500.f, 350.f,
		renderer.GetViewportWidth() / 2.f, renderer.GetViewportHeight() / 2.f);

	Rect(rect);

	//Set text rect to begining of layoutrect, then increment in forloop
	D2D1_RECT_F textRect = rect;
	textRect.bottom = rect.top;

	for (auto& intuitionPair : GameUtils::GetPlayer()->intuitions)
	{
		Intuition* intuition = intuitionPair.second;

		Text(stows(intuition->name), textRect);
		textRect.top += 30.f;
		textRect.bottom += 30.f;

		Text(stows(intuition->description), textRect);
		textRect.top += 30.f;
		textRect.bottom += 30.f;
	}
}
