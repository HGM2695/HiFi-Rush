#pragma once

#include "BoundingTypes.h"
#include "MathTypes.h"
#include "MeshTypes.h"
#include "Types.h"
#include "VertexTypes.h"

#include <vector>

namespace gm
{
	enum class ModelType
	{
		Static,
		Skeletal
	};

	struct ModelData
	{
		ModelType type = ModelType::Static;
		Matrix preTransform = Matrix::CreateScale(1.f);
		BoundingVolume localBounds{};

		std::vector<VertexMesh>				vertices;
		std::vector<uint32>					indices;
		std::vector<MeshSection>			sections;
		std::vector<MeshTextureSet>			textureSets;

		std::vector<VertexAnimationMesh>		skinnedVertices;
		std::vector<BoneData>					bones;
		std::vector<SkeletalAnimationClipData>	animations;
	};
}
