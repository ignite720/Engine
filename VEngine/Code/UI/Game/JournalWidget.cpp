#include "vpch.h"
#include "JournalWidget.h"
#include "Gameplay/JournalSystem.h"
#include "UI/GridLayout.h"

void JournalWidget::Draw(float deltaTime)
{
	DrawJournalEntriesToGridLayout();
}

void JournalWidget::DrawJournalEntriesToGridLayout()
{
	auto& journalEntries = JournalSystem::Get().GetJournalEntries();

	if (journalEntries.size() == 0)
	{
		//Nothing to display
		const auto emptyLayout = PercentAlignLayout(0.4f, 0.4f, 0.6f, 0.6f);
		FillRect(emptyLayout);
		Text("Journal empty.", emptyLayout);
		HandleCloseButton();
		return;
	}

	if (selectedJournalEntry != nullptr)
	{
		auto entryLayout = PercentAlignLayout(0.3f, 0.3f, 0.7f, 0.7f);

		FillRect(entryLayout);
		Text(selectedJournalEntry->title, entryLayout);

		entryLayout.PushDown(50.f);
		Text(selectedJournalEntry->text, entryLayout);

		entryLayout.PushDown(150.f);
		if (Button("Close", entryLayout))
		{
			selectedJournalEntry = nullptr;
		}

		return;
	}

	const auto gridOverallLayout = PercentAlignLayout(0.1f, 0.1f, 0.9f, 0.9f);

	const int numRows = 3;
	const int numColumns = 3;

	GridLayout gridLayout;
	gridLayout.SetLayouts(numRows, numColumns, gridOverallLayout, 20.f);

	const std::vector<Layout> gridLayouts = gridLayout.GetAllLayouts();
	int gridLayoutIndex = 0;

	for (auto& [title, entry] : journalEntries)
	{
		const auto& layout = gridLayouts.at(gridLayoutIndex);

		if (Button(title, layout))
		{
			selectedJournalEntry = &entry;
		}

		gridLayoutIndex++;
	}

	HandleCloseButton();
}

void JournalWidget::HandleCloseButton()
{
	const auto closeButtonLayout = PercentAlignLayout(0.8f, 0.8f, 0.9f, 0.9f);
	if (Button(L"Close", closeButtonLayout))
	{
		this->RemoveFromViewport();
	}
}