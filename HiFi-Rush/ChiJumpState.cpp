#include "ChiJumpState.h"
#include "Application.h"
#include "ChiStateMachineComponent.h"
#include "Input.h"

namespace gm
{
	/// Jump //////////////////////////////////////////////////////////////////////////////
	ChiJumpUpState::ChiJumpUpState()
		: ChiClipState(ChiStateId::JumpUp, ChiAnimationId::JumpUp)
	{
	}

	void ChiJumpUpState::Tick(ChiStateContext& context, float deltaTime)
	{
		TryChangeAirAction(context, true);
		if (IsAnimationCompleted(context))
			context.stateMachine->ChangeState(ChiStateId::JumpDown);
	}

	ChiJumpDownState::ChiJumpDownState()
		: ChiClipState(ChiStateId::JumpDown, ChiAnimationId::JumpDown, false)
	{
	}

	void ChiJumpDownState::Tick(ChiStateContext& context, float deltaTime)
	{
		TryChangeAirAction(context, true);

		// 임시로 loop를 false로 두고 Landing으로 전환합니다. 원래는 충돌 전까지 loop됩니다.
		if (IsAnimationCompleted(context))
			context.stateMachine->ChangeState(ChiStateId::JumpLanding);
	}

	ChiJumpLandingState::ChiJumpLandingState()
		: ChiClipState(ChiStateId::JumpLanding, ChiAnimationId::JumpLanding)
	{
	}

	void ChiJumpLandingState::Tick(ChiStateContext& context, float deltaTime)
	{
		const Input& input = APPLICATION.GetInput();

		if (input.IsKeyDown(KeyCode::Space))
		{
			context.stateMachine->ChangeState(ChiStateId::JumpUp);
			return;
		}

		if (input.IsKeyDown(KeyCode::LeftShift))
		{
			ChangeDashStateByInput(context);
			return;
		}

		if (IsAnimationCompleted(context))
			ReturnToIdleOrRun(context);
	}

	/// DoubleJump //////////////////////////////////////////////////////////////////////////////
	ChiJumpDoubleUpState::ChiJumpDoubleUpState()
		: ChiClipState(ChiStateId::JumpDoubleUp, ChiAnimationId::JumpDoubleUp)
	{
	}

	void ChiJumpDoubleUpState::Tick(ChiStateContext& context, float deltaTime)
	{
		if (TryChangeAirAction(context, false))
			return;

		if (IsAnimationCompleted(context))
			context.stateMachine->ChangeState(ChiStateId::JumpDoubleDown);
	}

	ChiJumpDoubleDownState::ChiJumpDoubleDownState()
		: ChiClipState(ChiStateId::JumpDoubleDown, ChiAnimationId::JumpDoubleDown, false)
	{
	}

	void ChiJumpDoubleDownState::Tick(ChiStateContext& context, float deltaTime)
	{
		TryChangeAirAction(context, false);
		// 임시로 loop를 false로 두고 Landing으로 전환합니다. 원래는 충돌 전까지 loop됩니다.
		if (IsAnimationCompleted(context))
			context.stateMachine->ChangeState(ChiStateId::JumpLanding);
	}
}
