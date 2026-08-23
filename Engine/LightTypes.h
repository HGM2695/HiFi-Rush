#pragma once

#include "MathTypes.h"

namespace gm
{
	enum class LightType
	{
		Directional,
		Point,
		Spot,
		Count
	};

	enum class SpotConeFalloff
	{
		Smooth,
		ToonSteps,
		Count
	};

	static_assert(static_cast<uint32>(SpotConeFalloff::Smooth) == 0 && static_cast<uint32>(SpotConeFalloff::ToonSteps) == 1);

	struct SceneAmbientSettings
	{
		Color	ambientColor = Colors::White;
		float	ambientIntensity = 0.15f;
	};
}
