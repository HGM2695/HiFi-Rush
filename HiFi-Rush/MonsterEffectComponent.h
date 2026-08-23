#pragma once

#include "Component.h"
#include "EffectSpawner.h"
#include "Event.h"

namespace gm
{
	class Resources;
	class EffectPresets;
	struct HitEvent;
	struct MonsterDeathAnimationCompletedEvent;

	class MonsterEffectComponent final : public Component
	{
	public:
		MonsterEffectComponent(Resources& resources, const EffectPresets& effectPresets);

	protected:
		void OnInitialize() override;

	private:
		void HandleDamaged(const HitEvent& event);
		void HandleDeathAnimationCompleted(const MonsterDeathAnimationCompletedEvent& event);

		EffectSpawner		_effectSpawner;
		EventConnection		_damagedConnection{};
		EventConnection		_deathAnimationCompletedConnection{};
	};
}
