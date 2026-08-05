#pragma once

#include "Types.h"

namespace gm::Math
{
	void	SetRandomSeed(uint32 seed);
	int32	RandomInt(int32 minInclusive, int32 maxInclusive);
	float	RandomFloat(float minValue, float maxValue);
}
