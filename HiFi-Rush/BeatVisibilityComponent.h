#pragma once

#include "Component.h"
#include "TriggerBinding.h"

namespace gm
{
	class BeatSystem;

	struct BeatVisibilityDesc
	{
		std::wstring	triggerId{};
		float			beatOffset = 0.f;
		bool			initialVisible = false;
		bool			visibleOnTrigger = true;
	};

	class BeatVisibilityComponent : public Component
	{
	public:
		BeatVisibilityComponent(const BeatSystem& beatSystem, const BeatVisibilityDesc& desc);

	protected:
		void OnInitialize() override;
		void OnTick(float deltaTime) override;

	private:
		enum class VisibilityState
		{
			Initial,
			Scheduled,
			Triggered,
		};

	private:
		void Schedule(float startBeat);
		void ResetAction();

	private:
		const BeatSystem&	_beatSystem;
		BeatVisibilityDesc	_desc{};
		TriggerBinding		_triggerBinding{};
		float				_triggerBeat = 0.f;
		VisibilityState		_state = VisibilityState::Initial;
	};
}
