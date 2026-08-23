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

	const ChiAnimationSettings& HiFiRushStatics::GetChiAnimationSettings()
	{
		return GetGameInstance().GetChiAnimationSettings();
	}

	const EffectPresets& HiFiRushStatics::GetEffectPresets()
	{
		return GetGameInstance().GetEffectPresets();
	}

	const RhythmJudge& HiFiRushStatics::GetRhythmJudge()
	{
		return GetGameInstance().GetRhythmJudge();
	}

	PlayerRuntimeState& HiFiRushStatics::GetPlayerRuntimeState()
	{
		return GetGameInstance().GetPlayerRuntimeState();
	}

#if GM_ENABLE_DEBUG_TOOLS
	EffectPresets& HiFiRushStatics::GetMutableEffectPresets()
	{
		return GetGameInstance().GetMutableEffectPresets();
	}

	DebugEventPublisher& HiFiRushStatics::GetDebugEventPublisher()
	{
		return GetGameInstance().GetDebugEventPublisher();
	}
#endif
}
