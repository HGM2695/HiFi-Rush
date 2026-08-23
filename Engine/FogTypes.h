#pragma once

#include "MathTypes.h"

namespace gm
{
	struct DepthFogSettings
	{
		Color	fogColor{ 0.35f, 0.35f, 0.42f, 1.f };
		float	startDistance = 1.f;
		float	endDistance = 50.f;
		float	density = 2.f;
		bool	enabled = false;
	};
}
