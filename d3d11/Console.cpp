#include "Console.h"
#include "Input.h"
#include "UISystem.h"
#include "CoreSystem.h"
#include "RenderSystem.h"
#include <vector>
#include "FileSystem.h"
#include "World.h"

wchar_t consoleString[128];
int consoleStringIndex = 0;
bool bConsoleActive = false;

std::vector<ConsoleViewItem> viewItems;

namespace ExecuteStrings
{
	const wchar_t* EXIT = L"EXIT";
	const wchar_t* GPU = L"GPU";
	const wchar_t* D3D_TIME = L"D3D TIME";
	const wchar_t* POP = L"POP";
	const wchar_t* CLEAR = L"CLEAR";
	const wchar_t* LEVEL = L"LEVEL";
}

namespace LevelNames
{
	const wchar_t* TestLevel = L"TESTLEVEL";
}

void Console::ConsoleInput()
{
	if (inputSystem.GetAnyKeyDown() && inputSystem.currentDownKey != VK_OEM_3)
	{
		if (inputSystem.GetKeyDownState(VK_BACK) && consoleStringIndex > 0)
		{
			consoleStringIndex--;
		}
		else if ((consoleStringIndex < _countof(consoleString)) && (inputSystem.currentDownKey != 0))
		{
			consoleString[consoleStringIndex] = inputSystem.currentDownKey; //wchar_t to int64 cast works
			consoleStringIndex++;
		}
	}
}

//Make sure D2D render target calls have been made (Begin/End Draw)
void Console::Tick()
{
	if (bConsoleActive)
	{
		Console::ConsoleInput();

		float width = (float)coreSystem.windowWidth;
		float height = (float)coreSystem.windowHeight;

		uiSystem.d2dRenderTarget->DrawRectangle({ 0, height - 20.f, width, height }, uiSystem.brushTransparentMenu);
		uiSystem.d2dRenderTarget->DrawText(consoleString, consoleStringIndex, uiSystem.textFormat,
			{ 0, height - 20.f, width, height }, uiSystem.brushText);
	}

	if (inputSystem.GetKeyUpState(VK_OEM_3)) //~ key, like doom and unreal
	{
		bConsoleActive = !bConsoleActive;
	}
	else if (inputSystem.GetKeyDownState(VK_RETURN) && bConsoleActive)
	{
		Console::ExecuteString();
	}
}

//Execute values need to be uppercase with WndProc
void Console::ExecuteString()
{
	ConsoleViewItem item = {};

	if (wcsncmp(consoleString, ExecuteStrings::EXIT, wcslen(ExecuteStrings::EXIT)) == 0)
	{
		coreSystem.msg.message = WM_QUIT; //TODO: Terrible. Fix.
	}
	else if (wcsncmp(consoleString, ExecuteStrings::GPU, wcslen(ExecuteStrings::GPU)) == 0)
	{
		_snwprintf_s(item.text, sizeof(item.text), L"%ls", renderSystem.adaptersDesc[0].Description);
		viewItems.push_back(item);
	}
	else if (wcsncmp(consoleString, ExecuteStrings::D3D_TIME, wcslen(ExecuteStrings::D3D_TIME)) == 0)
	{
		//TODO: need to figure out a better system to display real time data
		_snwprintf_s(item.text, sizeof(item.text), L"D3D11 Render: %f", renderSystem.renderTime);
		viewItems.push_back(item);
	}
	else if (wcsncmp(consoleString, ExecuteStrings::POP, wcslen(ExecuteStrings::POP)) == 0)
	{
		if (viewItems.size() > 0)
		{
			viewItems.pop_back();
		}
	}
	else if (wcsncmp(consoleString, ExecuteStrings::CLEAR, wcslen(ExecuteStrings::CLEAR)) == 0) //CLEAR TEXT
	{
		viewItems.clear();
	}
	else if(wcsncmp(consoleString, LevelNames::TestLevel, wcslen(LevelNames::TestLevel)) == 0) //LEVEL LOADING
	{
		g_FileSystem.ReadAllActorSystems(GetWorld(), "LevelSaves/test.sav");
	}

	//Reset console string and index
	memset(consoleString, 0, _countof(consoleString) * sizeof(wchar_t));
	consoleStringIndex = 0;
}

void Console::DrawViewItems()
{
	for (int i = 0; i < viewItems.size(); i++)
	{
		const float yMarginIncrement = 20.f * i;

		const float width = (float)coreSystem.windowWidth;
		const float height = (float)coreSystem.windowHeight;

		uiSystem.d2dRenderTarget->DrawTextA(viewItems[i].text, wcslen(viewItems[i].text), uiSystem.textFormat, 
			{ 0, yMarginIncrement, width, height }, uiSystem.brushText);

	}
}
