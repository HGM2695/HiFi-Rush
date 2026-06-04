#include "SkeletalAnimationBlender.h"
#include "SkeletalPose.h"

namespace gm
{
	void SkeletalAnimationBlender::BeginBlend(const SkeletalPose& currentPose, float duration)
	{
		if (duration <= 0.f || currentPose.IsValid() == false)
		{
			Reset();
			return;
		}

		_fromBoneModelMatrices = currentPose.GetBoneModelMatrices();
		_elapsed = 0.f;
		_duration = duration;
	}

	void SkeletalAnimationBlender::Tick(float deltaTime, SkeletalPose& pose)
	{
		if (IsBlending() == false)
			return;

		_elapsed += deltaTime;
		const float ratio = _elapsed / _duration;
		pose.BlendFrom(_fromBoneModelMatrices, ratio);

		if (ratio >= 1.f)
			Reset();
	}

	void SkeletalAnimationBlender::Reset()
	{
		_fromBoneModelMatrices.clear();
		_elapsed = 0.f;
		_duration = 0.f;
	}
}
