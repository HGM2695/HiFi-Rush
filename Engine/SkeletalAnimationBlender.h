#pragma once

#include "MathTypes.h"

#include <vector>

namespace gm
{
	class SkeletalPose;

	class SkeletalAnimationBlender
	{
	public:
		void BeginBlend(const SkeletalPose& currentPose, float duration);
		void Tick(float deltaTime, SkeletalPose& pose);
		void Reset();
		bool IsBlending() const { return _duration > 0.f && _fromBoneModelMatrices.empty() == false; }

	private:
		std::vector<Matrix>	_fromBoneModelMatrices;
		float				_elapsed = 0.f;
		float				_duration = 0.f;
	};
}
