#pragma once

#include "Component.h"
#include "MathTypes.h"
#include "TriggerBinding.h"

#include <optional>
#include <string>

namespace gm
{
	class BeatSystem;
	class LightComponent;

	struct TriggeredLightColorDesc
	{
		std::wstring	triggerId{};
		float			beatOffset = 0.f;
		Color			color = Colors::White;
	};

	class TriggeredLightColorComponent final : public Component
	{
	public:
		TriggeredLightColorComponent(const BeatSystem& beatSystem, TriggeredLightColorDesc desc);

	protected:
		void OnInitialize() override;
		void OnTick(float deltaTime) override;

	private:
		void ScheduleColorChange(float startBeat);
		void ResetColor();
		void ApplyColor();

	private:
		const BeatSystem&				_beatSystem;
		TriggeredLightColorDesc			_desc{};
		TriggerBinding					_triggerBinding{};
		LightComponent*					_light = nullptr;
		Color							_initialColor = Colors::White;
		std::optional<float>			_triggerBeat{};
		bool							_isApplied = false;
	};
}
