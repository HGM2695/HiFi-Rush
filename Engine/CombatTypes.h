#pragma once

#include "CollisionTypes.h"
#include "Event.h"

namespace gm
{
	class HitBoxComponent;
	class HurtBoxComponent;

	struct HitEvent : EventType
	{
		HitBoxComponent*	hitBox = nullptr;
		HurtBoxComponent*	hurtBox = nullptr;
		CollisionContact	contact{};
	};
}
