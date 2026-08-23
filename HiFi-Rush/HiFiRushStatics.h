#pragma once

namespace gm
{
	class BeatSystem;
	class ChiAnimationSettings;
	class EffectPresets;
	class RhythmJudge;
#if GM_ENABLE_DEBUG_TOOLS
	class DebugEventPublisher;
#endif
	class HiFiRushGameInstance;
	struct PlayerRuntimeState;

	class HiFiRushStatics
	{
	public:
		static HiFiRushGameInstance&				GetGameInstance();
		static BeatSystem&						GetBeatSystem();
		static const ChiAnimationSettings&	GetChiAnimationSettings();
		static const EffectPresets&			GetEffectPresets();
		static const RhythmJudge&				GetRhythmJudge();
		static PlayerRuntimeState&				GetPlayerRuntimeState();
#if GM_ENABLE_DEBUG_TOOLS
		static EffectPresets&				GetMutableEffectPresets();
		static DebugEventPublisher&		GetDebugEventPublisher();
#endif
	};
}
