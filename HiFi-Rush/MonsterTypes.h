#pragma once

#include "MathTypes.h"
#include "Types.h"

namespace gm
{
	enum class MonsterType : uint32
	{
		Sjango,
		Sword,
		Gunner,

		Count,
	};

	struct MonsterSpawnData
	{
		MonsterType	type = MonsterType::Count;
		Matrix		world = Matrix::Identity;

		int32		maxHealth = 1;
		bool		isInvincible = false;
		Vector3		bodyColliderCenter{};
		Vector3		bodyColliderSize{ 1.f, 1.f, 1.f };
		float		moveSpeed = 0.f;
		float		rotationInterpSpeed = 0.f;
		float		attackCooldownBeats = 0.f;
		int32		attackDamage = 0;
		float		attackRangeMin = 0.f;
		float		attackRangeMax = 0.f;
	};
}
