#pragma once

#include "CombatTypes.h"
#include "Component.h"
#include "WeakGameObjectPtr.h"

#include <vector>

namespace gm
{
	class Collider3DComponent;
	struct Collision3DEvent;

	class HitBoxComponent final : public Component
	{
	public:
		explicit HitBoxComponent(Collider3DComponent& collider);

		void	BeginAttack();
		void	EndAttack();
		bool	IsAttackActive() const { return _isAttackActive; }
		void	SetDamage(int32 damage);
		int32	GetDamage() const { return _damageInfo.amount; }
		void	SetIgnoreInvincibility(bool ignoreInvincibility) { _damageInfo.ignoreInvincibility = ignoreInvincibility; }
		bool	IsIgnoreInvincibility() const { return _damageInfo.ignoreInvincibility; }
		void	SetDamageInfo(const DamageInfo& damageInfo);
		const DamageInfo& GetDamageInfo() const { return _damageInfo; }

		Collider3DComponent&		GetCollider() { return _collider; }
		const Collider3DComponent&	GetCollider() const { return _collider; }

		EventPublisher<HitBoxComponent, HitEvent> OnHit;

	protected:
		void OnInitialize() override;

	private:
		void				HandleCollisionEvent(const Collision3DEvent& event);
		HurtBoxComponent*	FindHurtBox(Collider3DComponent& collider) const;
		bool				IsAlreadyHit(const WeakGameObjectPtr& target) const;

		Collider3DComponent&			_collider;
		std::vector<WeakGameObjectPtr>	_hitTargets{};
		EventConnection					_collisionEnterConnection{};
		EventConnection					_collisionStayConnection{};
		DamageInfo						_damageInfo{};
		bool							_isAttackActive = false;
	};
}
