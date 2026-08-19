#include "MonsterCombatActivationComponent.h"

#include "BeatSystem.h"
#include "Collider3DComponent.h"
#include "GameObject.h"
#include "GameplayScene.h"
#include "MonsterCombatComponent.h"
#include "MonsterStateMachineComponent.h"

#include <utility>

namespace gm
{
	MonsterCombatActivationComponent::MonsterCombatActivationComponent(const BeatSystem& beatSystem, MonsterCombatActivationDesc desc)
		: _beatSystem(beatSystem), _desc(std::move(desc))
	{
	}

	void MonsterCombatActivationComponent::Schedule(float activationBeat)
	{
		if (_state != ActivationState::Inactive)
			return;

		_activationBeat = activationBeat;
		_state = ActivationState::Scheduled;
	}

	void MonsterCombatActivationComponent::ResetActivation()
	{
		SetCombatActive(false);
		_activationBeat = 0.f;
		_state = ActivationState::Inactive;
	}

	void MonsterCombatActivationComponent::ActivateCombat()
	{
		SetCombatActive(true);
		_combatComponent->ResetAttackCooldown(_combatComponent->GetAttackCooldownBeats() * 0.5f);
		GM_ASSERT_RETURN(_stateMachine->ChangeState(_stateMachine->GetInitialStateId(), true), "Monster 초기 State 재시작에 실패했습니다.");
		_state = ActivationState::Active;
	}

	void MonsterCombatActivationComponent::SetCombatActive(bool isActive)
	{
		_combatComponent->SetCombatEnabled(isActive);
		_stateMachine->SetEnabled(isActive);
		for (const ColliderState& colliderState : _colliderStates)
			colliderState.collider->SetEnabled(isActive && colliderState.wasEnabled);
	}

	void MonsterCombatActivationComponent::OnInitialize()
	{
		_combatComponent = GetOwner().GetComponent<MonsterCombatComponent>();
		_stateMachine = GetOwner().GetComponent<MonsterStateMachineComponent>();
		GM_ASSERT_RETURN(_combatComponent, "MonsterCombatActivationComponent에 MonsterCombatComponent가 없습니다.");
		GM_ASSERT_RETURN(_stateMachine, "MonsterCombatActivationComponent에 MonsterStateMachineComponent가 없습니다.");
		for (Collider3DComponent* collider : GetOwner().GetColliders3D())
			_colliderStates.push_back({ collider, collider->IsEnabled() });

		ResetActivation();

		GameplayScene* scene = dynamic_cast<GameplayScene*>(GetOwner().GetScene());
		GM_ASSERT_RETURN(scene, "MonsterCombatActivationComponent는 GameplayScene에서만 사용할 수 있습니다.");
		GM_ASSERT_RETURN(_triggerBinding.Bind(scene->GetTriggerSystem(), _desc.triggerId, _desc.beatOffset,
			[this](float activationBeat) { Schedule(activationBeat); },
			[this]() { ResetActivation(); }), "MonsterCombatActivationComponent의 Trigger Binding에 실패했습니다.");
	}

	void MonsterCombatActivationComponent::OnTick(float)
	{
		if (_state != ActivationState::Scheduled || _beatSystem.HasPlaybackTime() == false || _beatSystem.GetCurrentBeat() < _activationBeat)
			return;

		ActivateCombat();
	}
}
