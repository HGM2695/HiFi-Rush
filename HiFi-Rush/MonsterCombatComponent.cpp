#include "MonsterCombatComponent.h"

#include "BeatSystem.h"
#include "GameObject.h"
#include "GameplayScene.h"
#include "MathUtil.h"
#include "TransformComponent.h"

#include <limits>

namespace gm
{
	MonsterCombatComponent::MonsterCombatComponent(MonsterType monsterType, const BeatSystem& beatSystem, float attackCooldownBeats)
		: _monsterType(monsterType), _beatSystem(beatSystem), _attackCooldownBeats(attackCooldownBeats)
	{
		GM_ASSERT(monsterType != MonsterType::Count, "MonsterCombatComponent에 유효한 Monster Type이 필요합니다.");
		GM_ASSERT(attackCooldownBeats >= 0.f, "Monster 공격 쿨다운은 0 Beat 이상이어야 합니다.");
	}

	void MonsterCombatComponent::SetTarget(const GameObject& target)
	{
		_target = target.GetWeakPtr();
	}

	Vector3 MonsterCombatComponent::GetTargetDirection() const
	{
		const GameObject* target = _target.Get();
		const TransformComponent* ownerTransform = GetOwner().GetTransform();
		if (target == nullptr || ownerTransform == nullptr || target->GetTransform() == nullptr)
			return {};

		const Vector3 direction = target->GetTransform()->GetPosition() - ownerTransform->GetPosition();
		return Math::GetNormalizedXZDirection(direction);
	}

	float MonsterCombatComponent::GetTargetDistance() const
	{
		const GameObject* target = _target.Get();
		const TransformComponent* ownerTransform = GetOwner().GetTransform();
		if (target == nullptr || ownerTransform == nullptr || target->GetTransform() == nullptr)
			return (std::numeric_limits<float>::max)();

		const Vector3 offset = Math::ProjectOnXZPlane(target->GetTransform()->GetPosition() - ownerTransform->GetPosition());
		return offset.Length();
	}

	void MonsterCombatComponent::SetAttackCooldownBeats(float cooldownBeats)
	{
		GM_ASSERT_RETURN(cooldownBeats >= 0.f, "Monster 공격 쿨다운은 0 Beat 이상이어야 합니다.");
		_attackCooldownBeats = cooldownBeats;
	}

	bool MonsterCombatComponent::IsAttackReady() const
	{
		return IsCombatEnabled() && HasTarget() && _beatSystem.GetCurrentBeat() >= _nextAttackBeat;
	}

	bool MonsterCombatComponent::TryStartAttack()
	{
		if (IsAttackReady() == false)
			return false;

		ResetAttackCooldown(_attackCooldownBeats);
		return true;
	}

	void MonsterCombatComponent::ResetAttackCooldown(float delayBeats)
	{
		GM_ASSERT_RETURN(delayBeats >= 0.f, "Monster 공격 지연 시간은 0 Beat 이상이어야 합니다.");
		_nextAttackBeat = _beatSystem.GetCurrentBeat() + delayBeats;
	}

	void MonsterCombatComponent::OnInitialize()
	{
		if (HasTarget() == false)
		{
			GameplayScene* gameplayScene = dynamic_cast<GameplayScene*>(GetOwner().GetScene());
			if (gameplayScene != nullptr && gameplayScene->GetPlayer().IsValid())
				_target = gameplayScene->GetPlayer();
		}

		_previousBeat = _beatSystem.GetCurrentBeat();
		ResetAttackCooldown(_attackCooldownBeats * 0.5f);
	}

	void MonsterCombatComponent::OnTick(float)
	{
		const float currentBeat = _beatSystem.GetCurrentBeat();
		if (currentBeat < _previousBeat)
			ResetAttackCooldown(_attackCooldownBeats * 0.5f);

		_previousBeat = currentBeat;
	}
}
