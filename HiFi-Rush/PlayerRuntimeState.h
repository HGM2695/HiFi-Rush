#pragma once

#include "Types.h"

namespace gm
{
	inline constexpr int32 DefaultPlayerMaxHealth = 100;
	inline constexpr float DefaultPlayerMaxReverb = 100.f;

	struct PlayerRuntimeState
	{
		int32	maxHealth = DefaultPlayerMaxHealth;
		int32	currentHealth = DefaultPlayerMaxHealth;
		float	maxReverb = DefaultPlayerMaxReverb;
		float	currentReverb = 0.f;
	};
}
