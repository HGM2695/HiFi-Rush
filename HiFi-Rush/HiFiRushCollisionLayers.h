#pragma once

#include "CollisionTypes.h"

namespace gm::HiFiRushCollisionLayer
{
	inline constexpr CollisionLayer Player = 1u << 1;
	inline constexpr CollisionLayer SceneTransitionTrigger = 1u << 2;
	inline constexpr CollisionLayer Monster = 1u << 3;
	inline constexpr CollisionLayer PlayerAttack = 1u << 4;
	inline constexpr CollisionLayer MonsterAttack = 1u << 5;
}
