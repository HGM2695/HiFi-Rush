#include "ChiDashState.h"
#include "Application.h"
#include "BeatSystem.h"
#include "ChiAnimationSettings.h"
#include "ChiMoveComponent.h"
#include "ChiStateMachineComponent.h"
#include "Input.h"

namespace gm
{
	namespace
	{
		constexpr float DashChainInputStartBeat = 0.5f;
	}

	/// Dash //////////////////////////////////////////////////////////////////////////////
	ChiDashState::ChiDashState(ChiStateId stateId, ChiAnimationClipId animationClipId, ChiDashDirection direction, float dashSpeed, bool rotateToDashDirection, ChiStateId nextDashState)
		: ChiState(stateId, animationClipId), _direction(direction), _nextDashState(nextDashState), _dashSpeed(dashSpeed), _rotateToDashDirection(rotateToDashDirection)
	{
	}

	void ChiDashState::Enter(ChiStateContext& context)
	{
		ChiState::Enter(context);
		_bufferedAttackInput.reset();

		_cachedDirection = GetDashDirection(context);
		if (_rotateToDashDirection)
			context.moveComponent->FaceDirectionImmediate(_cachedDirection);
	}

	void ChiDashState::Tick(ChiStateContext& context, float deltaTime)
	{
		context.moveComponent->MoveAlong(_cachedDirection, _dashSpeed, deltaTime, _rotateToDashDirection);

		if (TryChangeDashAttack(context))
			return;

		if (context.jumpInput)
		{
			context.stateMachine->ChangeState(ChiStateId::JumpUp, context.jumpInput.value());
			return;
		}

		if (_nextDashState != ChiStateId::None && context.dashInput)
		{
			const bool isOnBeat = context.dashInput->judgeGrade != RhythmJudgeGrade::OffBeat;
			if (GetStateElapsedBeat(context) > DashChainInputStartBeat && isOnBeat)
			{
				context.stateMachine->ChangeState(_nextDashState, context.dashInput.value());
				return;
			}
		}

		if (IsAnimationCompleted(context))
			ReturnToIdleOrRun(context);
	}

	bool ChiDashState::TryChangeDashAttack(ChiStateContext& context)
	{
		if (_bufferedAttackInput.has_value() == false)
		{
			if (context.weakAttackInput)
				_bufferedAttackInput = context.weakAttackInput;
			else if (context.strongAttackInput)
				_bufferedAttackInput = context.strongAttackInput;
		}

		if (IsBlendCompleted(context) == false)
			return false;

		const float elapsedBeat = GetElapsedBeatAfterBlend(context);
		if (_bufferedAttackInput && _bufferedAttackInput->type == RhythmInputType::WeakAttack)
		{
			context.moveComponent->FaceDirectionImmediate(_cachedDirection);
			context.stateMachine->ChangeState(elapsedBeat < 0.6f ? ChiStateId::AttackWeakDash : ChiStateId::AttackWeak0, _bufferedAttackInput.value());
			return true;
		}

		if (_bufferedAttackInput && _bufferedAttackInput->type == RhythmInputType::StrongAttack)
		{
			context.moveComponent->FaceDirectionImmediate(_cachedDirection);
			if (elapsedBeat < 0.6f)
			{
				if (_direction == ChiDashDirection::Front)
					context.stateMachine->ChangeState(ChiStateId::AttackStrongDash, 0.f, _bufferedAttackInput.value());
				else
					context.stateMachine->ChangeState(ChiStateId::AttackStrongDash, _bufferedAttackInput.value());
			}
			else
			{
				context.stateMachine->ChangeState(ChiStateId::AttackStrong0_0, _bufferedAttackInput.value());
			}
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
		: ChiDashState(ChiStateId::DashFront, ChiAnimationClipId::DashFront, ChiDashDirection::Front, 0.f, false, ChiStateId::DashDouble)
	{
	}

	ChiDashBackState::ChiDashBackState()
		: ChiDashState(ChiStateId::DashBack, ChiAnimationClipId::DashBack, ChiDashDirection::Back, 0.f, false, ChiStateId::DashDouble)
	{
	}

	ChiDashLeftState::ChiDashLeftState()
		: ChiDashState(ChiStateId::DashLeft, ChiAnimationClipId::DashLeft, ChiDashDirection::Left, 0.f, false, ChiStateId::DashDouble)
	{
	}

	ChiDashRightState::ChiDashRightState()
		: ChiDashState(ChiStateId::DashRight, ChiAnimationClipId::DashRight, ChiDashDirection::Right, 0.f, false, ChiStateId::DashDouble)
	{
	}

	ChiDashDoubleState::ChiDashDoubleState()
		: ChiDashState(ChiStateId::DashDouble, ChiAnimationClipId::DashDouble, ChiDashDirection::InputOrFront, 0.f, true, ChiStateId::DashTriple)
	{
	}

	ChiDashTripleState::ChiDashTripleState()
		: ChiDashState(ChiStateId::DashTriple, ChiAnimationClipId::DashTriple, ChiDashDirection::InputOrFront, 0.f, true)
	{
	}

	/// DashSky //////////////////////////////////////////////////////////////////////////////
	ChiDashSkyState::ChiDashSkyState()
		: ChiDashState(ChiStateId::DashSky, ChiAnimationClipId::DashSky, ChiDashDirection::InputOrFront, 0.f, true)
	{
	}

	void ChiDashSkyState::Tick(ChiStateContext& context, float deltaTime)
	{
		context.moveComponent->MoveAlong(GetCachedDirection(), GetDashSpeed(), deltaTime, true);

		if (GetElapsedBeatAfterBlend(context) > 0.5f)
		{
			const Input& input = APPLICATION.GetInput();
			if (context.strongAttackInput)
			{
				context.stateMachine->ChangeState(ChiStateId::AttackStump0, context.strongAttackInput.value());
				return;
			}
			if (input.IsMouseRepeat(MouseButton::Left))
			{
				context.stateMachine->ChangeState(ChiStateId::AttackSky0);
				return;
			}
		}

		if (IsAnimationCompleted(context))
			context.stateMachine->ChangeState(ChiStateId::DashSkyFall);
	}

	/// DashSkyFall //////////////////////////////////////////////////////////////////////////////
	ChiDashSkyFallState::ChiDashSkyFallState()
		: ChiState(ChiStateId::DashSkyFall, ChiAnimationClipId::DashSkyFall)
	{
	}

	void ChiDashSkyFallState::Tick(ChiStateContext& context, float deltaTime)
	{
		if (GetStateElapsedBeat(context) > 0.1f && context.weakAttackInput)
		{
			context.stateMachine->ChangeState(ChiStateId::AttackSky0, context.weakAttackInput.value());
			return;
		}

		if (context.strongAttackInput)
		{
			context.stateMachine->ChangeState(ChiStateId::AttackStump0, context.strongAttackInput.value());
			return;
		}
	}
}
