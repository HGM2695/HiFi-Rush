#pragma once

#include "Types.h"

#include <array>
#include <string>

namespace gm
{
	enum class ChiAnimationId
	{
		AttackWeakDash,
		AttackStrongDash,

		AttackWeak0,
		AttackWeak1,
		AttackWeak2,
		AttackWeak3,

		AttackStrong1,
		AttackStrong2,
		AttackStrong0_0,
		AttackStrong0_1,

		AttackStrongToWeak1,
		AttackStrongToWeak2,

		AttackWeakToStrong1,
		AttackWeakToStrong2,

		AttackDelayedWeak1,
		AttackDelayedWeak2,

		AttackStump0,
		AttackStump1,
		AttackStump2,

		AttackSky0,
		AttackSky1,
		AttackSky2,
		AttackSky3,

		DamageStrongKnockback,
		DamageWeakKnockback,
		DamageDead,
		DamageElectric,

		Idle,

		DashFront,
		DashBack,
		DashLeft,
		DashRight,
		DashDouble,
		DashTriple,
		DashSky,
		DashSkyFall,

		JumpUp,
		JumpDown,
		JumpLanding,
		JumpDoubleUp,
		JumpDoubleDown,

		RunFront,
		RunLeft,
		RunRight,
		RunStart,
		RunLanding,

		RightStartBitHeat,

		HibikiReady,
		HibikiAttack,

		Count
	};

	inline constexpr uint32 ToIndex(ChiAnimationId animationId)
	{
		return static_cast<uint32>(animationId);
	}

	inline constexpr uint32 ChiAnimationIdCount = ToIndex(ChiAnimationId::Count);

	inline constexpr std::array<const wchar_t*, ChiAnimationIdCount> ChiAnimationNames =
	{
		L"ANIM_ATTACK_WEAK_DASH",
		L"ANIM_ATTACK_STRONG_DASH",

		L"ANIM_ATTACK_WEAK0",
		L"ANIM_ATTACK_WEAK1",
		L"ANIM_ATTACK_WEAK2",
		L"ANIM_ATTACK_WEAK3",

		L"ANIM_ATTACK_STRONG1",
		L"ANIM_ATTACK_STRONG2",
		L"ANIM_ATTACK_STRONG0_0",
		L"ANIM_ATTACK_STRONG0_1",

		L"ANIM_ATTACK_RIGHT_START1",
		L"ANIM_ATTACK_RIGHT_START2",

		L"ANIM_ATTACK_LEFT_START1",
		L"ANIM_ATTACK_LEFT_START2",

		L"ANIM_ATTACK_ONEPASS1",
		L"ANIM_ATTACK_ONEPASS2",

		L"ANIM_ATTACK_STUMP0",
		L"ANIM_ATTACK_STUMP1",
		L"ANIM_ATTACK_STUMP2",

		L"ANIM_ATTACK_SKY0",
		L"ANIM_ATTACK_SKY1",
		L"ANIM_ATTACK_SKY2",
		L"ANIM_ATTACK_SKY3",

		L"ANIM_DAMAGE_STRONG_KNOCKBACK",
		L"ANIM_DAMAGE_WEAK_KNOCKBACK",
		L"ANIM_DAMAGE_DEAD",
		L"ANIM_DAMAGE_ELECTRIC",

		L"ANIM_IDLE",

		L"ANIM_DASH_FRONT",
		L"ANIM_DASH_BACK",
		L"ANIM_DASH_LEFT",
		L"ANIM_DASH_RIGHT",
		L"ANIM_DASH_DOUBLE",
		L"ANIM_DASH_TRIPLE",
		L"ANIM_DASH_SKY",
		L"ANIM_DASH_SKY_FALL",

		L"ANIM_JUMP_UP",
		L"ANIM_JUMP_DOWN",
		L"ANIM_JUMP_LANDING",
		L"ANIM_JUMP_DOUBLE_UP",
		L"ANIM_JUMP_DOUBLE_DOWN",

		L"ANIM_RUN_FRONT",
		L"ANIM_RUN_LEFT",
		L"ANIM_RUN_RIGHT",
		L"ANIM_RUN_START",
		L"ANIM_RUN_RANDING",

		L"ANIM_RIGHTSTART_BITHEAT",

		L"ANIM_HIBIKI_READY",
		L"ANIM_HIBIKI_ATTACK",
	};

	inline std::wstring GetChiAnimationName(ChiAnimationId animationId)
	{
		return ChiAnimationNames[ToIndex(animationId)];
	}

	inline std::wstring GetChiAnimationKey(ChiAnimationId animationId)
	{
		return std::wstring(L"chi.") + ChiAnimationNames[ToIndex(animationId)];
	}
}
