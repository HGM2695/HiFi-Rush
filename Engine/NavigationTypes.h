#pragma once

#include "MathTypes.h"
#include "Types.h"

#include <array>
#include <vector>

namespace gm
{
	enum class NavigationCellType
	{
		Normal,
		Jump,
		Dead,
		Dummy,
	};

	struct NavigationCellData
	{
		static constexpr uint32 PointCount = 3;
		static constexpr uint32 EdgeCount = 3;

		std::array<Vector3, PointCount>				points{};
		std::array<int32, EdgeCount>				neighborIndices{ -1, -1, -1 };
		std::array<NavigationCellType, EdgeCount>	neighborTypes{ NavigationCellType::Dummy, NavigationCellType::Dummy, NavigationCellType::Dummy };
		NavigationCellType							type = NavigationCellType::Normal;
	};

	struct NavigationMeshData
	{
		std::vector<NavigationCellData> cells;
	};
}
