#include "SkinningRenderData.h"
#include "MeshTypes.h"
#include <algorithm>

namespace gm
{
	BonePaletteConstantVS BuildBonePalette(const MeshSection& section, const std::vector<Matrix>& boneModelMatrices)
	{
		BonePaletteConstantVS bonePalette{};
		for (Matrix& matrix : bonePalette.boneMatrices)
			matrix = Matrix::CreateScale(1.f);

		const uint32 paletteCount = std::min<uint32>(MaxSkinningBoneCount, static_cast<uint32>(section.boneIndices.size()));
		for (uint32 paletteIndex = 0; paletteIndex < paletteCount; ++paletteIndex)
		{
			const uint32 skeletonBoneIndex = section.boneIndices[paletteIndex];
			bonePalette.boneMatrices[paletteIndex] = section.offsetMatrices[paletteIndex] * boneModelMatrices[skeletonBoneIndex];
		}
		return bonePalette;
	}
}
