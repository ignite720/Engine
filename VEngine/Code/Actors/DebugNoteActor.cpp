#include "vpch.h"
#include "DebugNoteActor.h"
#include "UI/DebugNoteWidget.h"
#include "UI/UISystem.h"

DebugNoteActor::DebugNoteActor()
{
	SetEmptyRootComponent();
}

void DebugNoteActor::Start()
{
	noteWidget = UISystem::CreateWidget<DebugNoteWidget>();
	noteWidget->noteText = noteText;
	noteWidget->AddToViewport();
}

void DebugNoteActor::Tick(float deltaTime)
{
	noteWidget->SetWorldPosition(GetHomogeneousPositionV());
}

Properties DebugNoteActor::GetProps()
{
	auto props = __super::GetProps();
	props.title = GetTypeName();
	props.Add("Note Text", &noteText);
	return props;
}
