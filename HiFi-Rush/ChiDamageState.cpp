#include "ChiDamageState.h"

namespace gm
{
	/// Damage //////////////////////////////////////////////////////////////////////////////
	ChiDamageState::ChiDamageState(ChiStateId stateId, ChiAnimationClipId animationClipId, bool returnToBaseMotion)
		: ChiState(stateId, animationClipId)
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

	void ChiDamageState::OnGroundContact(ChiStateContext&)
	{
	}

	void ChiDamageState::OnGroundLost(ChiStateContext&, const NavigationGroundLostEvent&)
	{
	}

	ChiWeakKnockbackDamageState::ChiWeakKnockbackDamageState()
		: ChiDamageState(ChiStateId::DamageWeakKnockback, ChiAnimationClipId::DamageWeakKnockback, true)
	{
	}

	ChiStrongKnockbackDamageState::ChiStrongKnockbackDamageState()
		: ChiDamageState(ChiStateId::DamageStrongKnockback, ChiAnimationClipId::DamageStrongKnockback, true)
	{
	}

	ChiDeadDamageState::ChiDeadDamageState()
		: ChiDamageState(ChiStateId::DamageDead, ChiAnimationClipId::DamageDead, false)
	{
	}
}
