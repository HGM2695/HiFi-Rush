#pragma once

#include "Types.h"

namespace gm
{
	inline constexpr int32 DefaultPlayerMaxHealth = 100;

	struct PlayerRuntimeState
	{
		int32	maxHealth = DefaultPlayerMaxHealth;
		int32	currentHealth = DefaultPlayerMaxHealth;
	};
}
