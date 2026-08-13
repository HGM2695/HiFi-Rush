#pragma once

#include "CollisionTypes.h"
#include "Event.h"

namespace gm
{
	class HitBoxComponent;
	class HurtBoxComponent;

	struct DamageInfo
	{
		int32	amount = 0;
		bool	ignoreInvincibility = false;
	};

	enum class DamageState
	{
		Ignored,
		Applied,
		Blocked,

		Count
	};

	struct DamageResult
	{
		int32		requestedDamage = 0;
		int32		appliedDamage = 0;
		int32		previousHealth = 0;
		int32		currentHealth = 0;
		DamageState	state = DamageState::Ignored;
		bool		isDead = false;
	};

	struct HitEvent : EventType
	{
		HitBoxComponent*	hitBox = nullptr;
		HurtBoxComponent*	hurtBox = nullptr;
		CollisionContact	contact{};
		DamageInfo			damage{};
		DamageResult		damageResult{};
	};

	struct HealthChangedEvent : EventType
	{
		int32	previousHealth = 0;
		int32	currentHealth = 0;
		int32	maxHealth = 0;
	};
}
