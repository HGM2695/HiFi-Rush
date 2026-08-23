#pragma once

#include "GameInstance.h"
#include "BeatSystem.h"
#include "ChiAnimationSettings.h"
#include "EffectPresets.h"
#include "PlayerRuntimeState.h"
#include "RhythmInputJudge.h"
#if GM_ENABLE_DEBUG_TOOLS
#include "DebugEventPublisher.h"
#endif

namespace gm
{
	class HiFiRushGameInstance final : public GameInstance
	{
	public:
		BeatSystem&						GetBeatSystem() { return _beatSystem; }
		const BeatSystem&				GetBeatSystem() const { return _beatSystem; }
		const ChiAnimationSettings&		GetChiAnimationSettings() const { return _chiAnimationSettings; }
		const EffectPresets&			GetEffectPresets() const { return _effectPresets; }
		const RhythmJudge&				GetRhythmJudge() const { return _rhythmJudge; }
		PlayerRuntimeState&				GetPlayerRuntimeState() { return _playerRuntimeState; }
		const PlayerRuntimeState&		GetPlayerRuntimeState() const { return _playerRuntimeState; }
#if GM_ENABLE_DEBUG_TOOLS
		EffectPresets& GetMutableEffectPresets() { return _effectPresets; }
		DebugEventPublisher& GetDebugEventPublisher() { return _debugEventPublisher; }
#endif

	protected:
		bool OnInitialize() override;
		void OnTick(float) override;

	private:
		void SetupScenes();
		void SetupDebugTools();

	private:
		BeatSystem					_beatSystem;
		ChiAnimationSettings		_chiAnimationSettings{};
		EffectPresets				_effectPresets{};
		RhythmJudge					_rhythmJudge{};
		PlayerRuntimeState			_playerRuntimeState{};
#if GM_ENABLE_DEBUG_TOOLS
		DebugEventPublisher	_debugEventPublisher;
#endif
	};
}
