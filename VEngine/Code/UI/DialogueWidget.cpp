#include "DialogueWidget.h"
#include "VString.h"

void DialogueWidget::Draw(float deltaTime)
{
	int sx, sy;
	GetScreenSpaceCoords(sx, sy);

	sy -= 75.f;
	Layout layout = CenterLayoutOnScreenSpaceCoords(100.f, 100.f, sx, sy);

	D2D1_RECT_F imageRect = {
		layout.rect.left - 50.f, layout.rect.top - 25.f, layout.rect.right + 50.f, layout.rect.bottom + 75.f
	};

	Image("speech_bubble.png", imageRect);
	Text(dialogueText, layout);
}

void DialogueWidget::SetText(std::string text)
{
	dialogueText = VString::stows(text);
}
