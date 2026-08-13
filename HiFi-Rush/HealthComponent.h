#pragma once

#include "CombatTypes.h"
#include "Component.h"

namespace gm
{
	class HealthComponent final : public Component
	{
	public:
		explicit HealthComponent(int32 maxHealth);

		DamageResult	ApplyDamage(const HitEvent& hitEvent);
		DamageResult	ApplyDamage(const DamageInfo& damageInfo);
		int32			Heal(int32 amount);

		int32	GetHealth() const { return _health; }
		int32	GetMaxHealth() const { return _maxHealth; }
		float	GetHealthRatio() const;
		bool	IsDead() const { return _health <= 0; }

		void	SetInvincible(bool invincible) { _isInvincible = invincible; }
		bool	IsInvincible() const { return _isInvincible || _remainingInvincibilityTime > 0.f; }
		void	StartInvincibility(float duration);
		void	ClearInvincibility();
		float	GetRemainingInvincibilityTime() const { return _remainingInvincibilityTime; }
		void	SetDamageInvincibilityDuration(float duration);
		float	GetDamageInvincibilityDuration() const { return _damageInvincibilityDuration; }

		EventPublisher<HealthComponent, HealthChangedEvent>	OnHealthChanged;
		EventPublisher<HealthComponent, HitEvent>			OnDamaged;
		EventPublisher<HealthComponent, HitEvent>			OnDeath;

	protected:
		void OnTick(float deltaTime) override;

	private:
		int32	_maxHealth = 1;
		int32	_health = 1;
		float	_damageInvincibilityDuration = 0.f;
		float	_remainingInvincibilityTime = 0.f;
		bool	_isInvincible = false;
	};
}
