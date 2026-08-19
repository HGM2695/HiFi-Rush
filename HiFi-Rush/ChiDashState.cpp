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
		constexpr float AnimationTicksPerBeat = 15.f;
		constexpr float DashChainInputStartBeat = 0.5f;
		constexpr float DashNormalActionStartBeat = 0.6f;
		constexpr float DirectionalDashRunTransitionBeat = 1.f + 7.5f / AnimationTicksPerBeat;
		constexpr float DoubleDashRunTransitionBeat = 1.5f;
		constexpr float TripleDashRunTransitionBeat = 1.f + 5.f / AnimationTicksPerBeat;
	}

	/// Dash //////////////////////////////////////////////////////////////////////////////
	ChiDashState::ChiDashState(ChiStateId stateId, ChiAnimationClipId animationClipId, ChiDashDirection direction, bool rotateToDashDirection, float runTransitionStartBeat, ChiStateId nextDashState)
		: ChiState(stateId, animationClipId), _direction(direction), _nextDashState(nextDashState), _rotateToDashDirection(rotateToDashDirection), _runTransitionStartBeat(runTransitionStartBeat)
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

	void ChiDashState::Tick(ChiStateContext& context, float)
	{
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
		else if (_nextDashState == ChiStateId::None && context.dashInput)
		{
			const bool isOnBeat = context.dashInput->judgeGrade != RhythmJudgeGrade::OffBeat;
			if (IsBlendCompleted(context) && GetElapsedBeatAfterBlend(context) >= DashNormalActionStartBeat && isOnBeat)
			{
				ChangeDashStateByInput(context, &context.dashInput.value());
				return;
			}
		}

		if (GetStateElapsedBeat(context) >= _runTransitionStartBeat && IsMoveInputPressed(context))
		{
			context.stateMachine->ChangeState(ChiStateId::Run);
			return;
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
			context.stateMachine->ChangeState(elapsedBeat < DashNormalActionStartBeat ? ChiStateId::AttackWeakDash : ChiStateId::AttackWeak0, _bufferedAttackInput.value());
			return true;
		}

		if (_bufferedAttackInput && _bufferedAttackInput->type == RhythmInputType::StrongAttack)
		{
			context.moveComponent->FaceDirectionImmediate(_cachedDirection);
			if (elapsedBeat < DashNormalActionStartBeat)
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
		: ChiDashState(ChiStateId::DashFront, ChiAnimationClipId::DashFront, ChiDashDirection::Front, false, DirectionalDashRunTransitionBeat, ChiStateId::DashDouble)
	{
	}

	ChiDashBackState::ChiDashBackState()
		: ChiDashState(ChiStateId::DashBack, ChiAnimationClipId::DashBack, ChiDashDirection::Back, false, DirectionalDashRunTransitionBeat, ChiStateId::DashDouble)
	{
	}

	ChiDashLeftState::ChiDashLeftState()
		: ChiDashState(ChiStateId::DashLeft, ChiAnimationClipId::DashLeft, ChiDashDirection::Left, false, DirectionalDashRunTransitionBeat, ChiStateId::DashDouble)
	{
	}

	ChiDashRightState::ChiDashRightState()
		: ChiDashState(ChiStateId::DashRight, ChiAnimationClipId::DashRight, ChiDashDirection::Right, false, DirectionalDashRunTransitionBeat, ChiStateId::DashDouble)
	{
	}

	ChiDashDoubleState::ChiDashDoubleState()
		: ChiDashState(ChiStateId::DashDouble, ChiAnimationClipId::DashDouble, ChiDashDirection::InputOrFront, true, DoubleDashRunTransitionBeat, ChiStateId::DashTriple)
	{
	}

	ChiDashTripleState::ChiDashTripleState()
		: ChiDashState(ChiStateId::DashTriple, ChiAnimationClipId::DashTriple, ChiDashDirection::InputOrFront, true, TripleDashRunTransitionBeat)
	{
	}

	/// DashSky //////////////////////////////////////////////////////////////////////////////
	ChiDashSkyState::ChiDashSkyState()
		: ChiDashState(ChiStateId::DashSky, ChiAnimationClipId::DashSky, ChiDashDirection::InputOrFront, true, 0.f)
	{
	}

	void ChiDashSkyState::Tick(ChiStateContext& context, float)
	{
		if (GetElapsedBeatAfterBlend(context) > 0.5f)
		{
			const Input& input = APPLICATION.GetInput();
			if (context.strongAttackInput)
			{
				context.stateMachine->ChangeState(ChiStateId::AttackStump0, context.strongAttackInput.value());
				return;
			}
			if (context.stateMachine->IsInputEnabled() && input.IsMouseRepeat(MouseButton::Left))
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
