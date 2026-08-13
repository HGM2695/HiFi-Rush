#include "HealthComponent.h"

#include <algorithm>

namespace gm
{
	HealthComponent::HealthComponent(int32 maxHealth)
		: _maxHealth((std::max)(maxHealth, 1)), _health(_maxHealth)
	{
		GM_ASSERT(maxHealth > 0, "Max Health는 0보다 커야 합니다.");
	}

	DamageResult HealthComponent::ApplyDamage(const HitEvent& hitEvent)
	{
		DamageResult result{};
		result.requestedDamage = hitEvent.damage.amount;
		result.previousHealth = _health;
		result.currentHealth = _health;

		if (hitEvent.damage.amount <= 0 || IsDead())
			return result;

		if (IsInvincible() && hitEvent.damage.ignoreInvincibility == false)
		{
			result.state = DamageState::Blocked;

			HitEvent blockedHitEvent = hitEvent;
			blockedHitEvent.damageResult = result;
			OnDamaged.Publish(blockedHitEvent);
			return result;
		}

		_health = (std::max)(_health - hitEvent.damage.amount, 0);
		result.appliedDamage = result.previousHealth - _health;
		result.currentHealth = _health;
		result.isDead = IsDead();

		if (result.appliedDamage <= 0)
			return result;

		result.state = DamageState::Applied;

		if (result.isDead == false && _damageInvincibilityDuration > 0.f)
			StartInvincibility(_damageInvincibilityDuration);

		HealthChangedEvent healthChangedEvent{};
		healthChangedEvent.previousHealth = result.previousHealth;
		healthChangedEvent.currentHealth = result.currentHealth;
		healthChangedEvent.maxHealth = _maxHealth;
		OnHealthChanged.Publish(healthChangedEvent);

		HitEvent damagedHitEvent = hitEvent;
		damagedHitEvent.damageResult = result;
		OnDamaged.Publish(damagedHitEvent);

		if (result.isDead)
			OnDeath.Publish(damagedHitEvent);

		return result;
	}

	DamageResult HealthComponent::ApplyDamage(const DamageInfo& damageInfo)
	{
		HitEvent hitEvent{};
		hitEvent.damage = damageInfo;
		return ApplyDamage(hitEvent);
	}

	int32 HealthComponent::Heal(int32 amount)
	{
		GM_ASSERT_RETURN_VAL(amount >= 0, 0, "회복량은 0 이상이어야 합니다.");
		if (amount == 0 || IsDead() || _health >= _maxHealth)
			return 0;

		const int32 previousHealth = _health;
		_health = (std::min)(_health + amount, _maxHealth);

		HealthChangedEvent healthChangedEvent{};
		healthChangedEvent.previousHealth = previousHealth;
		healthChangedEvent.currentHealth = _health;
		healthChangedEvent.maxHealth = _maxHealth;
		OnHealthChanged.Publish(healthChangedEvent);
		return _health - previousHealth;
	}

	float HealthComponent::GetHealthRatio() const
	{
		return static_cast<float>(_health) / static_cast<float>(_maxHealth);
	}

	void HealthComponent::StartInvincibility(float duration)
	{
		GM_ASSERT_RETURN(duration >= 0.f, "무적 시간은 0 이상이어야 합니다.");
		_remainingInvincibilityTime = duration;
	}

	void HealthComponent::ClearInvincibility()
	{
		_isInvincible = false;
		_remainingInvincibilityTime = 0.f;
	}

	void HealthComponent::SetDamageInvincibilityDuration(float duration)
	{
		GM_ASSERT_RETURN(duration >= 0.f, "Damage 무적 시간은 0 이상이어야 합니다.");
		_damageInvincibilityDuration = duration;
	}

	void HealthComponent::OnTick(float deltaTime)
	{
		if (_remainingInvincibilityTime <= 0.f)
			return;

		_remainingInvincibilityTime = (std::max)(_remainingInvincibilityTime - deltaTime, 0.f);
	}
}
