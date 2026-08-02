#pragma once

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
		EnvironmentRenderType	renderType = EnvironmentRenderType::Opaque;
		uint32					modelIndex = 0;
		uint32					moveBeat = 0;
		int32					colorVariant = 0;
		Matrix					world = Matrix::Identity;
		Vector3					moveEndPosition{};
	};

	struct EnvironmentMapData
	{
		std::vector<EnvironmentObjectData> objects;
	};
}
