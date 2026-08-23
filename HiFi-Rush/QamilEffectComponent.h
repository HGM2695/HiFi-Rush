#pragma once

#include "Component.h"
#include "EffectSpawner.h"
#include "Event.h"

namespace gm
{
	class EffectPresets;
	class EffectInstance;
	class Resources;
	struct HitEvent;

	class QamilEffectComponent final : public Component
	{
	public:
		QamilEffectComponent(Resources& resources, const EffectPresets& effectPresets);
		bool SpawnPunchImpact(const Vector3& handCenter, float handRadius) const;
		bool SpawnStump(const Vector3& platformPosition) const;
		bool SpawnSweepWarning(const Vector3& attackCenter, _Out_ EffectInstance& outInstance) const;
		bool SpawnSweep(const Vector3& attackCenter, bool isClockwise) const;
		bool SpawnLaserStrike(const Vector3& handCenter, const Vector3& direction) const;
		bool SpawnLaserRestoreSmoke(const Vector3& leftHandCenter, const Vector3& rightHandCenter) const;

	protected:
		void OnInitialize() override;

	private:
		void HandleDamaged(const HitEvent& event);

		EffectSpawner	_effectSpawner;
		EventConnection	_damagedConnection{};
	};
}
