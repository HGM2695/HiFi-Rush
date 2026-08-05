#pragma once

#include "MathUtil.h"

#include <cmath>

namespace gm::BeatMath
{
	inline float EvaluateCycleProgress(float currentBeat, float cycleBeats, float phaseOffsetBeats = 0.f)
	{
		if (cycleBeats <= 0.f)
			return 0.f;

		float cycleBeat = std::fmod(currentBeat + phaseOffsetBeats, cycleBeats);
		if (cycleBeat < 0.f)
			cycleBeat += cycleBeats;

		return cycleBeat / cycleBeats;
	}

	inline float EvaluateSinePulse(float currentBeat, float cycleBeats, float phaseOffsetBeats = 0.f)
	{
		return std::sin(Math::GM_PI * EvaluateCycleProgress(currentBeat, cycleBeats, phaseOffsetBeats));
	}
}
