#include "vpch.h"
#include "BattleCardSystem.h"
#include "BattleCard.h"
#include "VMath.h"

void BattleCardSystem::AddCard(BattleCard* card)
{
	battleCardMap.emplace(card->name, card);
}

BattleCard* BattleCardSystem::DrawCardAtRandom()
{
	auto battleCardMapIt = battleCardMap.begin();
	const int rand = VMath::RandomRangeInt(0, battleCardMap.size() - 1);
	std::advance(battleCardMapIt, rand);
	return battleCardMapIt->second;
}
