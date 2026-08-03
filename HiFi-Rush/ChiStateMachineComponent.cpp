#include "ChiStateMachineComponent.h"
#include "ChiAttackState.h"
#include "ChiDamageState.h"
#include "ChiDashState.h"
#include "ChiIdleRunState.h"
#include "ChiJumpState.h"
#include "ChiSpecialState.h"
#include "GameObject.h"
#include "ChiMoveComponent.h"
#include "NavMeshControllerComponent.h"
#include "Rigidbody3DComponent.h"
#include "SkeletalAnimatorComponent.h"

namespace gm
{
	namespace
	{
		void RegisterClipState(std::unordered_map<ChiStateId, std::unique_ptr<ChiState>>& states, ChiStateId stateId, ChiAnimationId animationId, bool isLoop = false)
		{
			states.emplace(stateId, std::make_unique<ChiClipState>(stateId, animationId, isLoop));
		}
	}

	ChiStateMachineComponent::ChiStateMachineComponent() = default;
	ChiStateMachineComponent::~ChiStateMachineComponent() = default;

	void ChiStateMachineComponent::OnInitialize()
	{
		_animatorComponent = GetOwner().GetComponent<SkeletalAnimatorComponent>();
		GM_ASSERT_RETURN(_animatorComponent, "ChiStateMachineComponent는 SkeletalAnimatorComponent가 필요합니다.");

		_moveComponent = GetOwner().GetComponent<ChiMoveComponent>();
		GM_ASSERT_RETURN(_moveComponent, "ChiStateMachineComponent는 ChiMoveComponent가 필요합니다.");

		NavMeshControllerComponent* navMeshControllerComponent = GetOwner().GetComponent<NavMeshControllerComponent>();
		GM_ASSERT_RETURN(navMeshControllerComponent, "ChiStateMachineComponent는 NavMeshControllerComponent가 필요합니다.");

		Rigidbody3DComponent* rigidbodyComponent = GetOwner().GetRigidbody3D();
		GM_ASSERT_RETURN(rigidbodyComponent, "ChiStateMachineComponent는 Rigidbody3DComponent가 필요합니다.");

		_context.stateMachine = this;
		_context.moveComponent = _moveComponent;
		_context.navMeshControllerComponent = navMeshControllerComponent;
		_context.rigidbodyComponent = rigidbodyComponent;
		_context.animatorComponent = _animatorComponent;

		RegisterAnimationClips();
		RegisterStates();
		ChangeState(ChiStateId::Idle);
	}

	void ChiStateMachineComponent::OnTick(float deltaTime)
	{
		ChiState* currentState = FindState(_currentStateId);
		if (currentState == nullptr)
			return;

		currentState->Tick(_context, deltaTime);
	}

	void ChiStateMachineComponent::ChangeState(ChiStateId nextStateId)
	{
		if (_currentStateId == nextStateId)
			return;

		ChiState* next = FindState(nextStateId);
		GM_ASSERT_RETURN(next, "등록되지 않은 ChiState로 전환할 수 없습니다.");

		ChiState* currentState = FindState(_currentStateId);
		if (currentState)
			currentState->Exit(_context);

		_currentStateId = nextStateId;
		next->Enter(_context);
	}

	void ChiStateMachineComponent::RegisterAnimationClips()
	{
		for (uint32 animationIndex = 0; animationIndex < ChiAnimationIdCount; ++animationIndex)
		{
			const ChiAnimationId animationId = static_cast<ChiAnimationId>(animationIndex);
			const std::wstring animationName = GetChiAnimationName(animationId);

			if (_animatorComponent->HasClip(animationName))
				continue;

			_animatorComponent->AddClip(animationName, GetChiAnimationKey(animationId));
		}
	}

