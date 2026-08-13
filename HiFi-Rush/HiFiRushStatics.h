#pragma once

namespace gm
{
	class BeatSystem;
#if GM_ENABLE_DEBUG_TOOLS
	class DebugEventPublisher;
#endif
	class HiFiRushGameInstance;
	struct PlayerRuntimeState;

	class HiFiRushStatics
	{
	public:
		static HiFiRushGameInstance&	GetGameInstance();
		static BeatSystem&				GetBeatSystem();
		static PlayerRuntimeState&		GetPlayerRuntimeState();
#if GM_ENABLE_DEBUG_TOOLS
		static DebugEventPublisher&		GetDebugEventPublisher();
#endif
	};
}
