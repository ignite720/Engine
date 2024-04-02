#pragma once

#include "GridActor.h"

class Steam;

class SteamValve : public GridActor
{
public:
	ACTOR_SYSTEM(SteamValve);

	void Create() override;
	void Start() override;
	Properties GetProps() override;

	void OnLinkRotate() override;

private:
	void EnableDisableSteam();

	std::string steamName;
	Steam* steam = nullptr;
	bool isValveOn = true;
};
