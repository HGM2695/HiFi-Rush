#include "ChiIdleRunState.h"
#include "Application.h"
#include "ChiMoveComponent.h"
#include "ChiStateMachineComponent.h"
#include "Input.h"

namespace gm
{
	namespace
	{
		bool IsMoveInputPressed(const Input& input)
		{
			return input.IsKeyRepeat(KeyCode::W) || input.IsKeyRepeat(KeyCode::A) || input.IsKeyRepeat(KeyCode::S) || input.IsKeyRepeat(KeyCode::D);
		}
	}

	/// Idle //////////////////////////////////////////////////////////////////////////////
	void ChiIdleState::Enter(ChiStateContext& context)
	{
		PlayAnimation(context, ChiAnimationId::Idle, true);
	}

	void ChiIdleState::Tick(ChiStateContext& context, float deltaTime)
	{
		const Input& input = APPLICATION.GetInput();

		if (TryChangeHibiki(context))
			return;

		if (input.IsMouseDown(MouseButton::Left))
		{
			context.stateMachine->ChangeState(ChiStateId::AttackWeak0);
			return;
		}

		if (input.IsMouseDown(MouseButton::Right))
		{
			context.stateMachine->ChangeState(ChiStateId::AttackStrong0_0);
			return;
		}

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

		if (IsMoveInputPressed(input))
			context.stateMachine->ChangeState(ChiStateId::Run);
	}

	/// Run //////////////////////////////////////////////////////////////////////////////
	void ChiRunState::Enter(ChiStateContext& context)
	{
		PlayAnimation(context, ChiAnimationId::RunFront, true);
	}

	void ChiRunState::Tick(ChiStateContext& context, float deltaTime)
	{
		const Input& input = APPLICATION.GetInput();

		if (TryChangeHibiki(context))
			return;

		if (input.IsMouseDown(MouseButton::Left))
		{
			context.stateMachine->ChangeState(ChiStateId::AttackWeak0);
			return;
		}

		if (input.IsMouseDown(MouseButton::Right))
		{
			context.stateMachine->ChangeState(ChiStateId::AttackStrong0_0);
			return;
		}

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

		if (context.moveComponent->IsMoving() == false)
			context.stateMachine->ChangeState(ChiStateId::Idle);
	}
}
