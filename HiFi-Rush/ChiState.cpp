#include "ChiState.h"
#include "Application.h"
#include "ChiMoveComponent.h"
#include "ChiStateMachineComponent.h"
#include "Input.h"
#include "SkeletalAnimatorComponent.h"

#include <cmath>

namespace gm
{
	namespace
	{
		AnimationPlayOption MakePlayOption(bool isLoop)
		{
			AnimationPlayOption playOption{};
			playOption.loopOverride = isLoop;
			playOption.blendDuration = ChiDefaultBlendDuration;
			return playOption;
		}

		bool IsMoveInputPressed(const Input& input)
		{
			return input.IsKeyRepeat(KeyCode::W) || input.IsKeyRepeat(KeyCode::A) || input.IsKeyRepeat(KeyCode::S) || input.IsKeyRepeat(KeyCode::D);
		}

		bool IsMovementLockedState(ChiStateId stateId)
		{
			switch (stateId)
			{
			case ChiStateId::AttackWeakDash:
			case ChiStateId::AttackStrongDash:
			case ChiStateId::AttackWeak0:
			case ChiStateId::AttackWeak1:
			case ChiStateId::AttackWeak2:
			case ChiStateId::AttackWeak3:
			case ChiStateId::AttackStrong0_0:
			case ChiStateId::AttackStrong0_1:
			case ChiStateId::AttackStrong1:
			case ChiStateId::AttackStrong2:
			case ChiStateId::AttackStrongToWeak1:
			case ChiStateId::AttackStrongToWeak2:
			case ChiStateId::AttackWeakToStrong1:
			case ChiStateId::AttackWeakToStrong2:
			case ChiStateId::AttackDelayedWeak1:
			case ChiStateId::AttackDelayedWeak2:
			case ChiStateId::AttackStump0:
			case ChiStateId::AttackStump1:
			case ChiStateId::AttackStump2:
			case ChiStateId::AttackSky0:
			case ChiStateId::AttackSky1:
			case ChiStateId::AttackSky2:
			case ChiStateId::AttackSky3:
			case ChiStateId::DamageStrongKnockback:
			case ChiStateId::DamageWeakKnockback:
			case ChiStateId::DamageDead:
			case ChiStateId::HibikiReady:
			case ChiStateId::HibikiAttack:
				return true;
			default:
				return false;
			}
		}

		bool UsesRootMotionState(ChiStateId stateId)
		{
			switch (stateId)
			{
			case ChiStateId::DashFront:
			case ChiStateId::DashBack:
			case ChiStateId::DashLeft:
			case ChiStateId::DashRight:
			case ChiStateId::DashDouble:
			case ChiStateId::DashTriple:
			case ChiStateId::DashSky:
			case ChiStateId::DashSkyFall:
			case ChiStateId::AttackWeakDash:
			case ChiStateId::AttackStrongDash:
			case ChiStateId::AttackWeak0:
			case ChiStateId::AttackWeak1:
			case ChiStateId::AttackWeak2:
			case ChiStateId::AttackWeak3:
			case ChiStateId::AttackStrong0_0:
			case ChiStateId::AttackStrong0_1:
			case ChiStateId::AttackStrong1:
			case ChiStateId::AttackStrong2:
			case ChiStateId::AttackStrongToWeak1:
			case ChiStateId::AttackStrongToWeak2:
			case ChiStateId::AttackWeakToStrong1:
			case ChiStateId::AttackWeakToStrong2:
			case ChiStateId::AttackDelayedWeak1:
			case ChiStateId::AttackDelayedWeak2:
			case ChiStateId::AttackStump0:
			case ChiStateId::AttackStump1:
			case ChiStateId::AttackStump2:
			case ChiStateId::AttackSky0:
			case ChiStateId::AttackSky1:
			case ChiStateId::AttackSky2:
			case ChiStateId::AttackSky3:
			case ChiStateId::DamageStrongKnockback:
			case ChiStateId::DamageWeakKnockback:
			case ChiStateId::HibikiReady:
			case ChiStateId::HibikiAttack:
				return true;
			default:
				return false;
			}
		}
	}

	void ChiState::PlayAnimation(ChiStateContext& context, ChiAnimationId animationId, bool isLoop) const
	{
		PlayAnimation(context, animationId, MakePlayOption(isLoop));
	}

	void ChiState::PlayAnimation(ChiStateContext& context, ChiAnimationId animationId, const AnimationPlayOption& playOption) const
	{
		context.animatorComponent->Play(GetChiAnimationName(animationId), playOption);
	}

	void ChiState::ReturnToIdleOrRun(ChiStateContext& context) const
	{
		if (IsMoveInputPressed(APPLICATION.GetInput()))
			context.stateMachine->ChangeState(ChiStateId::Run);
		else
			context.stateMachine->ChangeState(ChiStateId::Idle);
	}

	void ChiState::OnGroundContact(ChiStateContext& context, const NavigationGroundContactEvent&)
	{
		context.stateMachine->ChangeState(ChiStateId::JumpLanding);
	}

