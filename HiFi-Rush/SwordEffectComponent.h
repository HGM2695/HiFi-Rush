#pragma once

#include "Component.h"
#include "EffectSpawner.h"

namespace gm
{
	enum class SwordAnimationId : uint32;
	class EffectPresets;
	class Resources;

	class SwordEffectComponent final : public Component
	{
	public:
		SwordEffectComponent(Resources& resources, const EffectPresets& effectPresets);

		bool SpawnChargeEffect(SwordAnimationId animationId);
		bool SpawnSlashEffect(SwordAnimationId animationId);

	private:
		EffectSpawner _effectSpawner;
	};
}
