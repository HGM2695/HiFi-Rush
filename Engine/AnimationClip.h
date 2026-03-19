#pragma once

#include "AnimationNotify.h"
#include "Resource.h"
#include <vector>

namespace gm
{
	class AnimationClip : public Resource
	{
	public:
		virtual uint32_t GetFrameIndexByTime(float time) const = 0;

		float										GetLength() const { return _length; }
		uint32_t									GetFrameCount() const { return _frameCount; }
		bool										IsLoop() const { return _isLoop; }
		const std::vector<AnimationNotifyEvent>&	GetNotifyEvents() const { return _notifyEvents; }

		void	AddNotify(float time, const std::wstring& name);
		void	SetLoop(bool isLoop) { _isLoop = isLoop; }

	protected:
		virtual bool	LoadInternal(const std::wstring& path) = 0;

	protected:
		float								_length = 0.f;
		uint32_t							_frameCount = 0;
		bool								_isLoop = true;
		std::vector<AnimationNotifyEvent>	_notifyEvents;
	};
}
