#pragma once

#include "Types.h"

#include <array>
#include <string>

namespace gm
{
	enum class GunnerAnimationId : uint32
	{
		GroundAttackReady,
		GroundAttackShoot,
		GroundAttackLanding,
		SkyAttackReady,
		SkyAttackShoot,
		SkyAttackLanding,
		DamageLow,
		AirDamageBasic,
		AirDamageFall,
		AirDamage0,
		AirDamage1,
		AirDamage2,
		Die,
		Idle,
		DownWakeUp,
		DownFall,
		Appear,
		DashLeft,
		DashRight,
		DashFront,
		DashBack,
		WalkFront,
		WalkBack,
		WalkLeft,
		WalkRight,

		Count,
	};

	inline constexpr uint32 ToIndex(GunnerAnimationId animationId)
	{
		return static_cast<uint32>(animationId);
	}

	inline constexpr uint32 GunnerAnimationIdCount = ToIndex(GunnerAnimationId::Count);

	inline constexpr std::array<const wchar_t*, GunnerAnimationIdCount> GunnerAnimationClipNames =
	{
		L"ANIM_GROUND_ATTACK_READY",
		L"ANIM_GROUND_ATTACK_SHOOT",
		L"ANIM_GROUND_ATTACK_LANDING",
		L"ANIM_SKY_ATTACK_READY",
		L"ANIM_SKY_ATTACK_SHOOT",
		L"ANIM_SKY_ATTACK_LANDING",
		L"ANIM_DAMAGE_LOW",
		L"ANIM_AIR_DAMAGE_BASIC",
		L"ANIM_AIR_DAMAGE_FALL",
		L"ANIM_AIR_DAMAGE_0",
		L"ANIM_AIR_DAMAGE_1",
		L"ANIM_AIR_DAMAGE_2",
		L"ANIM_DIE",
		L"ANIM_IDLE",
		L"ANIM_DOWN_WAKE_UP",
		L"ANIM_DOWN_FALL",
		L"ANIM_APPEAR",
		L"ANIM_DASH_LEFT",
		L"ANIM_DASH_RIGHT",
		L"ANIM_DASH_FRONT",
		L"ANIM_DASH_BACK",
		L"ANIM_WALK_FRONT",
		L"ANIM_WALK_BACK",
		L"ANIM_WALK_LEFT",
		L"ANIM_WALK_RIGHT",
	};

	inline std::wstring GetGunnerAnimationClipName(GunnerAnimationId animationId)
	{
		return GunnerAnimationClipNames[ToIndex(animationId)];
	}
}
