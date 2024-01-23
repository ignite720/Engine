#pragma once

#include "../Widget.h"

class JournalEntry;

class JournalWidget : public Widget
{
public:
	void Draw(float deltaTime) override;

private:
	void DrawJournalEntriesToGridLayout();

	JournalEntry* selectedJournalEntry = nullptr;
};
