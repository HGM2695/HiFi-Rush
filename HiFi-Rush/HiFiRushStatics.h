#pragma once

namespace gm
{
	class BeatSystem;
#if GM_ENABLE_DEBUG_TOOLS
	class DebugEventPublisher;
#endif
	class HiFiRushGameInstance;

	class HiFiRushStatics
	{
	public:
		static HiFiRushGameInstance&	GetGameInstance();
		static BeatSystem&				GetBeatSystem();
#if GM_ENABLE_DEBUG_TOOLS
		static DebugEventPublisher&		GetDebugEventPublisher();
#endif
	};
}
