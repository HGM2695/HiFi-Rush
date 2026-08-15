#pragma once

#include "Types.h"

#include <array>
#include <string>

namespace gm
{
	enum class SwordAnimationId : uint32
	{
		Idle,
		AttackSlash,
		AttackJumpSlash,
		WalkFront,
		WalkBack,
		WalkRight,
		WalkLeft,
		Appear,
		DamageLow0,
		DamageLow1,
		DamageLow2,
		DamageMiddle,
		DamageHigh,
		AirDamageStart,
		AirDamageFall,
		AirDamage0,
		AirDamage1,
		AirDamage2,
		DownWakeUp,
		DownFall,
		Die,
		DashFront,
		DashBack,

		Count,
	};

	inline constexpr uint32 ToIndex(SwordAnimationId animationId)
	{
		return static_cast<uint32>(animationId);
	}

	inline constexpr uint32 SwordAnimationIdCount = ToIndex(SwordAnimationId::Count);

	inline constexpr std::array<const wchar_t*, SwordAnimationIdCount> SwordAnimationClipNames =
	{
		L"ANIM_IDLE",
		L"ANIM_ATTACK_SLASH",
		L"ANIM_ATTACK_JUMP_SLASH",
		L"ANIM_WALK_FRONT",
		L"ANIM_WALK_BACK",
		L"ANIM_WALK_RIGHT",
		L"ANIM_WALK_LEFT",
		L"ANIM_APPEAR",
		L"ANIM_DAMAGE_LOW_0",
		L"ANIM_DAMAGE_LOW_1",
		L"ANIM_DAMAGE_LOW_2",
		L"ANIM_DAMAGE_MIDDLE",
		L"ANIM_DAMAGE_HIGH",
		L"ANIM_AIR_DAMAGE_START",
		L"ANIM_AIR_DAMAGE_FALL",
		L"ANIM_AIR_DAMAGE_0",
		L"ANIM_AIR_DAMAGE_1",
		L"ANIM_AIR_DAMAGE_2",
		L"ANIM_DOWN_WAKE_UP",
		L"ANIM_DOWN_FALL",
		L"ANIM_DIE",
		L"ANIM_DASH_FRONT",
		L"ANIM_DASH_BACK",
	};

	inline std::wstring GetSwordAnimationClipName(SwordAnimationId animationId)
	{
		return SwordAnimationClipNames[ToIndex(animationId)];
	}
}
