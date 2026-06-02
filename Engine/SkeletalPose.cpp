#include "SkeletalPose.h"
#include "SkeletalAnimationClip.h"
#include "SkeletalMesh.h"
#include "MathUtil.h"

#include <algorithm>

namespace gm
{
	namespace
	{
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

	void SkeletalPose::ApplyAnimation(const SkeletalMesh& skeletalMesh, const SkeletalAnimationClip& clip, float playTime)
	{
		const std::vector<BoneData>& bones = skeletalMesh.GetBones();
		if (bones.empty())
		{
			_boneModelMatrices.clear();
			return;
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
}
