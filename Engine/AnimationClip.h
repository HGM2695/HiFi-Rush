#pragma once

#include "EngineCore.h"
#include "AnimationNotify.h"
#include "Resource.h"
#include <vector>

namespace gm
{
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
		virtual bool								Load(const std::wstring& path) override = 0;

	protected:
		float								_length = 0.f;
		uint32								_frameCount = 0;
		bool								_isLoop = true;
		std::vector<AnimationNotifyEvent>	_notifyEvents;
	};
}
