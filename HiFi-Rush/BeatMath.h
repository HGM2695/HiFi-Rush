#pragma once

#include "MathUtil.h"

#include <cmath>

namespace gm::BeatMath
{
	inline float CalcAnimationPlayRate(float currentBeat, float secondsPerBeat, float markerTime)
	{
		if (secondsPerBeat <= 0.f || markerTime <= 0.f)
			return 1.f;

		const float markerDurationBeats = markerTime / secondsPerBeat;
		float targetBeat = std::round(currentBeat + markerDurationBeats);
		if (targetBeat <= currentBeat)
			targetBeat = std::floor(currentBeat) + 1.f;

		const float secondsUntilTarget = (targetBeat - currentBeat) * secondsPerBeat;
		return markerTime / secondsUntilTarget;
	}

	inline float EvaluateCycleRatio(float currentBeat, float cycleBeats, float phaseOffsetBeats = 0.f)
	{
		if (cycleBeats <= 0.f)
			return 0.f;

		float cycleBeat = std::fmod(currentBeat + phaseOffsetBeats, cycleBeats);
		if (cycleBeat < 0.f)
			cycleBeat += cycleBeats;

		return cycleBeat / cycleBeats;
	}

	// 애니메이션 클립 전체 길이(초)
	inline float EvaluateCycleTime(float currentBeat, float cycleBeats, float duration, float phaseOffsetBeats = 0.f)
	{
		return duration * EvaluateCycleRatio(currentBeat, cycleBeats, phaseOffsetBeats);
	}

	inline float EvaluateSinePulse(float currentBeat, float cycleBeats, float phaseOffsetBeats = 0.f)
	{
		return std::sin(Math::GM_PI * EvaluateCycleRatio(currentBeat, cycleBeats, phaseOffsetBeats));
	}
}
