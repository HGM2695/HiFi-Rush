#include "ChiDamageState.h"
#include "ChiStateMachineComponent.h"

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

	/// Weak //////////////////////////////////////////////////////////////////////////////
	ChiWeakKnockbackDamageState::ChiWeakKnockbackDamageState()
		: ChiDamageState(ChiStateId::DamageWeakKnockback, ChiAnimationClipId::DamageWeakKnockback, true)
	{
	}

	/// Strong //////////////////////////////////////////////////////////////////////////////
	ChiStrongKnockbackDamageState::ChiStrongKnockbackDamageState()
		: ChiDamageState(ChiStateId::DamageStrongKnockback, ChiAnimationClipId::DamageStrongKnockback, true)
	{
	}

	/// Dead //////////////////////////////////////////////////////////////////////////////
	ChiDeadDamageState::ChiDeadDamageState()
		: ChiDamageState(ChiStateId::DamageDead, ChiAnimationClipId::DamageDead, false)
	{
	}

	void ChiDeadDamageState::Enter(ChiStateContext& context)
	{
		ChiDamageState::Enter(context);
		_isAnimationCompletionPublished = false;
	}

	void ChiDeadDamageState::Tick(ChiStateContext& context, float)
	{
		if (_isAnimationCompletionPublished || IsAnimationCompleted(context) == false)
			return;

		_isAnimationCompletionPublished = true;
		context.stateMachine->CompleteDeathAnimation();
	}
}
