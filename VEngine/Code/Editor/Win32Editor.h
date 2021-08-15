#pragma once
#include "IEditor.h"

struct Win32Editor : IEditor
{
	void Init(int argc, char* argv[]) override;
	void Tick() override;
	void SetMousePos() override;
	virtual void Log(const std::wstring logMessage) override;
	void SetupWindow();
	void HandleMessages();
};