	void ChiStateMachineComponent::RegisterStates()
	{
		_states.emplace(ChiStateId::Idle, std::make_unique<ChiIdleState>());
		_states.emplace(ChiStateId::Run, std::make_unique<ChiRunState>());

		_states.emplace(ChiStateId::AttackWeak0, std::make_unique<ChiWeak0AttackState>());
		_states.emplace(ChiStateId::AttackWeak1, std::make_unique<ChiWeak1AttackState>());
		_states.emplace(ChiStateId::AttackWeak2, std::make_unique<ChiWeak2AttackState>());
		_states.emplace(ChiStateId::AttackWeak3, std::make_unique<ChiWeak3AttackState>());
		_states.emplace(ChiStateId::AttackWeakDash, std::make_unique<ChiWeakDashAttackState>());
		_states.emplace(ChiStateId::AttackStrongDash, std::make_unique<ChiStrongDashAttackState>());
		_states.emplace(ChiStateId::AttackStrong0_0, std::make_unique<ChiStrong0_0AttackState>());
		_states.emplace(ChiStateId::AttackStrong0_1, std::make_unique<ChiStrong0_1AttackState>());
		_states.emplace(ChiStateId::AttackStrong1, std::make_unique<ChiStrong1AttackState>());
		_states.emplace(ChiStateId::AttackStrong2, std::make_unique<ChiStrong2AttackState>());
		_states.emplace(ChiStateId::AttackStrongToWeak1, std::make_unique<ChiStrongToWeak1AttackState>());
		_states.emplace(ChiStateId::AttackStrongToWeak2, std::make_unique<ChiStrongToWeak2AttackState>());
		_states.emplace(ChiStateId::AttackWeakToStrong1, std::make_unique<ChiWeakToStrong1AttackState>());
		_states.emplace(ChiStateId::AttackWeakToStrong2, std::make_unique<ChiWeakToStrong2AttackState>());
		_states.emplace(ChiStateId::AttackDelayedWeak1, std::make_unique<ChiDelayedWeak1AttackState>());
		_states.emplace(ChiStateId::AttackDelayedWeak2, std::make_unique<ChiDelayedWeak2AttackState>());
		_states.emplace(ChiStateId::AttackStump0, std::make_unique<ChiStump0AttackState>());
		_states.emplace(ChiStateId::AttackStump1, std::make_unique<ChiStump1AttackState>());
		_states.emplace(ChiStateId::AttackStump2, std::make_unique<ChiStump2AttackState>());
		_states.emplace(ChiStateId::AttackSky0, std::make_unique<ChiSky0AttackState>());
		_states.emplace(ChiStateId::AttackSky1, std::make_unique<ChiSky1AttackState>());
		_states.emplace(ChiStateId::AttackSky2, std::make_unique<ChiSky2AttackState>());
		_states.emplace(ChiStateId::AttackSky3, std::make_unique<ChiSky3AttackState>());

		_states.emplace(ChiStateId::DashFront, std::make_unique<ChiDashFrontState>());
		_states.emplace(ChiStateId::DashBack, std::make_unique<ChiDashBackState>());
		_states.emplace(ChiStateId::DashLeft, std::make_unique<ChiDashLeftState>());
		_states.emplace(ChiStateId::DashRight, std::make_unique<ChiDashRightState>());
		_states.emplace(ChiStateId::DashDouble, std::make_unique<ChiDashDoubleState>());
		_states.emplace(ChiStateId::DashTriple, std::make_unique<ChiDashTripleState>());
		_states.emplace(ChiStateId::DashSky, std::make_unique<ChiDashSkyState>());
		_states.emplace(ChiStateId::DashSkyFall, std::make_unique<ChiDashSkyFallState>());

		_states.emplace(ChiStateId::JumpUp, std::make_unique<ChiJumpUpState>());
		_states.emplace(ChiStateId::JumpDown, std::make_unique<ChiJumpDownState>());
		_states.emplace(ChiStateId::JumpLanding, std::make_unique<ChiJumpLandingState>());
		_states.emplace(ChiStateId::JumpDoubleUp, std::make_unique<ChiJumpDoubleUpState>());
		_states.emplace(ChiStateId::JumpDoubleDown, std::make_unique<ChiJumpDoubleDownState>());

		_states.emplace(ChiStateId::DamageStrongKnockback, std::make_unique<ChiStrongKnockbackDamageState>());
		_states.emplace(ChiStateId::DamageWeakKnockback, std::make_unique<ChiWeakKnockbackDamageState>());
		_states.emplace(ChiStateId::DamageDead, std::make_unique<ChiDeadDamageState>());

		_states.emplace(ChiStateId::HibikiReady, std::make_unique<ChiHibikiReadyState>());
		_states.emplace(ChiStateId::HibikiAttack, std::make_unique<ChiHibikiAttackState>());
	}

	ChiState* ChiStateMachineComponent::FindState(ChiStateId stateId) const
	{
		auto iter = _states.find(stateId);
		if (iter == _states.end())
			return nullptr;

		return iter->second.get();
	}
}
