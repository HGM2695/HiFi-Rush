#pragma once

#include "CombatTypes.h"
#include "Component.h"
#include "WeakGameObjectPtr.h"

#include <functional>
#include <utility>
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
		void	SetRehitInterval(float rehitInterval);
		float	GetRehitInterval() const { return _rehitInterval; }
		void	SetDamage(int32 damage);
		int32	GetDamage() const { return _damageInfo.amount; }
		void	SetHitReactionType(HitReactionType hitReactionType) { _damageInfo.hitReactionType = hitReactionType; }
		HitReactionType GetHitReactionType() const { return _damageInfo.hitReactionType; }
		void	SetWorldKnockbackDirection(const Vector3& direction) { _damageInfo.worldKnockbackDirection = direction; }
		void	SetIgnoreInvincibility(bool ignoreInvincibility) { _damageInfo.ignoreInvincibility = ignoreInvincibility; }
		bool	IsIgnoreInvincibility() const { return _damageInfo.ignoreInvincibility; }
		void	SetDamageInfo(const DamageInfo& damageInfo);
		const DamageInfo& GetDamageInfo() const { return _damageInfo; }
		void	SetHitCondition(std::function<bool(const HurtBoxComponent&)> hitCondition) { _hitCondition = std::move(hitCondition); }

		Collider3DComponent&		GetCollider() { return _collider; }
		const Collider3DComponent&	GetCollider() const { return _collider; }

		EventPublisher<HitBoxComponent, HitEvent> OnHit;

	protected:
		void OnInitialize() override;
		void OnTick(float deltaTime) override;

	private:
		struct HitTarget
		{
			WeakGameObjectPtr	target{};
			float				remainingTime = 0.f;
		};

		void				HandleCollisionEvent(const Collision3DEvent& event);
		HurtBoxComponent*	FindHurtBox(Collider3DComponent& collider) const;
		bool				IsAlreadyHit(const WeakGameObjectPtr& target) const;

		Collider3DComponent&			_collider;
		std::vector<HitTarget>			_hitTargets{};
		EventConnection					_collisionEnterConnection{};
		EventConnection					_collisionStayConnection{};
		DamageInfo						_damageInfo{};
		std::function<bool(const HurtBoxComponent&)> _hitCondition{};
		float							_rehitInterval = 0.f;
		bool							_isAttackActive = false;
	};
}
