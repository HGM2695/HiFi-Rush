#include "ChiSpecialState.h"
#include "ChiStateMachineComponent.h"

namespace gm
{
	/// Hibiki //////////////////////////////////////////////////////////////////////////////
	ChiHibikiReadyState::ChiHibikiReadyState()
		: ChiState(ChiStateId::HibikiReady, ChiAnimationClipId::HibikiReady)
	{
	}

	void ChiHibikiReadyState::Tick(ChiStateContext& context, float deltaTime)
	{
		if (IsAnimationCompleted(context))
			context.stateMachine->ChangeState(ChiStateId::HibikiAttack);
	}

	ChiHibikiAttackState::ChiHibikiAttackState()
		: ChiState(ChiStateId::HibikiAttack, ChiAnimationClipId::HibikiAttack)
	{
	}

	void ChiHibikiAttackState::Tick(ChiStateContext& context, float deltaTime)
	{
		if (IsAnimationCompleted(context))
			ReturnToIdleOrRun(context);
	}
}
