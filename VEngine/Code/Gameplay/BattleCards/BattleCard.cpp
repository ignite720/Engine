#include "vpch.h"
#include "BattleCard.h"
#include "Actors/Game/Player.h"

BattleCard::BattleCard(std::wstring name_, std::wstring desc_, std::string imageFilename_)
{
	name = name_;
	desc = desc_;
	imageFilename = imageFilename_;
}

void BattleCard::Activate()
{
	auto player = Player::system.GetFirstActor();

	if (!player->CheckAndExpendActionPoints(cost))
	{
		return;
	}

	for (int i = 0; i < player->battleCardsInHand.size(); i++)
	{
		auto card = player->battleCardsInHand[i];
		if (card == this)
		{
			player->battleCardsInHand.erase(player->battleCardsInHand.begin() + i);
			return;
		}
	}
}
