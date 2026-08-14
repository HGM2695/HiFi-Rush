#pragma once

#include "Component.h"
#include "MonsterTypes.h"
#include "WeakGameObjectPtr.h"

namespace gm
{
	class BeatSystem;
	class GameObject;

	class MonsterCombatComponent final : public Component
	{
	public:
		MonsterCombatComponent(MonsterType monsterType, const BeatSystem& beatSystem, float attackCooldownBeats);

		MonsterType	GetMonsterType() const { return _monsterType; }
		void		SetTarget(const GameObject& target);
		void		ClearTarget() { _target.Reset(); }
		GameObject*	GetTarget() const { return _target.Get(); }
		bool		HasTarget() const { return _target.IsValid(); }
		Vector3		GetTargetDirection() const;
		float		GetTargetDistance() const;

		void		SetCombatEnabled(bool enabled) { _combatEnabled = enabled; }
		bool		IsCombatEnabled() const { return IsEnabled() && _combatEnabled; }
		void		SetAttackCooldownBeats(float cooldownBeats);
		float		GetAttackCooldownBeats() const { return _attackCooldownBeats; }
		bool		IsAttackReady() const;
		bool		TryStartAttack();
		void		ResetAttackCooldown(float delayBeats = 0.f);

	protected:
		void OnInitialize() override;
		void OnTick(float deltaTime) override;

	private:
		MonsterType			_monsterType = MonsterType::Count;
		const BeatSystem&	_beatSystem;
		WeakGameObjectPtr	_target{};
		float				_attackCooldownBeats = 0.f;
		float				_nextAttackBeat = 0.f;
		float				_previousBeat = 0.f;
		bool				_combatEnabled = true;
	};
}
