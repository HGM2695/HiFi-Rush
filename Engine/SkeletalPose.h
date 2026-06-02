#pragma once

#include "MathTypes.h"

#include <vector>

namespace gm
{
	class SkeletalAnimationClip;
	class SkeletalMesh;

	class SkeletalPose
	{
	public:
		void						RebuildBindPose(const SkeletalMesh& skeletalMesh);
		void						ApplyAnimation(const SkeletalMesh& skeletalMesh, const SkeletalAnimationClip& clip, float playTime);

		const std::vector<Matrix>&	GetBoneModelMatrices() const { return _boneModelMatrices; }
		bool						IsValid() const { return _boneModelMatrices.empty() == false; }
		void						Clear() { _boneModelMatrices.clear(); }

	private:
		std::vector<Matrix>			_boneModelMatrices;
	};
}
