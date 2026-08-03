#include "ChiDashState.h"
#include "Application.h"
#include "ChiMoveComponent.h"
#include "ChiStateMachineComponent.h"
#include "Input.h"

namespace gm
{
	/// Dash //////////////////////////////////////////////////////////////////////////////
	ChiDashState::ChiDashState(ChiStateId stateId, ChiAnimationId animationId, ChiDashDirection direction, float dashSpeed, bool rotateToDashDirection, ChiStateId nextDashState)
		: ChiClipState(stateId, animationId)
		, _direction(direction)
		, _nextDashState(nextDashState)
		, _dashSpeed(dashSpeed)
		, _rotateToDashDirection(rotateToDashDirection)
	{
	}

	void ChiDashState::Enter(ChiStateContext& context)
	{
		ChiClipState::Enter(context);

		_prevMoveEnabled = context.moveComponent->IsMoveEnabled();
		context.moveComponent->SetMoveEnabled(false);

		_cachedDirection = GetDashDirection(context);
		if (_rotateToDashDirection)
			context.moveComponent->FaceDirectionImmediate(_cachedDirection);
	}

	void ChiDashState::Tick(ChiStateContext& context, float deltaTime)
	{
		context.moveComponent->MoveAlong(_cachedDirection, _dashSpeed, deltaTime, _rotateToDashDirection);

		if (TryChangeDashAttack(context))
			return;

		const Input& input = APPLICATION.GetInput();
		if (input.IsKeyDown(KeyCode::Space))
		{
			context.stateMachine->ChangeState(ChiStateId::JumpUp);
			return;
		}

		if (_nextDashState != ChiStateId::None && input.IsKeyDown(KeyCode::LeftShift))
		{
			context.stateMachine->ChangeState(_nextDashState);
			return;
		}

		if (IsAnimationCompleted(context))
			ReturnToIdleOrRun(context);
	}

	void ChiDashState::Exit(ChiStateContext& context)
	{
		ChiClipState::Exit(context);
		context.moveComponent->SetMoveEnabled(_prevMoveEnabled);
	}

	bool ChiDashState::TryChangeDashAttack(ChiStateContext& context) const
	{
		const Input& input = APPLICATION.GetInput();
		if (input.IsMouseDown(MouseButton::Left))
		{
			context.moveComponent->FaceDirectionImmediate(_cachedDirection);
			context.stateMachine->ChangeState(ChiStateId::AttackWeakDash);
			return true;
		}

		if (input.IsMouseDown(MouseButton::Right))
		{
			context.moveComponent->FaceDirectionImmediate(_cachedDirection);
			context.stateMachine->ChangeState(ChiStateId::AttackStrongDash);
			return true;
		}

		return false;
	}

	Vector3 ChiDashState::GetDashDirection(ChiStateContext& context) const
	{
		switch (_direction)
		{
		case ChiDashDirection::Front:
			return context.moveComponent->GetForwardDirection();
		case ChiDashDirection::Back:
			return -context.moveComponent->GetForwardDirection();
		case ChiDashDirection::Left:
			return -context.moveComponent->GetRightDirection();
		case ChiDashDirection::Right:
			return context.moveComponent->GetRightDirection();
		case ChiDashDirection::InputOrFront:
		{
			const Vector3 inputDirection = context.moveComponent->GetInputMoveDirection();
			if (inputDirection.LengthSquared() > 0.f)
				return inputDirection;

			return context.moveComponent->GetForwardDirection();
		}
		default:
			GM_ASSERT_RETURN_VAL(false, Vector3(0.f, 0.f, 1.f), "지원하지 않는 ChiDashDirection입니다.");
		}
	}

	ChiDashFrontState::ChiDashFrontState()
		: ChiDashState(ChiStateId::DashFront, ChiAnimationId::DashFront, ChiDashDirection::Front, 0.f, false, ChiStateId::DashDouble)
	{
	}

	ChiDashBackState::ChiDashBackState()
		: ChiDashState(ChiStateId::DashBack, ChiAnimationId::DashBack, ChiDashDirection::Back, 0.f, false, ChiStateId::DashDouble)
	{
	}

	ChiDashLeftState::ChiDashLeftState()
		: ChiDashState(ChiStateId::DashLeft, ChiAnimationId::DashLeft, ChiDashDirection::Left, 0.f, false, ChiStateId::DashDouble)
	{
	}

	ChiDashRightState::ChiDashRightState()
		: ChiDashState(ChiStateId::DashRight, ChiAnimationId::DashRight, ChiDashDirection::Right, 0.f, false, ChiStateId::DashDouble)
	{
	}

	ChiDashDoubleState::ChiDashDoubleState()
		: ChiDashState(ChiStateId::DashDouble, ChiAnimationId::DashDouble, ChiDashDirection::InputOrFront, 0.f, true, ChiStateId::DashTriple)
	{
	}

	ChiDashTripleState::ChiDashTripleState()
		: ChiDashState(ChiStateId::DashTriple, ChiAnimationId::DashTriple, ChiDashDirection::InputOrFront, 0.f, true)
	{
	}

	/// DashSky //////////////////////////////////////////////////////////////////////////////
	ChiDashSkyState::ChiDashSkyState()
		: ChiDashState(ChiStateId::DashSky, ChiAnimationId::DashSky, ChiDashDirection::InputOrFront, 0.f, true)
	{
	}

	void ChiDashSkyState::Tick(ChiStateContext& context, float deltaTime)
	{
		context.moveComponent->MoveAlong(GetCachedDirection(), GetDashSpeed(), deltaTime, true);

		const Input& input = APPLICATION.GetInput();
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

		if (IsAnimationCompleted(context))
			context.stateMachine->ChangeState(ChiStateId::DashSkyFall);
	}

	/// DashSkyFall //////////////////////////////////////////////////////////////////////////////
	ChiDashSkyFallState::ChiDashSkyFallState()
		: ChiClipState(ChiStateId::DashSkyFall, ChiAnimationId::DashSkyFall)
	{
	}

	void ChiDashSkyFallState::Tick(ChiStateContext& context, float deltaTime)
	{
		const Input& input = APPLICATION.GetInput();
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
	}
}
