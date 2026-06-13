#include "ChiAttackState.h"
#include "Application.h"
#include "ChiStateMachineComponent.h"
#include "Input.h"

namespace gm
{
	/// WeakAttack //////////////////////////////////////////////////////////////////////////////
	ChiWeakAttackState::ChiWeakAttackState(ChiStateId stateId, ChiAnimationId animationId, ChiStateId nextWeakAttackState, ChiStateId rightAttackState)
		: ChiClipState(stateId, animationId)
		, _nextWeakAttackState(nextWeakAttackState)
		, _rightAttackState(rightAttackState)
	{
	}

	void ChiWeakAttackState::Tick(ChiStateContext& context, float deltaTime)
	{
		const Input& input = APPLICATION.GetInput();

		if (_nextWeakAttackState != ChiStateId::None && input.IsMouseDown(MouseButton::Left))
		{
			context.stateMachine->ChangeState(_nextWeakAttackState);
			return;
		}

		if (_rightAttackState != ChiStateId::None && input.IsMouseDown(MouseButton::Right))
		{
			context.stateMachine->ChangeState(_rightAttackState);
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

		ChiClipState::Tick(context, deltaTime);
	}

	ChiWeak0AttackState::ChiWeak0AttackState()
		: ChiWeakAttackState(ChiStateId::AttackWeak0, ChiAnimationId::AttackWeak0, ChiStateId::AttackWeak1, ChiStateId::AttackWeakToStrong1)
	{
	}

	ChiWeak1AttackState::ChiWeak1AttackState()
		: ChiWeakAttackState(ChiStateId::AttackWeak1, ChiAnimationId::AttackWeak1, ChiStateId::AttackWeak2)
	{
	}

	ChiWeak2AttackState::ChiWeak2AttackState()
		: ChiWeakAttackState(ChiStateId::AttackWeak2, ChiAnimationId::AttackWeak2, ChiStateId::AttackWeak3)
	{
	}

	ChiWeak3AttackState::ChiWeak3AttackState()
		: ChiWeakAttackState(ChiStateId::AttackWeak3, ChiAnimationId::AttackWeak3)
	{
	}

	ChiWeakDashAttackState::ChiWeakDashAttackState()
		: ChiClipState(ChiStateId::AttackWeakDash, ChiAnimationId::AttackWeakDash)
	{
	}

	void ChiWeakDashAttackState::Tick(ChiStateContext& context, float deltaTime)
	{
		const Input& input = APPLICATION.GetInput();

		if (input.IsMouseDown(MouseButton::Left))
		{
			context.stateMachine->ChangeState(ChiStateId::AttackDelayedWeak1);
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

		if (IsAnimationCompleted(context))
			ReturnToIdleOrRun(context);
	}

	/// StrongAttack //////////////////////////////////////////////////////////////////////////////
	ChiStrongAttackState::ChiStrongAttackState(ChiStateId stateId, ChiAnimationId animationId, ChiStateId nextStrongAttackState, ChiStateId leftAttackState, ChiStateId autoNextState)
		: ChiClipState(stateId, animationId)
		, _nextStrongAttackState(nextStrongAttackState)
		, _leftAttackState(leftAttackState)
		, _autoNextState(autoNextState)
	{
	}

	void ChiStrongAttackState::Tick(ChiStateContext& context, float deltaTime)
	{
		const Input& input = APPLICATION.GetInput();

		if (_nextStrongAttackState != ChiStateId::None && input.IsMouseDown(MouseButton::Right))
		{
			context.stateMachine->ChangeState(_nextStrongAttackState);
			return;
		}

		if (_leftAttackState != ChiStateId::None && input.IsMouseDown(MouseButton::Left))
		{
			context.stateMachine->ChangeState(_leftAttackState);
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

		if (IsAnimationCompleted(context))
		{
			if (_autoNextState != ChiStateId::None)
				context.stateMachine->ChangeState(_autoNextState);
			else
				ReturnToIdleOrRun(context);
		}
	}

	ChiStrong0_0AttackState::ChiStrong0_0AttackState()
		: ChiStrongAttackState(ChiStateId::AttackStrong0_0, ChiAnimationId::AttackStrong0_0, ChiStateId::None, ChiStateId::None, ChiStateId::AttackStrong0_1)
	{
	}

	ChiStrong0_1AttackState::ChiStrong0_1AttackState()
		: ChiStrongAttackState(ChiStateId::AttackStrong0_1, ChiAnimationId::AttackStrong0_1, ChiStateId::AttackStrong1, ChiStateId::AttackStrongToWeak1)
	{
	}

	ChiStrong1AttackState::ChiStrong1AttackState()
		: ChiStrongAttackState(ChiStateId::AttackStrong1, ChiAnimationId::AttackStrong1, ChiStateId::AttackStrong2)
	{
	}

	ChiStrong2AttackState::ChiStrong2AttackState()
		: ChiStrongAttackState(ChiStateId::AttackStrong2, ChiAnimationId::AttackStrong2)
	{
	}

	ChiStrongDashAttackState::ChiStrongDashAttackState()
		: ChiClipState(ChiStateId::AttackStrongDash, ChiAnimationId::AttackStrongDash)
	{
	}

	void ChiStrongDashAttackState::Tick(ChiStateContext& context, float deltaTime)
	{
		const Input& input = APPLICATION.GetInput();

		if (input.IsKeyDown(KeyCode::LeftShift))
		{
			context.stateMachine->ChangeState(ChiStateId::DashSky);
			return;
		}

		if (input.IsMouseDown(MouseButton::Left))
		{
			context.stateMachine->ChangeState(ChiStateId::AttackSky0);
			return;
		}

		if (input.IsMouseDown(MouseButton::Right))
		{
			context.stateMachine->ChangeState(ChiStateId::AttackStump0);
			return;
		}

		if (input.IsKeyDown(KeyCode::Space))
		{
			context.stateMachine->ChangeState(ChiStateId::JumpUp);
			return;
		}

		if (IsAnimationCompleted(context))
			context.stateMachine->ChangeState(ChiStateId::JumpDown);
	}

	/// BranchAttack //////////////////////////////////////////////////////////////////////////////
	ChiBranchAttackState::ChiBranchAttackState(ChiStateId stateId, ChiAnimationId animationId, ChiStateId leftAttackState, ChiStateId rightAttackState, ChiStateId autoNextState)
		: ChiClipState(stateId, animationId)
		, _leftAttackState(leftAttackState)
		, _rightAttackState(rightAttackState)
		, _autoNextState(autoNextState)
	{
	}

	void ChiBranchAttackState::Tick(ChiStateContext& context, float deltaTime)
	{
		const Input& input = APPLICATION.GetInput();

		if (_leftAttackState != ChiStateId::None && input.IsMouseDown(MouseButton::Left))
		{
			context.stateMachine->ChangeState(_leftAttackState);
			return;
		}

		if (_rightAttackState != ChiStateId::None && input.IsMouseDown(MouseButton::Right))
		{
			context.stateMachine->ChangeState(_rightAttackState);
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

		if (IsAnimationCompleted(context))
		{
			if (_autoNextState != ChiStateId::None)
				context.stateMachine->ChangeState(_autoNextState);
			else
				ReturnToIdleOrRun(context);
		}
	}

	ChiStrongToWeak1AttackState::ChiStrongToWeak1AttackState()
		: ChiBranchAttackState(ChiStateId::AttackStrongToWeak1, ChiAnimationId::AttackStrongToWeak1, ChiStateId::AttackStrongToWeak2, ChiStateId::AttackStrongToWeak2)
	{
	}

	ChiStrongToWeak2AttackState::ChiStrongToWeak2AttackState()
		: ChiBranchAttackState(ChiStateId::AttackStrongToWeak2, ChiAnimationId::AttackStrongToWeak2)
	{
	}

	ChiWeakToStrong1AttackState::ChiWeakToStrong1AttackState()
		: ChiBranchAttackState(ChiStateId::AttackWeakToStrong1, ChiAnimationId::AttackWeakToStrong1, ChiStateId::AttackWeakToStrong2, ChiStateId::AttackWeakToStrong2)
	{
	}

	ChiWeakToStrong2AttackState::ChiWeakToStrong2AttackState()
		: ChiBranchAttackState(ChiStateId::AttackWeakToStrong2, ChiAnimationId::AttackWeakToStrong2)
	{
	}

	ChiDelayedWeak1AttackState::ChiDelayedWeak1AttackState()
		: ChiBranchAttackState(ChiStateId::AttackDelayedWeak1, ChiAnimationId::AttackDelayedWeak1, ChiStateId::AttackDelayedWeak2)
	{
	}

	ChiDelayedWeak2AttackState::ChiDelayedWeak2AttackState()
		: ChiBranchAttackState(ChiStateId::AttackDelayedWeak2, ChiAnimationId::AttackDelayedWeak2, ChiStateId::AttackSky0, ChiStateId::AttackStump0, ChiStateId::JumpDown)
	{
	}

	/// AirAttack //////////////////////////////////////////////////////////////////////////////
	ChiAirAttackState::ChiAirAttackState(ChiStateId stateId, ChiAnimationId animationId, ChiStateId nextAirAttackState)
		: ChiClipState(stateId, animationId)
		, _nextAirAttackState(nextAirAttackState)
	{
	}

	void ChiAirAttackState::Tick(ChiStateContext& context, float deltaTime)
	{
		const Input& input = APPLICATION.GetInput();

		if (input.IsKeyDown(KeyCode::LeftShift))
		{
			context.stateMachine->ChangeState(ChiStateId::DashSky);
			return;
		}

		if (input.IsMouseDown(MouseButton::Right))
		{
			context.stateMachine->ChangeState(ChiStateId::AttackStump0);
			return;
		}

		if (_nextAirAttackState != ChiStateId::None && input.IsMouseDown(MouseButton::Left))
		{
			context.stateMachine->ChangeState(_nextAirAttackState);
			return;
		}

		if (IsAnimationCompleted(context))
			context.stateMachine->ChangeState(ChiStateId::JumpDoubleDown);
	}

	ChiSky0AttackState::ChiSky0AttackState()
		: ChiAirAttackState(ChiStateId::AttackSky0, ChiAnimationId::AttackSky0, ChiStateId::AttackSky1)
	{
	}

	ChiSky1AttackState::ChiSky1AttackState()
		: ChiAirAttackState(ChiStateId::AttackSky1, ChiAnimationId::AttackSky1, ChiStateId::AttackSky2)
	{
	}

	ChiSky2AttackState::ChiSky2AttackState()
		: ChiAirAttackState(ChiStateId::AttackSky2, ChiAnimationId::AttackSky2, ChiStateId::AttackSky3)
	{
	}

	ChiSky3AttackState::ChiSky3AttackState()
		: ChiAirAttackState(ChiStateId::AttackSky3, ChiAnimationId::AttackSky3)
	{
	}

	/// StumpAttack //////////////////////////////////////////////////////////////////////////////
	ChiStump0AttackState::ChiStump0AttackState()
		: ChiClipState(ChiStateId::AttackStump0, ChiAnimationId::AttackStump0)
	{
	}

	void ChiStump0AttackState::Tick(ChiStateContext& context, float deltaTime)
	{
		if (IsAnimationCompleted(context))
			context.stateMachine->ChangeState(ChiStateId::AttackStump1);
	}

	ChiStump1AttackState::ChiStump1AttackState()
		: ChiClipState(ChiStateId::AttackStump1, ChiAnimationId::AttackStump1)
	{
	}

	void ChiStump1AttackState::Tick(ChiStateContext& context, float deltaTime)
	{
		// 착지 판정과 연결되면 이 상태에서 AttackStump2로 전환합니다.
		if (IsAnimationCompleted(context))
			context.stateMachine->ChangeState(ChiStateId::AttackStump2);
	}

	ChiStump2AttackState::ChiStump2AttackState()
		: ChiClipState(ChiStateId::AttackStump2, ChiAnimationId::AttackStump2)
	{
	}

	void ChiStump2AttackState::Tick(ChiStateContext& context, float deltaTime)
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
}
