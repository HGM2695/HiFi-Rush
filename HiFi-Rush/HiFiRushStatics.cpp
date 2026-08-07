#include "HiFiRushStatics.h"
#include "Application.h"
#include "HiFiRushGameInstance.h"

namespace gm
{
	HiFiRushGameInstance& HiFiRushStatics::GetGameInstance()
	{
		return static_cast<HiFiRushGameInstance&>(APPLICATION.GetGameInstance());
	}

	BeatSystem& HiFiRushStatics::GetBeatSystem()
	{
		return GetGameInstance().GetBeatSystem();
	}

#if GM_ENABLE_DEBUG_TOOLS
	DebugEventPublisher& HiFiRushStatics::GetDebugEventPublisher()
	{
		return GetGameInstance().GetDebugEventPublisher();
	}
#endif
}
