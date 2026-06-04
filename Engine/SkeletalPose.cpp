#include "SkeletalPose.h"
#include "SkeletalAnimationClip.h"
#include "SkeletalMesh.h"
#include "MathUtil.h"

#include <algorithm>

namespace gm
{
	namespace
	{
		Matrix BlendMatrix(const Matrix& from, const Matrix& to, float ratio)
		{
			Matrix fromMatrix = from;
			Vector3 fromScale{};
			Quaternion fromRotation{};
			Vector3 fromPosition{};
			fromMatrix.Decompose(fromScale, fromRotation, fromPosition);

			Matrix toMatrix = to;
			Vector3 toScale{};
			Quaternion toRotation{};
			Vector3 toPosition{};
			toMatrix.Decompose(toScale, toRotation, toPosition);

			const Vector3 scale = Vector3::Lerp(fromScale, toScale, ratio);
			const Quaternion rotation = Quaternion::Slerp(fromRotation, toRotation, ratio);
			const Vector3 position = Vector3::Lerp(fromPosition, toPosition, ratio);

			return Math::CreateTransformMatrix(position, rotation, scale);
		}

		uint32 FindKeyFrameIndex(const std::vector<KeyFrameData>& keyFrames, float trackPosition)
		{
			for (uint32 i = 0; i + 1 < keyFrames.size(); ++i)
			{
				if (trackPosition < keyFrames[i + 1].trackPosition)
					return i;
			}

			return static_cast<uint32>(keyFrames.size() - 1);
		}

		KeyFrameData SampleKeyFrame(const AnimationChannelData& channel, float trackPosition)
		{
			const std::vector<KeyFrameData>& keyFrames = channel.keyFrames;
			if (keyFrames.empty())
				return KeyFrameData{};

			if (keyFrames.size() == 1 || trackPosition <= keyFrames.front().trackPosition)
				return keyFrames.front();

			const uint32 currentIndex = FindKeyFrameIndex(keyFrames, trackPosition);
			if (currentIndex + 1 >= keyFrames.size())
				return keyFrames.back();

			const KeyFrameData& current = keyFrames[currentIndex];
			const KeyFrameData& next = keyFrames[currentIndex + 1];
			const float frameLength = next.trackPosition - current.trackPosition;
			const float ratio = (trackPosition - current.trackPosition) / frameLength;

			KeyFrameData result{};
			result.scale = Vector3::Lerp(current.scale, next.scale, ratio);
			result.rotation = Quaternion::Slerp(current.rotation, next.rotation, ratio);
			result.position = Vector3::Lerp(current.position, next.position, ratio);
			result.trackPosition = trackPosition;
			return result;
		}
	}

	SkeletalPoseApplyResult SkeletalPose::ApplyAnimation(const SkeletalMesh& skeletalMesh, const SkeletalAnimationClip& clip, float playTime, int32 rootMotionBoneIndex)
	{
		SkeletalPoseApplyResult result{};
		const std::vector<BoneData>& bones = skeletalMesh.GetBones();
		if (bones.empty())
		{
			_boneModelMatrices.clear();
			return result;
		}

		const float ticksPerSecond = clip.GetTicksPerSecond();
		const float trackPosition = playTime * ticksPerSecond;

		std::vector<Matrix> localMatrices(bones.size());
		for (uint32 boneIndex = 0; boneIndex < bones.size(); ++boneIndex)
		{
			localMatrices[boneIndex] = bones[boneIndex].transform;
		}

		for (const AnimationChannelData& channel : clip.GetChannels())
		{
			KeyFrameData keyFrame = SampleKeyFrame(channel, trackPosition);

			if (static_cast<int32>(channel.boneIndex) == rootMotionBoneIndex)
			{
				result.rootMotionPosition = keyFrame.position;
				result.hasRootMotion = true;
				keyFrame.position = Vector3{};
			}

			localMatrices[channel.boneIndex] = Math::CreateTransformMatrix(keyFrame.position, keyFrame.rotation, keyFrame.scale);
		}

		_boneModelMatrices.assign(bones.size(), Matrix::CreateScale(1.f));
		for (uint32 boneIndex = 0; boneIndex < bones.size(); ++boneIndex)
		{
			const BoneData& bone = bones[boneIndex];
			if (bone.parentBoneIndex >= 0)
				_boneModelMatrices[boneIndex] = localMatrices[boneIndex] * _boneModelMatrices[bone.parentBoneIndex];
			else
				_boneModelMatrices[boneIndex] = localMatrices[boneIndex];
		}

		return result;
	}

	void SkeletalPose::RebuildBindPose(const SkeletalMesh& skeletalMesh)
	{
		const std::vector<BoneData>& bones = skeletalMesh.GetBones();
		_boneModelMatrices.assign(bones.size(), Matrix::CreateScale(1.f));

		for (uint32 boneIndex = 0; boneIndex < bones.size(); ++boneIndex)
		{
			const BoneData& bone = bones[boneIndex];
			if (bone.parentBoneIndex >= 0)
				_boneModelMatrices[boneIndex] = bone.transform * _boneModelMatrices[bone.parentBoneIndex];
			else
				_boneModelMatrices[boneIndex] = bone.transform;
		}
	}

	void SkeletalPose::BlendFrom(const std::vector<Matrix>& fromBoneModelMatrices, float ratio)
	{
		const float blendRatio = std::clamp(ratio, 0.f, 1.f);
		for (uint32 boneIndex = 0; boneIndex < _boneModelMatrices.size(); ++boneIndex)
			_boneModelMatrices[boneIndex] = BlendMatrix(fromBoneModelMatrices[boneIndex], _boneModelMatrices[boneIndex], blendRatio);
	}
}
