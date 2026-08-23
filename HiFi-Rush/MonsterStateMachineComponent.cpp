#include "MonsterStateMachineComponent.h"
#include "HiFiRushAudio.h"

#include "CharacterMovementComponent.h"
#include "GameObject.h"
#include "HealthComponent.h"
#include "MonsterCombatComponent.h"
#include "MonsterState.h"
#include "NavMeshControllerComponent.h"
#include "Rigidbody3DComponent.h"
#include "SkeletalAnimatorComponent.h"

namespace gm
{
	MonsterStateMachineComponent::MonsterStateMachineComponent() = default;
	MonsterStateMachineComponent::~MonsterStateMachineComponent() = default;

	bool MonsterStateMachineComponent::RegisterState(std::unique_ptr<MonsterState> state)
	{
		GM_ASSERT_RETURN_VAL(state, false, "등록할 Monster State가 유효하지 않습니다.");

		const MonsterStateId stateId = state->GetStateId();
		GM_ASSERT_RETURN_VAL(stateId != MonsterStateId::None && stateId != MonsterStateId::Count, false, "등록할 수 없는 Monster State ID입니다.");
		GM_ASSERT_RETURN_VAL(_states.contains(stateId) == false, false, "Monster State ID가 중복되었습니다.");

		_states.emplace(stateId, std::move(state));
		return true;
	}

	bool MonsterStateMachineComponent::HasState(MonsterStateId stateId) const
	{
		return FindState(stateId) != nullptr;
	}

	bool MonsterStateMachineComponent::ChangeState(MonsterStateId nextStateId, bool restart)
	{
		if (_currentStateId == nextStateId && restart == false)
			return true;

		MonsterState* nextState = FindState(nextStateId);
		GM_ASSERT_RETURN_VAL(nextState, false, "등록되지 않은 Monster State로 전환할 수 없습니다.");

		MonsterState* currentState = FindState(_currentStateId);
		if (currentState)
			currentState->Exit(_context);

		_currentStateId = nextStateId;
		nextState->Enter(_context);
		return true;
	}

	void MonsterStateMachineComponent::CompleteDeathAnimation()
	{
		MonsterDeathAnimationCompletedEvent event{};
		OnDeathAnimationCompleted.Publish(event);
		GetOwner().Destroy();
	}

	void MonsterStateMachineComponent::OnInitialize()
	{
		_context.stateMachine = this;
		_context.combatComponent = GetOwner().GetComponent<MonsterCombatComponent>();
		if (_context.combatComponent != nullptr)
			_context.beatSystem = &_context.combatComponent->GetBeatSystem();
		_context.moveComponent = GetOwner().GetComponent<CharacterMovementComponent>();
		_context.rigidbodyComponent = GetOwner().GetRigidbody3D();
		_context.healthComponent = GetOwner().GetComponent<HealthComponent>();
		_context.animatorComponent = GetOwner().GetComponent<SkeletalAnimatorComponent>();
		NavMeshControllerComponent* navMeshController = GetOwner().GetComponent<NavMeshControllerComponent>();

		GM_ASSERT_RETURN(_context.combatComponent, "MonsterStateMachineComponent는 MonsterCombatComponent가 필요합니다.");
		GM_ASSERT_RETURN(_context.moveComponent, "MonsterStateMachineComponent는 CharacterMovementComponent가 필요합니다.");
		GM_ASSERT_RETURN(_context.rigidbodyComponent, "MonsterStateMachineComponent는 Rigidbody3DComponent가 필요합니다.");
		GM_ASSERT_RETURN(_context.healthComponent, "MonsterStateMachineComponent는 HealthComponent가 필요합니다.");
		GM_ASSERT_RETURN(_context.animatorComponent, "MonsterStateMachineComponent는 SkeletalAnimatorComponent가 필요합니다.");
		GM_ASSERT_RETURN(navMeshController, "MonsterStateMachineComponent는 NavMeshControllerComponent가 필요합니다.");

		_context.healthComponent->OnDamaged.Subscribe(_damagedConnection,
			[this](const HitEvent& event)
			{
				OnDamaged(event);
			});
		navMeshController->OnGroundContact.Subscribe(_navigationGroundContactConnection,
			[this](const NavigationGroundContactEvent&)
			{
				OnGroundContact();
			});
		_context.moveComponent->OnMovementBaseContact.Subscribe(_movementBaseContactConnection,
			[this](const MovementBaseContactEvent&)
			{
				OnGroundContact();
			});

		if (_initialStateId != MonsterStateId::None)
			ChangeState(_initialStateId);
	}

	void MonsterStateMachineComponent::OnTick(float deltaTime)
	{
		MonsterState* currentState = FindState(_currentStateId);
		if (currentState)
			currentState->Tick(_context, deltaTime);
	}

	void MonsterStateMachineComponent::OnDamaged(const HitEvent& event)
	{
		_context.lastHitReactionType = event.damage.hitReactionType;
		if (event.damageResult.state == DamageState::Applied)
		{
			if (_context.moveComponent)
				_context.moveComponent->FaceDirectionImmediate(-event.GetWorldKnockbackDirection());
			PlayRandomSound2D(HiFiRushSound::MonsterHitImpacts);
		}

		MonsterState* currentState = FindState(_currentStateId);
		if (currentState)
			currentState->OnDamaged(_context, event);
	}

	void MonsterStateMachineComponent::OnGroundContact()
	{
		MonsterState* currentState = FindState(_currentStateId);
		if (currentState)
			currentState->OnGroundContact(_context);
	}

	MonsterState* MonsterStateMachineComponent::FindState(MonsterStateId stateId) const
	{
		const auto iter = _states.find(stateId);
		if (iter == _states.end())
			return nullptr;

		return iter->second.get();
	}
}
