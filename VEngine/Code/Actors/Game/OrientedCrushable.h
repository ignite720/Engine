#pragma once

#include "Crushable.h"

//Crushable actor that can be crushed/broken when it's correct orientation is hit.
class OrientedCrushable : public Crushable
{
public:
	ACTOR_SYSTEM(OrientedCrushable);
	DEFAULT_PROPS;

	void Crushed() override;
};