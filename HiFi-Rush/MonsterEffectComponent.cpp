#include "MonsterEffectComponent.h"

#include "CombatTypes.h"
#include "GameObject.h"
#include "HealthComponent.h"
#include "MonsterStateMachineComponent.h"
#include "Scene.h"
#include "TransformComponent.h"

namespace gm
{
	namespace
	{
		const std::wstring CombatHitEffectId = L"Combat.Hit";
		const std::wstring MonsterDeathEffectId = L"Monster.Death";
	}

	MonsterEffectComponent::MonsterEffectComponent(Resources& resources, const EffectPresets& effectPresets)
		: _effectSpawner(resources, effectPresets)
	{
	}

	void MonsterEffectComponent::OnInitialize()
	{
		HealthComponent* healthComponent = GetOwner().GetComponent<HealthComponent>();
		MonsterStateMachineComponent* stateMachine = GetOwner().GetComponent<MonsterStateMachineComponent>();
		GM_ASSERT_RETURN(healthComponent && stateMachine, "MonsterEffectComponent는 Health와 MonsterStateMachine Component가 필요합니다.");

		healthComponent->OnDamaged.Subscribe(_damagedConnection, [this](const HitEvent& event) { HandleDamaged(event); });
		stateMachine->OnDeathAnimationCompleted.Subscribe(_deathAnimationCompletedConnection, [this](const MonsterDeathAnimationCompletedEvent& event) { HandleDeathAnimationCompleted(event); });
	}

	void MonsterEffectComponent::HandleDamaged(const HitEvent& event)
	{
		if (event.damageResult.state == DamageState::Ignored || event.hurtBox == nullptr)
			return;

		Scene* scene = GetOwner().GetScene();
		GM_ASSERT_RETURN(scene, "Monster Hit Effect를 생성할 Scene이 없습니다.");
		_effectSpawner.SpawnAtWorld(*scene, CombatHitEffectId, Matrix::CreateTranslation(event.contact.selfPoint));
	}

	void MonsterEffectComponent::HandleDeathAnimationCompleted(const MonsterDeathAnimationCompletedEvent&)
	{
		Scene* scene = GetOwner().GetScene();
		GM_ASSERT_RETURN(scene, "Monster Death Effect를 생성할 Scene이 없습니다.");
		const Vector3 position = GetOwner().GetTransform()->GetPosition() + Vector3{ 0.f, 1.f, 0.f };
		_effectSpawner.SpawnAtWorld(*scene, MonsterDeathEffectId, Matrix::CreateTranslation(position));
	}
}
