#include "ChiJumpState.h"
#include "Application.h"
#include "ChiStateMachineComponent.h"
#include "Input.h"
#include "Rigidbody3DComponent.h"

#include <cmath>

namespace gm
{
	namespace
	{
		constexpr float JumpImpulse = 9.f;
		constexpr float DoubleJumpImpulse = 12.f;
		constexpr float JumpStartTime = 0.1f;
		constexpr float JumpLandingBlendDuration = 0.075f;
		constexpr float DoubleJumpDownStartTime = 0.f;
		constexpr float JumpGravityScale = 3.f;
		constexpr float ApexGravityScale = 1.f;
		constexpr float ApexVelocityThreshold = 2.f;

		AnimationPlayOption MakeJumpPlayOption(bool isLoop, float startTime, float blendDuration = ChiDefaultBlendDuration)
		{
			return AnimationPlayOption{ .startTime = startTime, .blendDuration = blendDuration, .loopOverride = isLoop };
		}

		void ApplyVerticalImpulse(ChiStateContext& context, float impulse)
		{
			GM_ASSERT_RETURN(context.rigidbodyComponent, "점프 상태에는 Rigidbody3DComponent가 필요합니다.");

			Vector3 velocity = context.rigidbodyComponent->GetVelocity();
			velocity.y = 0.f;
			context.rigidbodyComponent->SetVelocity(velocity);
			context.rigidbodyComponent->SetGravityScale(JumpGravityScale);
			context.rigidbodyComponent->AddImpulse(Vector3{ 0.f, impulse, 0.f });
		}

		void UpdateApexGravity(ChiStateContext& context)
		{
			GM_ASSERT_RETURN(context.rigidbodyComponent, "점프 상태에는 Rigidbody3DComponent가 필요합니다.");
			const float verticalSpeed = std::abs(context.rigidbodyComponent->GetVelocity().y);
			context.rigidbodyComponent->SetGravityScale(verticalSpeed <= ApexVelocityThreshold ? ApexGravityScale : JumpGravityScale);
		}

		void RestoreJumpGravity(ChiStateContext& context)
		{
			if (context.rigidbodyComponent)
				context.rigidbodyComponent->SetGravityScale(JumpGravityScale);
		}

		bool HasLeftApex(const ChiStateContext& context)
		{
			return context.rigidbodyComponent && context.rigidbodyComponent->GetVelocity().y <= -ApexVelocityThreshold;
		}
	}

	/// Jump //////////////////////////////////////////////////////////////////////////////
	ChiJumpUpState::ChiJumpUpState()
		: ChiClipState(ChiStateId::JumpUp, ChiAnimationId::JumpUp, MakeJumpPlayOption(true, JumpStartTime))
	{
	}

	void ChiJumpUpState::Enter(ChiStateContext& context)
	{
		ChiClipState::Enter(context);
		ApplyVerticalImpulse(context, JumpImpulse);
	}

	void ChiJumpUpState::Tick(ChiStateContext& context, float deltaTime)
	{
		UpdateApexGravity(context);

		if (TryChangeAirAction(context, true))
			return;

		if (HasLeftApex(context))
			context.stateMachine->ChangeState(ChiStateId::JumpDown);
	}

	void ChiJumpUpState::Exit(ChiStateContext& context)
	{
		RestoreJumpGravity(context);
		ChiClipState::Exit(context);
	}

	/// JumpDown //////////////////////////////////////////////////////////////////////////////
	ChiJumpDownState::ChiJumpDownState()
		: ChiClipState(ChiStateId::JumpDown, ChiAnimationId::JumpDown, MakeJumpPlayOption(true, JumpStartTime))
	{
	}

	void ChiJumpDownState::Tick(ChiStateContext& context, float deltaTime)
	{
		if (TryChangeAirAction(context, true))
			return;
	}

	/// JumpLanding //////////////////////////////////////////////////////////////////////////////
	ChiJumpLandingState::ChiJumpLandingState()
		: ChiClipState(ChiStateId::JumpLanding, ChiAnimationId::JumpLanding, MakeJumpPlayOption(false, 0.f, JumpLandingBlendDuration))
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

	void ChiJumpDoubleUpState::Enter(ChiStateContext& context)
	{
		ChiClipState::Enter(context);
		ApplyVerticalImpulse(context, DoubleJumpImpulse);
	}

	void ChiJumpDoubleUpState::Tick(ChiStateContext& context, float deltaTime)
	{
		UpdateApexGravity(context);

		if (TryChangeAirAction(context, false))
			return;

		if (HasLeftApex(context))
			context.stateMachine->ChangeState(ChiStateId::JumpDoubleDown);
	}

	void ChiJumpDoubleUpState::Exit(ChiStateContext& context)
	{
		RestoreJumpGravity(context);
		ChiClipState::Exit(context);
	}

	ChiJumpDoubleDownState::ChiJumpDoubleDownState()
		: ChiClipState(ChiStateId::JumpDoubleDown, ChiAnimationId::JumpDoubleDown, MakeJumpPlayOption(true, DoubleJumpDownStartTime))
	{
	}

	void ChiJumpDoubleDownState::Tick(ChiStateContext& context, float deltaTime)
	{
		if (TryChangeAirAction(context, false))
			return;
	}
}
