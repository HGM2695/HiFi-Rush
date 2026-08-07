#pragma once

#include "EnvironmentComponentTypes.h"
#include "MathTypes.h"
#include "Types.h"

#include <vector>

namespace gm
{
	enum class EnvironmentRenderType : uint32
	{
		Opaque = 2,
		InOrderBlend = 4,
		AfterEdge = 5,
	};

	struct EnvironmentObjectData
	{
		std::vector<EnvironmentComponentData>	components{};
		EnvironmentRenderType					renderType = EnvironmentRenderType::Opaque;
		uint32									modelIndex = 0;
		Matrix									world = Matrix::Identity;
	};

	struct MapData
	{
		std::vector<EnvironmentObjectData>	objects;
	};
}
