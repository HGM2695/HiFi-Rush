#pragma once

#include "Event.h"

namespace gm
{
	class BeatSystem;

	enum class RhythmInputType
	{
		WeakAttack,
		StrongAttack,
		Jump,
		Dash
	};

	enum class RhythmJudgeGrade
	{
		Perfect,
		Good,
		OffBeat
	};

	struct RhythmJudgeResult : EventType
	{
		RhythmInputType		type = RhythmInputType::WeakAttack;
		RhythmJudgeGrade	judgeGrade = RhythmJudgeGrade::OffBeat;
		float				inputBeat = 0.f;
		int64				judgedBeatIndex = 0;
		float				beatError = 0.f;
		float				secError = 0.f;
	};

	struct RhythmJudgeSettings
	{
		float perfectWindowSeconds = 0.045f;
		float goodWindowSeconds = 0.09f;
		float inputOffsetSeconds = 0.f;
	};

	class RhythmJudge
	{
	public:
		explicit RhythmJudge(const RhythmJudgeSettings& settings = {});

		void							SetInputOffset(float offsetSeconds) { _settings.inputOffsetSeconds = offsetSeconds; }
		RhythmJudgeResult				Judge(const BeatSystem& beatSystem, RhythmInputType inputType) const;
		bool							HasPassedInputDeadline(const BeatSystem& beatSystem, float targetBeat) const;
		float							GetRawInputBeat(const BeatSystem& beatSystem, const RhythmJudgeResult& result) const;

	private:
		float							CalculateInputBeat(const BeatSystem& beatSystem) const;
		float							CalculateInputOffsetBeats(const BeatSystem& beatSystem) const;

		RhythmJudgeSettings				_settings{};
	};

}
