#pragma once

#include "GameInstance.h"
#include "BeatSystem.h"
#if GM_ENABLE_DEBUG_TOOLS
#include "DebugEventPublisher.h"
#endif

namespace gm
{
	class HiFiRushGameInstance final : public GameInstance
	{
	public:
		BeatSystem& GetBeatSystem() { return _beatSystem; }
		const BeatSystem& GetBeatSystem() const { return _beatSystem; }
#if GM_ENABLE_DEBUG_TOOLS
		DebugEventPublisher& GetDebugEventPublisher() { return _debugEventPublisher; }
#endif

	protected:
		bool OnInitialize() override;
		void OnTick(float) override;

	private:
		void SetupScenes();
		void SetupDebugTools();

	private:
		BeatSystem		_beatSystem;
#if GM_ENABLE_DEBUG_TOOLS
		DebugEventPublisher	_debugEventPublisher;
#endif
	};
}
