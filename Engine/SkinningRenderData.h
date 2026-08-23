#pragma once

#include "MathTypes.h"
#include "Types.h"
#include <array>
#include <vector>

namespace gm
{
	struct MeshSection;

	inline constexpr uint32 MaxSkinningBoneCount = 512;

	struct BonePaletteConstantVS
	{
		std::array<Matrix, MaxSkinningBoneCount> boneMatrices;
	};

	BonePaletteConstantVS BuildBonePalette(const MeshSection& section, const std::vector<Matrix>& boneModelMatrices);
}