	void ChiState::OnGroundLost(ChiStateContext& context, const NavigationGroundLostEvent&)
	{
		context.stateMachine->ChangeState(ChiStateId::JumpDown);
	}

	bool ChiState::IsAnimationCompleted(const ChiStateContext& context) const
	{
		return context.animatorComponent->GetState() == AnimationState::Completed;
	}

	void ChiState::ChangeDashStateByInput(ChiStateContext& context) const
	{
		const Vector3 inputDirection = context.moveComponent->GetInputMoveDirection();
		if (inputDirection.LengthSquared() <= 0.f)
		{
			context.stateMachine->ChangeState(ChiStateId::DashFront);
			return;
		}

		const float forwardAmount = inputDirection.Dot(context.moveComponent->GetForwardDirection());
		const float rightAmount = inputDirection.Dot(context.moveComponent->GetRightDirection());
		if (std::abs(rightAmount) > std::abs(forwardAmount))
		{
			context.stateMachine->ChangeState(rightAmount > 0.f ? ChiStateId::DashRight : ChiStateId::DashLeft);
			return;
		}

		context.stateMachine->ChangeState(forwardAmount < 0.f ? ChiStateId::DashBack : ChiStateId::DashFront);
	}

	bool ChiState::TryChangeAirAction(ChiStateContext& context, bool canDoubleJump) const
	{
		const Input& input = APPLICATION.GetInput();
		if (input.IsKeyDown(KeyCode::LeftShift))
		{
			context.stateMachine->ChangeState(ChiStateId::DashSky);
			return true;
		}

		if (input.IsMouseDown(MouseButton::Right))
		{
			context.stateMachine->ChangeState(ChiStateId::AttackStump0);
			return true;
		}

		if (input.IsMouseDown(MouseButton::Left))
		{
			context.stateMachine->ChangeState(ChiStateId::AttackSky0);
			return true;
		}

		if (canDoubleJump && input.IsKeyDown(KeyCode::Space))
		{
			context.stateMachine->ChangeState(ChiStateId::JumpDoubleUp);
			return true;
		}

		return false;
	}

	bool ChiState::TryChangeHibiki(ChiStateContext& context) const
	{
		if (APPLICATION.GetInput().IsKeyDown(KeyCode::R) == false)
			return false;

		context.stateMachine->ChangeState(ChiStateId::HibikiReady);
		return true;
	}

	/// Clip //////////////////////////////////////////////////////////////////////////////
	ChiClipState::ChiClipState(ChiStateId stateId, ChiAnimationId animationId, bool isLoop)
		: ChiClipState(stateId, animationId, MakePlayOption(isLoop))
	{}

	ChiClipState::ChiClipState(ChiStateId stateId, ChiAnimationId animationId, const AnimationPlayOption& playOption)
		: _stateId(stateId)
		, _animationId(animationId)
		, _playOption(playOption)
	{}

	void ChiClipState::Enter(ChiStateContext& context)
	{
		PlayAnimation(context, _animationId, _playOption);

		_enabledRootMotionOnEnter = UsesRootMotionState(_stateId);
		if (_enabledRootMotionOnEnter)
		{
			_prevRootMotionEnabled = context.moveComponent->IsRootMotionEnabled();
			context.moveComponent->SetRootMotionEnabled(true);
		}

		_disabledMoveOnEnter = IsMovementLockedState(_stateId);
		if (_disabledMoveOnEnter)
		{
			_prevMoveEnabled = context.moveComponent->IsMoveEnabled();
			context.moveComponent->SetMoveEnabled(false);
		}
	}

	void ChiClipState::Tick(ChiStateContext& context, float deltaTime)
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

		if (input.IsMouseDown(MouseButton::Left))
		{
			switch (_stateId)
			{
			case ChiStateId::AttackSky0:
				context.stateMachine->ChangeState(ChiStateId::AttackSky1);
				return;
			case ChiStateId::AttackSky1:
				context.stateMachine->ChangeState(ChiStateId::AttackSky2);
				return;
			case ChiStateId::AttackSky2:
				context.stateMachine->ChangeState(ChiStateId::AttackSky3);
				return;
			default:
				break;
			}
		}

		if (input.IsMouseDown(MouseButton::Right))
		{
			switch (_stateId)
			{
			case ChiStateId::AttackStump0:
				context.stateMachine->ChangeState(ChiStateId::AttackStump1);
				return;
			case ChiStateId::AttackStump1:
				context.stateMachine->ChangeState(ChiStateId::AttackStump2);
				return;
			default:
				break;
			}
		}

		if (context.animatorComponent->IsLoop() == false && IsAnimationCompleted(context))
			ReturnToIdleOrRun(context);
	}

	void ChiClipState::Exit(ChiStateContext& context)
	{
		if (_enabledRootMotionOnEnter)
		{
			context.moveComponent->SetRootMotionEnabled(_prevRootMotionEnabled);
			_enabledRootMotionOnEnter = false;
		}

		if (_disabledMoveOnEnter)
		{
			context.moveComponent->SetMoveEnabled(_prevMoveEnabled);
			_disabledMoveOnEnter = false;
		}
	}
}
