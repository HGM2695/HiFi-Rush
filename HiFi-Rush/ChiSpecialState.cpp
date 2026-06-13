#include "ChiSpecialState.h"
#include "ChiStateMachineComponent.h"

namespace gm
{
	/// Hibiki //////////////////////////////////////////////////////////////////////////////
	ChiHibikiReadyState::ChiHibikiReadyState()
		: ChiClipState(ChiStateId::HibikiReady, ChiAnimationId::HibikiReady)
	{
	}

	void ChiHibikiReadyState::Tick(ChiStateContext& context, float deltaTime)
	{
		if (IsAnimationCompleted(context))
			context.stateMachine->ChangeState(ChiStateId::HibikiAttack);
	}

	ChiHibikiAttackState::ChiHibikiAttackState()
		: ChiClipState(ChiStateId::HibikiAttack, ChiAnimationId::HibikiAttack)
	{
	}

	void ChiHibikiAttackState::Tick(ChiStateContext& context, float deltaTime)
	{
		if (IsAnimationCompleted(context))
			ReturnToIdleOrRun(context);
	}
}
