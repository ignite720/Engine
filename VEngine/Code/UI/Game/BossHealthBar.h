#pragma once

#include "../Widget.h"

class BossHealthBar : public Widget
{
public:
	virtual void Draw(float deltaTime) override;
	void SetBossName(const std::wstring bossName_) { bossName = bossName_; }
	void SetBossHealth(int bossHealth_) { bossHealth = bossHealth_; }

private:
	std::wstring bossName;
	int bossHealth = 100;
};
