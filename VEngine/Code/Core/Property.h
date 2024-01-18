#pragma once

#include "Core/UID.h"
#include <string>
#include <optional>
#include <functional>
#include <typeindex>

struct Property
{
	//Function to call when the set value is changed.
	std::function<void(void*)> change;

	std::string name;

	//Path to take filenames from when working with autocomplete on stringwidgets.
	//Need to pass in paths with leading & trailing slashes. Eg. "/Dialogues/"
	std::string autoCompletePath;

	std::optional<std::type_index> info;

	uint64_t size = 0;

	void* data = nullptr;

	UID ownerUID = 0;

	//Sets widgets to be inactive/readonly
	bool readOnly = false;

	//hide property in UI and skips over copying properties across to new props
	bool hide = false;

	template <typename T>
	T* GetData()
	{
		return (T*)data;
	}
};
