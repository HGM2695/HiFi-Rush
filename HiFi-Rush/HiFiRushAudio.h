#pragma once

namespace gm
{
	struct RhythmBGMDesc
	{
		const wchar_t*	commonResourceKey = nullptr;
		const wchar_t*	fileName = nullptr;
		float			bpm = 0.f;
		float			songOffsetSeconds = 0.f;
		float			volume = 1.f;
	};

	namespace HiFiRushBGM
	{
		inline constexpr RhythmBGMDesc Tutorial{ L"Tutorial.BGM", L"metroSound.mp3", 136.f, -0.220588f, 1.f };
		inline constexpr RhythmBGMDesc Outside{ L"Outside.BGM", L"OpenMapSound2.mp3", 136.f, 0.0857f, 1.f };
		inline constexpr RhythmBGMDesc Qamil{ L"Qamil.BGM", L"TooBigToFail.mp3", 136.f, 0.056f, 0.35f };
	}

	void PlayRhythmBGM(const RhythmBGMDesc& desc);
}
