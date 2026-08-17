#pragma once

#include "ChiAnimationTypes.h"
#include "MathTypes.h"

#include <array>
#include <string>

namespace gm
{
	inline constexpr wchar_t ChiAnimationSettingsFilePath[] = L"Data/ChiAnimationSettings.bin";

	struct ChiAnimationSetting
	{
		bool	lockInputMovement = false;
		bool	useGravity = true;
		Vector3	rootMotionWeight{ 1.f, 1.f, 1.f };

		float	blendDuration = 0.1f;
		float	playbackStartBeats = 0.2f;

		float	impactMarkerBeat = 0.f;
		float	maxPlaybackRateScale = 1.25f;
		float	actionCancelStartBeats = 0.f;
	};

	class ChiAnimationSettings
	{
	public:
		bool						Load(const std::wstring& filePath);
		const ChiAnimationSetting&	Get(ChiAnimationClipId animationClipId) const { return _settings[ToIndex(animationClipId)]; }

	private:
		std::array<ChiAnimationSetting, ChiAnimationClipIdCount> _settings{};
	};
}
