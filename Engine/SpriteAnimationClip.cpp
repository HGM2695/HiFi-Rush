#include "SpriteAnimationClip.h"
#include "Application.h"
#include "Resources.h"
#include "Texture.h"
#include <algorithm>
#include <cmath>

namespace gm
{
	std::shared_ptr<SpriteAnimationClip> SpriteAnimationClip::Create(const SpriteAnimationClipDesc& desc)
	{
		// TODO: 추후 Animation Data 파일 파싱 구조로 변경.
		// 현재 SpriteAnimationClip은 코드에서 직접 구성한 뒤 Resources::Add로 등록한다.

		return std::shared_ptr<SpriteAnimationClip>(new SpriteAnimationClip());
	}

	SpriteAnimationClip::SpriteAnimationClip() = default;
	SpriteAnimationClip::~SpriteAnimationClip() = default;

	void SpriteAnimationClip::AddFrame(const SpriteFrame& frame)
	{
		_frameList.push_back(frame);
		_frameCount = static_cast<uint32>(_frameList.size());
		_length += frame.duration;
	}

	uint32 SpriteAnimationClip::GetFrameIndexByTime(float time) const
	{
		GM_ASSERT_RETURN_VAL(_frameList.empty() == false, 0, "SpriteAnimationClip이 비어있습니다.");
		GM_ASSERT_RETURN_VAL(_length > 0.f, 0, "Animation _length가 0입니다.");

		// 각 프레임은 [start, end) 형태의 시간 구간을 가집니다.
		const float normalizedTime = std::fmod(time, _length);
		float accumulatedTime = 0.f;

		for (uint32 i = 0; i < _frameCount; ++i)
		{
			accumulatedTime += _frameList[i].duration;
			if (normalizedTime < accumulatedTime)
				return i;
		}

		return _frameCount - 1;
	}
}
