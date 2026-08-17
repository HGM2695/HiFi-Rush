#include "RhythmInputJudge.h"
#include "BeatSystem.h"
#include "GMAssert.h"

#include <cmath>

namespace gm
{
	RhythmJudge::RhythmJudge(const RhythmJudgeSettings& settings)
		: _settings(settings)
	{
		GM_ASSERT(_settings.perfectWindowSeconds >= 0.f, "Perfect 판정 범위는 0 이상이어야 합니다.");
		GM_ASSERT(_settings.goodWindowSeconds >= _settings.perfectWindowSeconds, "Good 판정 범위는 Perfect 판정 범위 이상이어야 합니다.");
	}

	RhythmJudgeResult RhythmJudge::Judge(const BeatSystem& beatSystem, RhythmInputType inputType) const
	{
		RhythmJudgeResult command{};
		command.type = inputType;
		const float secondsPerBeat = beatSystem.GetSecondsPerBeat();
		const float inputOffsetBeats = secondsPerBeat > 0.f ? _settings.inputOffsetSeconds / secondsPerBeat : 0.f;
		command.inputBeat = beatSystem.GetCurrentBeat() + inputOffsetBeats;
		command.judgedBeatIndex = static_cast<int64>(std::round(command.inputBeat));
		command.beatError = command.inputBeat - static_cast<float>(command.judgedBeatIndex);
		command.secError = command.beatError * secondsPerBeat;

		const float absoluteErrorSeconds = std::abs(command.secError);
		if (absoluteErrorSeconds <= _settings.perfectWindowSeconds)
			command.judgeGrade = RhythmJudgeGrade::Perfect;
		else if (absoluteErrorSeconds <= _settings.goodWindowSeconds)
			command.judgeGrade = RhythmJudgeGrade::Good;
		else
			command.judgeGrade = RhythmJudgeGrade::OffBeat;

		return command;
	}

}
