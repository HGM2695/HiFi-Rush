#pragma once

#include "GraphicsTypes.h"
#include "MathTypes.h"
#include "Types.h"
#include <array>
#include <string>
#include <vector>

namespace gm
{
	struct BoneData
	{
		std::wstring	name;
		Matrix			transform = Matrix::CreateScale(1.f);
		int32			parentBoneIndex = -1;
	};

	struct KeyFrameData
	{
		Vector3		scale = Vector3{ 1.f, 1.f, 1.f };
		Quaternion	rotation = Quaternion{ 0.f, 0.f, 0.f, 1.f };
		Vector3		position = Vector3{ 0.f, 0.f, 0.f };
		float		trackPosition = 0.f;
	};

	struct AnimationChannelData
	{
		std::wstring				name;
		uint32						boneIndex = 0;
		std::vector<KeyFrameData>	keyFrames;
	};

	struct SkeletalAnimationClipData
	{
		std::wstring						name;
		float								duration = 0.f;
		float								ticksPerSecond = 0.f;
		std::vector<AnimationChannelData>	channels;
	};

	struct MeshSection
	{
		std::wstring		name;
		uint32				indexStart = 0;
		uint32				indexCount = 0;
		uint32				textureSetIndex = 0;

		std::vector<uint32>	boneIndices;
		std::vector<Matrix>	offsetMatrices;
	};

	struct MeshTextureSet
	{
		std::array<std::wstring, TextureSlotCount> textureKeys{};
	};
}
