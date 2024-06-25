#include "vpch.h"
#include "Input.h"
#include <GameInput.h>
#include "Core/Debug.h"

IGameInput* gGameInput;
IGameInputDevice* gGamepad;

std::vector<GameInputKeyState> gPreviousFrameKeyState;

IGameInputReading* keyboardInputReading;
IGameInputReading* mouseInputReading;

int64_t previousMouseX;
int64_t previousMouseY;

int64_t previousMouseWheelY;

bool previousMouseLeftDown;
bool previousMouseRightDown;
bool previousMouseMiddleDown;

std::set<Keys> currentHeldKeys;
std::set<Keys> currentDownKeys;
std::set<Keys> currentUpKeys;

std::multimap<std::string, Keys> keyMap;

void PollKeyboardInput();
void PollMouseInput();

namespace Input
{
	bool mouseWheelUp;
	bool mouseWheelDown;

	bool blockInput = false;

	bool mouseLeftUp;
	bool mouseLeftDown;
	bool mouseRightUp;
	bool mouseRightDown;
	bool mouseMiddleUp;
	bool mouseMiddleDown;

	void PollInput()
	{
		PollKeyboardInput();
		PollMouseInput();
	}

	void InitKeyMap()
	{
		keyMap.emplace("MoveForward", Keys::W);
		keyMap.emplace("MoveBack", Keys::S);
		keyMap.emplace("MoveLeft", Keys::A);
		keyMap.emplace("MoveRight", Keys::D);

		keyMap.emplace("RotateUp", Keys::Up);
		keyMap.emplace("RotateDown", Keys::Down);
		keyMap.emplace("RotateLeft", Keys::Left);
		keyMap.emplace("RotateRight", Keys::Right);

		keyMap.emplace("Link", Keys::Enter);
		keyMap.emplace("Unlink", Keys::BackSpace);

		keyMap.emplace("ToggleGrid", Keys::Space);

		keyMap.emplace("Interact", Keys::Down);

		keyMap.emplace("OpenJournal", Keys::J);

		HR(GameInputCreate(&gGameInput));
	}

	void Reset()
	{
		currentUpKeys.clear();
		currentDownKeys.clear();

		mouseWheelUp = false;
		mouseWheelDown = false;
		mouseLeftUp = false;
		mouseRightUp = false;
		mouseMiddleUp = false;
	}

	void ResetHeldKeys()
	{
		currentHeldKeys.clear();
	}

	void SetKeyDown(Keys key)
	{
		currentDownKeys.insert(key);
		currentHeldKeys.insert(key);
		currentUpKeys.erase(key);
	}

	void SetKeyUp(Keys key)
	{
		currentUpKeys.insert(key);
		currentDownKeys.erase(key);
		currentHeldKeys.erase(key);
	}

	bool GetKeyDown(std::string keyMapping)
	{
		auto mappingIt = keyMap.find(keyMapping);
		if (mappingIt != keyMap.end())
		{
			if (currentDownKeys.contains(mappingIt->second))
			{
				return true;
			}
		}
		return false;
	}

	bool GetKeyUp(std::string keyMapping)
	{
		auto mappingIt = keyMap.find(keyMapping);
		if (mappingIt != keyMap.end())
		{
			if (currentUpKeys.contains(mappingIt->second))
			{
				return true;
			}
		}
		return false;
	}

	bool GetKeyHeld(std::string keyMapping)
	{
		auto mappingIt = keyMap.find(keyMapping);
		if (mappingIt != keyMap.end())
		{
			if (currentHeldKeys.contains(mappingIt->second))
			{
				return true;
			}
		}
		return false;
	}

	bool GetKeyDown(Keys key)
	{
		if (blockInput) return false;
		return currentDownKeys.find(key) != currentDownKeys.end();
	}

	bool GetKeyUp(Keys key)
	{
		if (blockInput) return false;
		return currentUpKeys.find(key) != currentUpKeys.end();
	}

	bool GetKeyHeld(Keys key)
	{
		if (blockInput) return false;
		return currentHeldKeys.find(key) != currentHeldKeys.end();
	}

	bool GetAnyKeyDown()
	{
		if (blockInput) return false;
		return currentDownKeys.size();
	}

	bool GetAnyKeyUp()
	{
		if (blockInput) return false;
		return currentUpKeys.size();
	}

	void SetLeftMouseDown()
	{
		previousMouseLeftDown = true;
		mouseLeftDown = true;
		mouseLeftUp = false;
	}

	void SetRightMouseUp()
	{
		previousMouseRightDown = false;
		mouseRightUp = true;
		mouseRightDown = false;
	}

