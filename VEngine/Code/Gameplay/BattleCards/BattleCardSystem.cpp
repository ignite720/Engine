#include "vpch.h"
#include "BattleCardSystem.h"
#include "BattleCard.h"
#include "VMath.h"

void BattleCardSystem::AddCard(BattleCard* card)
{
	assert(battleCardMap.find(card->name) == battleCardMap.end());
	battleCardMap.emplace(card->name, card);
}

BattleCard* BattleCardSystem::DrawCardAtRandom()
{
	auto battleCardMapIt = battleCardMap.begin();
	int rand = VMath::RandomRangeInt(0, battleCardMap.size() - 1);
	std::advance(battleCardMapIt, rand);
	return battleCardMapIt->second;
}
