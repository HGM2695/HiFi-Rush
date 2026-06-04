#pragma once

#include "MathTypes.h"
#include "Types.h"

#include <vector>

namespace gm
{
	class SkeletalAnimationClip;
	class SkeletalMesh;

	struct SkeletalPoseApplyResult
	{
		Vector3 rootMotionPosition{};
		bool	hasRootMotion = false;
	};

	class SkeletalPose
	{
	public:
		void						RebuildBindPose(const SkeletalMesh& skeletalMesh);
		SkeletalPoseApplyResult		ApplyAnimation(const SkeletalMesh& skeletalMesh, const SkeletalAnimationClip& clip, float playTime, int32 rootMotionBoneIndex = -1);
		void						BlendFrom(const std::vector<Matrix>& fromBoneModelMatrices, float ratio);

		const std::vector<Matrix>&	GetBoneModelMatrices() const { return _boneModelMatrices; }
		bool						IsValid() const { return _boneModelMatrices.empty() == false; }
		void						Clear() { _boneModelMatrices.clear(); }

	private:
		std::vector<Matrix>			_boneModelMatrices;
	};
}
