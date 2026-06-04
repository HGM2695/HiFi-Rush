#include "ChiState.h"
#include "Application.h"
#include "ChiStateMachineComponent.h"
#include "Input.h"
#include "ChiMoveComponent.h"
#include "SkeletalAnimatorComponent.h"

#include <cmath>

namespace gm
{
	namespace
	{
		constexpr float ChiDefaultBlendDuration = 0.15f;

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

		void ChangeDashStateByInput(ChiStateContext& context)
		{
			const Vector3 inputDirection = context.moveComponent->GetInputMoveDirection();
			if (inputDirection.LengthSquared() <= 0.f)
			{
				context.stateMachine->ChangeState(ChiStateId::DashFront);
				return;
			}

			if (std::abs(inputDirection.x) > std::abs(inputDirection.z))
			{
				context.stateMachine->ChangeState(inputDirection.x > 0.f ? ChiStateId::DashRight : ChiStateId::DashLeft);
				return;
			}

			context.stateMachine->ChangeState(inputDirection.z < 0.f ? ChiStateId::DashBack : ChiStateId::DashFront);
		}

		bool TryChangeAirAction(ChiStateContext& context, bool canDoubleJump)
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

		bool TryChangeHibiki(ChiStateContext& context)
		{
			if (APPLICATION.GetInput().IsKeyDown(KeyCode::R) == false)
				return false;

			context.stateMachine->ChangeState(ChiStateId::HibikiReady);
			return true;
		}
	}

	void ChiState::PlayAnimation(ChiStateContext& context, ChiAnimationId animationId, bool isLoop) const
	{
		context.animatorComponent->Play(GetChiAnimationName(animationId), MakePlayOption(isLoop));
	}

	void ChiState::ReturnToIdleOrRun(ChiStateContext& context) const
	{
		if (IsMoveInputPressed(APPLICATION.GetInput()))
			context.stateMachine->ChangeState(ChiStateId::Run);
		else
			context.stateMachine->ChangeState(ChiStateId::Idle);
	}

	bool ChiState::IsAnimationCompleted(const ChiStateContext& context) const
	{
		return context.animatorComponent->GetState() == AnimationState::Completed;
	}

	/// Clip //////////////////////////////////////////////////////////////////////////////
	ChiClipState::ChiClipState(ChiStateId stateId, ChiAnimationId animationId, bool isLoop)
		: _stateId(stateId)
		, _animationId(animationId)
		, _isLoop(isLoop)
	{}

	void ChiClipState::Enter(ChiStateContext& context)
	{
		PlayAnimation(context, _animationId, _isLoop);

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

		if (_isLoop == false && IsAnimationCompleted(context))
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

		if (IsAnimationCompleted(context))
			context.stateMachine->ChangeState(ChiStateId::JumpLanding);
	}

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

	/// Damage //////////////////////////////////////////////////////////////////////////////
	ChiDamageState::ChiDamageState(ChiStateId stateId, ChiAnimationId animationId, bool returnToBaseMotion)
		: ChiClipState(stateId, animationId)
		, _returnToBaseMotion(returnToBaseMotion)
	{
	}

	void ChiDamageState::Tick(ChiStateContext& context, float deltaTime)
	{
		if (IsAnimationCompleted(context) == false)
			return;

		if (_returnToBaseMotion)
			ReturnToIdleOrRun(context);
	}

	ChiWeakKnockbackDamageState::ChiWeakKnockbackDamageState()
		: ChiDamageState(ChiStateId::DamageWeakKnockback, ChiAnimationId::DamageWeakKnockback, true)
	{
	}

	ChiStrongKnockbackDamageState::ChiStrongKnockbackDamageState()
		: ChiDamageState(ChiStateId::DamageStrongKnockback, ChiAnimationId::DamageStrongKnockback, true)
	{
	}

	ChiDeadDamageState::ChiDeadDamageState()
		: ChiDamageState(ChiStateId::DamageDead, ChiAnimationId::DamageDead, false)
	{
	}

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
