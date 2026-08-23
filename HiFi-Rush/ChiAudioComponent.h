#pragma once

#include "Component.h"
#include "Event.h"

#include <cstddef>

namespace gm
{
	struct HitEvent;
	struct RhythmJudgeResult;

	class ChiAudioComponent final : public Component
	{
	protected:
		void OnInitialize() override;

	private:
		void HandleRhythmActionStarted(const RhythmJudgeResult& result);
		void HandleDamaged(const HitEvent& event);

		EventConnection	_rhythmActionConnection{};
		EventConnection	_damagedConnection{};
		size_t			_attackVoiceIndex = 0;
		size_t			_jumpVoiceIndex = 0;
	};
}
