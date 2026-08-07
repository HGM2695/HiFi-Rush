#pragma once

#include "Component.h"
#include "IBeatTriggerAction.h"

namespace gm
{
	class BeatSystem;

	struct BeatVisibilityDesc
	{
		bool initialVisible = false;
		bool visibleOnTrigger = true;
	};

	class BeatVisibilityComponent : public Component, public IBeatTriggerAction
	{
	public:
		BeatVisibilityComponent(const BeatSystem& beatSystem, const BeatVisibilityDesc& desc);

		void Schedule(float startBeat) override;
		void Reset() override;

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
		const BeatSystem&	_beatSystem;
		BeatVisibilityDesc	_desc{};
		float				_triggerBeat = 0.f;
		VisibilityState		_state = VisibilityState::Initial;
	};
}
