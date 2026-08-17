#pragma once

namespace gm
{
	enum class ChiStateId
	{
		None,

		Idle,
		Run,

		AttackWeakDash,
		AttackStrongDash,
		AttackWeak0,
		AttackWeak1,
		AttackWeak2,
		AttackWeak3,
		AttackStrong0_0,
		AttackStrong0_1,
		AttackStrong1,
		AttackStrong2,
		AttackStrongToWeak1,
		AttackStrongToWeak2,
		AttackStrongToWeakBeatHit,
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

		DamageStrongKnockback,
		DamageWeakKnockback,
		DamageDead,
		DamageElectric,

		HibikiReady,
		HibikiAttack,
	};
}
