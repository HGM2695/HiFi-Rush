#pragma once

#include "MathUtil.h"

#include <cmath>

namespace gm::BeatMath
{
	inline float EvaluateSinePulse(float currentBeat, float cycleBeats, float phaseOffsetBeats = 0.f)
	{
		if (cycleBeats <= 0.f)
			return 0.f;

		float cycleBeat = std::fmod(currentBeat + phaseOffsetBeats, cycleBeats);
		if (cycleBeat < 0.f)
			cycleBeat += cycleBeats;

		return std::sin(Math::GM_PI * (cycleBeat / cycleBeats));
	}
}
