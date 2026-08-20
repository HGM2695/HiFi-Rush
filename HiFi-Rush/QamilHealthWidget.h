#pragma once

#include "Event.h"
#include "UserWidget.h"

#include <array>

namespace gm
{
	class BeatSystem;
	class HealthComponent;
	class Image;
	struct HealthChangedEvent;

	class QamilHealthWidget final : public UserWidget
	{
	public:
		inline static constexpr const wchar_t* RootWidgetName = L"QamilHealthRoot";

		QamilHealthWidget(const BeatSystem& beatSystem, HealthComponent& healthComponent);

	protected:
		std::unique_ptr<Widget> BuildWidgetTree() override;
		void OnInitialize() override;
		void OnTick(float deltaTime) override;

	private:
		static constexpr uint32 HealthSegmentCount = 4;
		static constexpr uint32 DamageEffectCount = 4;

		struct HealthSegmentEntry
		{
			Image*	background = nullptr;
			Image*	brokenBackground = nullptr;
			Image*	health = nullptr;
			Image*	damage = nullptr;
		};

		struct DamageEffectEntry
		{
			Image*	image = nullptr;
			float	elapsed = 0.f;
			bool	isActive = false;
		};

		void HandleHealthChanged(const HealthChangedEvent& event);
		void UpdateHealthSegments();
		void UpdateShadow();
		void PlayDamageFeedback();
		void UpdateDamageOverlay(float deltaTime);
		void UpdateDamageEffects(float deltaTime);
		void StopDamageOverlay();
		float CalculateSegmentFillRatio(uint32 segmentIndex) const;
		float CalculateHealthBoundaryX() const;

	private:
		std::array<HealthSegmentEntry, HealthSegmentCount>	_healthSegments{};
		std::array<DamageEffectEntry, DamageEffectCount>		_damageEffects{};
		std::array<Image*, 2>								_goldImages{};
		Image*											_shadowImage = nullptr;
		Image*											_nameImage = nullptr;

		const BeatSystem&	_beatSystem;
		HealthComponent&	_healthComponent;
		EventConnection		_healthChangedConnection{};
		float				_healthRatio = 1.f;
		float				_damageOverlayAlpha = 0.f;
		uint32				_nextDamageEffectIndex = 0;
	};
}
