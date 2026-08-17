#include "ChiJumpState.h"
#include "BeatSystem.h"
#include "ChiAnimationSettings.h"
#include "ChiStateMachineComponent.h"
#include "Rigidbody3DComponent.h"

#include <cmath>

namespace gm
{
	namespace
	{
		constexpr float JumpImpulse = 9.f;
		constexpr float DoubleJumpImpulse = 12.f;
		constexpr float JumpStartTime = 0.f;
		constexpr float DoubleJumpDownStartTime = 0.f;
		constexpr float JumpGravityScale = 3.f;
		constexpr float ApexGravityScale = 1.f;
		constexpr float ApexVelocityThreshold = 2.f;

		AnimationPlayOption MakeJumpPlayOption(bool isLoop, float startTime)
		{
			return AnimationPlayOption{ .startTime = startTime, .loopOverride = isLoop };
		}

		void ApplyVerticalImpulse(ChiStateContext& context, float impulse)
		{
			context.rigidbodyComponent->ClearVerticalVelocity();
			context.rigidbodyComponent->SetGravityScale(JumpGravityScale);
			context.rigidbodyComponent->AddImpulse(Vector3{ 0.f, impulse, 0.f });
		}

		void UpdateApexGravity(ChiStateContext& context)
		{
			const float verticalSpeed = std::abs(context.rigidbodyComponent->GetVelocity().y);
			context.rigidbodyComponent->SetGravityScale(verticalSpeed <= ApexVelocityThreshold ? ApexGravityScale : JumpGravityScale);
		}

		void RestoreJumpGravity(ChiStateContext& context)
		{
			context.rigidbodyComponent->SetGravityScale(JumpGravityScale);
		}

		bool HasLeftApex(const ChiStateContext& context)
		{
			return context.rigidbodyComponent->GetVelocity().y <= -ApexVelocityThreshold;
		}

	}

	/// Jump //////////////////////////////////////////////////////////////////////////////
	ChiJumpUpState::ChiJumpUpState()
		: ChiState(ChiStateId::JumpUp, ChiAnimationClipId::JumpUp, MakeJumpPlayOption(true, JumpStartTime))
	{
	}

	void ChiJumpUpState::Enter(ChiStateContext& context)
	{
		ChiState::Enter(context);
		ApplyVerticalImpulse(context, JumpImpulse);
	}

	void ChiJumpUpState::Tick(ChiStateContext& context, float deltaTime)
	{
		UpdateApexGravity(context);

		const float elapsedBeat = GetElapsedBeatAfterBlend(context);
		if (elapsedBeat > 0.2f && context.dashInput)
		{
			context.stateMachine->ChangeState(ChiStateId::DashSky, context.dashInput.value());
			return;
		}
		if (elapsedBeat > 0.2f && context.jumpInput)
		{
			context.stateMachine->ChangeState(ChiStateId::JumpDoubleUp, 0.f, context.jumpInput.value());
			return;
		}
		if (context.strongAttackInput)
		{
			context.stateMachine->ChangeState(ChiStateId::AttackStump0, context.strongAttackInput.value());
			return;
		}
		if (elapsedBeat > 0.1f && context.weakAttackInput)
		{
			context.stateMachine->ChangeState(ChiStateId::AttackSky0, context.weakAttackInput.value());
			return;
		}

		if (HasLeftApex(context))
			context.stateMachine->ChangeState(ChiStateId::JumpDown);
	}

	void ChiJumpUpState::Exit(ChiStateContext& context)
	{
		RestoreJumpGravity(context);
	}

	/// JumpDown //////////////////////////////////////////////////////////////////////////////
	ChiJumpDownState::ChiJumpDownState()
		: ChiState(ChiStateId::JumpDown, ChiAnimationClipId::JumpDown, MakeJumpPlayOption(true, JumpStartTime))
	{
	}

	void ChiJumpDownState::Tick(ChiStateContext& context, float deltaTime)
	{
		if (TryChangeAirAction(context, true, 0.1f))
			return;
	}

	/// JumpLanding //////////////////////////////////////////////////////////////////////////////
	ChiJumpLandingState::ChiJumpLandingState()
		: ChiState(ChiStateId::JumpLanding, ChiAnimationClipId::JumpLanding, MakeJumpPlayOption(false, 0.f))
	{
	}

	void ChiJumpLandingState::Tick(ChiStateContext& context, float deltaTime)
	{
		if (context.jumpInput)
		{
			context.stateMachine->ChangeState(ChiStateId::JumpUp, context.jumpInput.value());
			return;
		}

		if (context.dashInput)
		{
			ChangeDashStateByInput(context, &context.dashInput.value());
			return;
		}

		if (IsAnimationCompleted(context))
			ReturnToIdleOrRun(context);
	}

	/// DoubleJump //////////////////////////////////////////////////////////////////////////////
	ChiJumpDoubleUpState::ChiJumpDoubleUpState()
		: ChiState(ChiStateId::JumpDoubleUp, ChiAnimationClipId::JumpDoubleUp)
	{
	}

	void ChiJumpDoubleUpState::Enter(ChiStateContext& context)
	{
		ChiState::Enter(context);
		ApplyVerticalImpulse(context, DoubleJumpImpulse);
	}

	void ChiJumpDoubleUpState::Tick(ChiStateContext& context, float deltaTime)
	{
		UpdateApexGravity(context);

		if (TryChangeAirAction(context, false, 0.1f))
			return;

		if (HasLeftApex(context))
			context.stateMachine->ChangeState(ChiStateId::JumpDoubleDown);
	}

	void ChiJumpDoubleUpState::Exit(ChiStateContext& context)
	{
		RestoreJumpGravity(context);
	}

	ChiJumpDoubleDownState::ChiJumpDoubleDownState()
		: ChiState(ChiStateId::JumpDoubleDown, ChiAnimationClipId::JumpDoubleDown, MakeJumpPlayOption(true, DoubleJumpDownStartTime))
	{
	}

	void ChiJumpDoubleDownState::Tick(ChiStateContext& context, float deltaTime)
	{
		if (TryChangeAirAction(context, false, 0.1f))
			return;
	}
}
