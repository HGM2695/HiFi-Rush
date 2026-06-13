#include "ChiDamageState.h"

namespace gm
{
	/// Damage //////////////////////////////////////////////////////////////////////////////
	ChiDamageState::ChiDamageState(ChiStateId stateId, ChiAnimationId animationId, bool returnToBaseMotion)
		: ChiClipState(stateId, animationId)
		, _returnToBaseMotion(returnToBaseMotion)
	{
	}

	void ChiDamageState::Tick(ChiStateContext& context, float deltaTime)
	{
		if (IsAnimationCompleted(context) == false)
			return;

		if (_returnToBaseMotion)
			ReturnToIdleOrRun(context);
	}

	ChiWeakKnockbackDamageState::ChiWeakKnockbackDamageState()
		: ChiDamageState(ChiStateId::DamageWeakKnockback, ChiAnimationId::DamageWeakKnockback, true)
	{
	}

	ChiStrongKnockbackDamageState::ChiStrongKnockbackDamageState()
		: ChiDamageState(ChiStateId::DamageStrongKnockback, ChiAnimationId::DamageStrongKnockback, true)
	{
	}

	ChiDeadDamageState::ChiDeadDamageState()
		: ChiDamageState(ChiStateId::DamageDead, ChiAnimationId::DamageDead, false)
	{
	}
}
