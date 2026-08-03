#include "ChiJumpState.h"
#include "Application.h"
#include "ChiStateMachineComponent.h"
#include "Input.h"
#include "NavMeshControllerComponent.h"
#include "Rigidbody3DComponent.h"

namespace gm
{
	namespace
	{
		constexpr float JumpImpulse = 9.5f;
		constexpr float DoubleJumpImpulse = 9.5f;

		void ApplyVerticalImpulse(ChiStateContext& context, float impulse)
		{
			GM_ASSERT_RETURN(context.rigidbodyComponent, "점프 상태에는 Rigidbody3DComponent가 필요합니다.");

			Vector3 velocity = context.rigidbodyComponent->GetVelocity();
			velocity.y = 0.f;
			context.rigidbodyComponent->SetVelocity(velocity);
			context.rigidbodyComponent->AddImpulse(Vector3{ 0.f, impulse, 0.f });
		}

		bool IsAscending(const ChiStateContext& context)
		{
			return context.rigidbodyComponent && context.rigidbodyComponent->GetVelocity().y > 0.f;
		}

		bool IsGrounded(const ChiStateContext& context)
		{
			return context.navMeshControllerComponent && context.navMeshControllerComponent->IsGrounded();
		}
	}

	/// Jump //////////////////////////////////////////////////////////////////////////////
	ChiJumpUpState::ChiJumpUpState()
		: ChiClipState(ChiStateId::JumpUp, ChiAnimationId::JumpUp)
	{
	}

	void ChiJumpUpState::Enter(ChiStateContext& context)
	{
		ChiClipState::Enter(context);
		ApplyVerticalImpulse(context, JumpImpulse);
	}

	void ChiJumpUpState::Tick(ChiStateContext& context, float deltaTime)
	{
		if (TryChangeAirAction(context, true))
			return;

		if (IsAscending(context) == false)
			context.stateMachine->ChangeState(ChiStateId::JumpDown);
	}

	ChiJumpDownState::ChiJumpDownState()
		: ChiClipState(ChiStateId::JumpDown, ChiAnimationId::JumpDown, false)
	{
	}

	void ChiJumpDownState::Tick(ChiStateContext& context, float deltaTime)
	{
		if (TryChangeAirAction(context, true))
			return;

		// NavMeshController가 바닥 접촉을 확정한 뒤 착지 상태로 전환합니다.
		if (IsGrounded(context))
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

	void ChiJumpDoubleUpState::Enter(ChiStateContext& context)
	{
		ChiClipState::Enter(context);
		ApplyVerticalImpulse(context, DoubleJumpImpulse);
	}

	void ChiJumpDoubleUpState::Tick(ChiStateContext& context, float deltaTime)
	{
		if (TryChangeAirAction(context, false))
			return;

		if (IsAscending(context) == false)
			context.stateMachine->ChangeState(ChiStateId::JumpDoubleDown);
	}

	ChiJumpDoubleDownState::ChiJumpDoubleDownState()
		: ChiClipState(ChiStateId::JumpDoubleDown, ChiAnimationId::JumpDoubleDown, false)
	{
	}

	void ChiJumpDoubleDownState::Tick(ChiStateContext& context, float deltaTime)
	{
		if (TryChangeAirAction(context, false))
			return;

		// NavMeshController가 바닥 접촉을 확정한 뒤 착지 상태로 전환합니다.
		if (IsGrounded(context))
			context.stateMachine->ChangeState(ChiStateId::JumpLanding);
	}
}
