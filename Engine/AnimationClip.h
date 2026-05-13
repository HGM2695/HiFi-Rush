#pragma once

#include "EngineCore.h"
#include "AnimationNotify.h"
#include "Resource.h"
#include <vector>

namespace gm
{
	// 실제 프레임, 키프레임, 본 트랙 같은 재생 데이터는 SpriteAnimationClip 등 구체 Clip 클래스가 직접 관리합니다.
	class AnimationClip : public Resource
	{
	public:
		float										GetLength() const { return _length; }
		uint32										GetFrameCount() const { return _frameCount; }
		bool										IsLoop() const { return _isLoop; }
		const std::vector<AnimationNotifyEvent>&	GetNotifyEvents() const { return _notifyEvents; }

		void										AddNotify(float time, const std::wstring& name);
		void										SetLoop(bool isLoop) { _isLoop = isLoop; }

	protected:
		float								_length = 0.f;
		uint32								_frameCount = 0;
		bool								_isLoop = true;
		std::vector<AnimationNotifyEvent>	_notifyEvents;
	};
}
