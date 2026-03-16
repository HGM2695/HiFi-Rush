#include "SpriteAnimationClip.h"
#include "Application.h"
#include "GMAssert.h"
#include "Resources.h"
#include "Texture.h"
#include <algorithm>
#include <cmath>

namespace gm
{
	SpriteAnimationClip::SpriteAnimationClip() = default;
	SpriteAnimationClip::~SpriteAnimationClip() = default;

	void SpriteAnimationClip::AddFrame(const SpriteFrame& frame)
	{
		_frameList.push_back(frame);
		_frameCount = static_cast<uint32_t>(_frameList.size());
		_length += frame.duration;
	}

	uint32_t SpriteAnimationClip::GetFrameIndexByTime(float time) const
	{
		GM_ASSERT_RETURN_VAL(_frameList.empty() == false, 0, "SpriteAnimationClip이 비어있습니다.");
		GM_ASSERT_RETURN_VAL(_length > 0.f, 0, "Animation _length가 0입니다.");

		// 각 프레임은 [start, end) 형태의 시간 구간을 가집니다.
		const float normalizedTime = std::fmod(time, _length);
		float accumulatedTime = 0.f;

		for (uint32_t i = 0; i < _frameCount; ++i)
		{
			accumulatedTime += _frameList[i].duration;
			if (normalizedTime < accumulatedTime)
				return i;
		}

		return _frameCount - 1;
	}

	bool SpriteAnimationClip::LoadInternal(const std::wstring& path)
	{
		_length = 0.f;
		_frameCount = 0;
		_frameList.clear();

		// 추후 Animation Data 파일로 부터 파싱하는 구조로 변경 예정
		if (path == L"TestSpriteIdleLeft")
		{
			_texture = APPLICATION.GetResources().Find<Texture>(L"PlayerLeft");
			GM_ASSERT_RETURN_VAL(_texture, false, "SpriteAnimationClip이 사용하는 Texture가 존재하지 않습니다.");

			const int frameWidth = static_cast<int>(_texture->GetWidth() / 4);
			const int frameHeight = static_cast<int>(_texture->GetHeight() / 9);

			for (int i = 0; i < 3; ++i)
				AddFrame({frameWidth * i, 0, frameWidth, frameHeight, 0.5f });
			AddFrame({ frameWidth, 0, frameWidth, frameHeight, 0.5f });
		}
		else if (path == L"TestSpriteMoveLeft")
		{
			_texture = APPLICATION.GetResources().Find<Texture>(L"PlayerLeft");
			GM_ASSERT_RETURN_VAL(_texture, false, "SpriteAnimationClip이 사용하는 Texture가 존재하지 않습니다.");

			const int frameWidth = static_cast<int>(_texture->GetWidth() / 4);
			const int frameHeight = static_cast<int>(_texture->GetHeight() / 9);

			for (int i = 0; i < 3; ++i)
				AddFrame({ frameWidth * i, frameHeight, frameWidth, frameHeight, 0.3f });
			AddFrame({ frameWidth, frameHeight, frameWidth, frameHeight, 0.3f });
		}
		if (path == L"TestSpriteIdleRight")
		{
			_texture = APPLICATION.GetResources().Find<Texture>(L"PlayerRight");
			GM_ASSERT_RETURN_VAL(_texture, false, "SpriteAnimationClip이 사용하는 Texture가 존재하지 않습니다.");

			const int frameWidth = static_cast<int>(_texture->GetWidth() / 4);
			const int frameHeight = static_cast<int>(_texture->GetHeight() / 9);

			for (int i = 0; i < 3; ++i)
				AddFrame({ frameWidth * i, 0, frameWidth, frameHeight, 0.5f });
			AddFrame({ frameWidth, 0, frameWidth, frameHeight, 0.5f });
		}
		else if (path == L"TestSpriteMoveRight")
		{
			_texture = APPLICATION.GetResources().Find<Texture>(L"PlayerRight");
			GM_ASSERT_RETURN_VAL(_texture, false, "SpriteAnimationClip이 사용하는 Texture가 존재하지 않습니다.");

			const int frameWidth = static_cast<int>(_texture->GetWidth() / 4);
			const int frameHeight = static_cast<int>(_texture->GetHeight() / 9);

			for (int i = 0; i < 3; ++i)
				AddFrame({ frameWidth * i, frameHeight, frameWidth, frameHeight, 0.3f });
			AddFrame({ frameWidth, frameHeight, frameWidth, frameHeight, 0.3f });
		}

		return _frameCount > 0 && _length > 0;
	}
}