	void SetRightMouseDown()
	{
		previousMouseRightDown = true;
		mouseRightDown = true;
		mouseRightUp = false;
	}

	void SetMiddleMouseUp()
	{
		previousMouseMiddleDown = false;
		mouseMiddleUp = true;
		mouseMiddleDown = false;
	}

	void SetMiddleMouseDown()
	{
		previousMouseMiddleDown = true;
		mouseMiddleDown = true;
		mouseMiddleUp = false;
	}

	void SetLeftMouseUp()
	{
		previousMouseLeftDown = false;
		mouseLeftUp = true;
		mouseLeftDown = false;
	}

	bool GetMouseLeftUp()
	{
		if (blockInput) return false;
		return mouseLeftUp;
	}

	bool GetMouseRightUp()
	{
		if (blockInput) return false;
		return mouseRightUp;
	}

	bool GetMouseLeftDown()
	{
		if (blockInput) return false;
		return mouseLeftDown;
	}

	bool GetMouseRightDown()
	{
		if (blockInput) return false;
		return mouseRightDown;
	}

	bool GetMouseMiddleUp()
	{
		if (blockInput) return false;
		return mouseMiddleUp;
	}

	bool GetMouseMiddleDown()
	{
		if (blockInput) return false;
		return mouseMiddleDown;
	}

	size_t GetNumCurrentKeysDown()
	{
		return currentDownKeys.size();
	}

	size_t GetNumCurrentKeysUp()
	{
		return currentUpKeys.size();
	}

	std::set<Keys> GetAllDownKeys()
	{
		return currentDownKeys;
	}

	std::set<Keys> GetAllUpKeys()
	{
		return currentUpKeys;
	}
}

void PollKeyboardInput()
{
	const auto ScanCodeToVirtualKey = [](UINT scanCode) -> Keys
		{
			const UINT key = MapVirtualKeyA(scanCode, MAPVK_VSC_TO_VK);
			return (Keys)key;
		};

	if (SUCCEEDED(gGameInput->GetCurrentReading(GameInputKindKeyboard, gGamepad, &keyboardInputReading)))
	{
		const uint32_t keyCount = keyboardInputReading->GetKeyCount();
		std::vector<GameInputKeyState> keyStates(keyCount);
		keyboardInputReading->GetKeyState(keyCount, keyStates.data());

		for (const auto& keyState : keyStates)
		{
			const Keys key = ScanCodeToVirtualKey(keyState.scanCode);
			Input::SetKeyDown(key);
		}

		//Set key up if doesn't exist on current frame.
		for (const auto& keyState : gPreviousFrameKeyState)
		{
			const auto key = ScanCodeToVirtualKey(keyState.scanCode);
			if (!Input::GetKeyDown(key))
			{
				Input::SetKeyUp(key);
			}
		}

		gPreviousFrameKeyState = keyStates;
	}
}

void PollMouseInput()
{
	if (SUCCEEDED(gGameInput->GetCurrentReading(GameInputKindMouse, gGamepad, &mouseInputReading)))
	{
		GameInputMouseState mouseState = {};
		mouseInputReading->GetMouseState(&mouseState);

		//Mouse wheel
		const int64_t mouseWheelDeltaY = mouseState.wheelY - previousMouseWheelY;

		if (mouseWheelDeltaY > 0)
		{
			Input::mouseWheelUp = true;
		}
		else if (mouseWheelDeltaY < 0)
		{
			Input::mouseWheelDown = true;
		}

		previousMouseWheelY = mouseState.wheelY;

		//Mouse position
		const int64_t mouseDeltaX = mouseState.positionX - previousMouseX;
		const int64_t mouseDeltaY = mouseState.positionY - previousMouseY;

		previousMouseX = mouseState.positionX;
		previousMouseY = mouseState.positionY;

		//Button input
		if (previousMouseLeftDown)
		{
			Input::SetLeftMouseUp();
		}
		else if (mouseState.buttons & GameInputMouseButtons::GameInputMouseLeftButton)
		{
			Input::SetLeftMouseDown();
		}

		if (previousMouseRightDown)
		{
			Input::SetRightMouseUp();
		}
		else if (mouseState.buttons & GameInputMouseButtons::GameInputMouseRightButton)
		{
			Input::SetRightMouseDown();
		}

		if (previousMouseMiddleDown)
		{
			Input::SetMiddleMouseUp();
		}
		if (mouseState.buttons & GameInputMouseButtons::GameInputMouseMiddleButton)
		{
			Input::SetMiddleMouseDown();
		}
	}
}
