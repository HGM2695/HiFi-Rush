#include "ChiIdleRunState.h"
#include "ChiMoveComponent.h"
#include "ChiStateMachineComponent.h"

namespace gm
{
	/// Idle //////////////////////////////////////////////////////////////////////////////
	ChiIdleState::ChiIdleState()
		: ChiState(ChiStateId::Idle, ChiAnimationClipId::Idle, true)
	{
	}

	void ChiIdleState::Tick(ChiStateContext& context, float deltaTime)
	{
		if (TryChangeGroundAction(context))
			return;

		if (IsMoveInputPressed(context))
			context.stateMachine->ChangeState(ChiStateId::Run);
	}

	/// Run //////////////////////////////////////////////////////////////////////////////
	ChiRunState::ChiRunState()
		: ChiState(ChiStateId::Run, ChiAnimationClipId::RunFront, true)
	{
	}

	void ChiRunState::Tick(ChiStateContext& context, float deltaTime)
	{
		if (TryChangeGroundAction(context))
			return;

		if (context.moveComponent->IsMoving() == false)
			context.stateMachine->ChangeState(ChiStateId::Idle);
	}
}
