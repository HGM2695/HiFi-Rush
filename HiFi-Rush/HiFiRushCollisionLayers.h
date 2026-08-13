#pragma once

#include "CollisionTypes.h"

namespace gm::HiFiRushCollisionLayer
{
	inline constexpr CollisionLayer Player = 1u << 1;
	inline constexpr CollisionLayer SceneTransitionTrigger = 1u << 2;
}
