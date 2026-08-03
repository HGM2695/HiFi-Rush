#pragma once

#include "Event.h"

#include <string>
#include <vector>

namespace gm
{
	struct AnimationNotifyEvent final : EventType
	{
		float			time = 0.f;
		std::wstring	name;
	};

	class AnimationNotifyDispatcher
	{
	public:
		void Reset(float currentTime = 0.f);
		void Dispatch(const std::vector<AnimationNotifyEvent>& clipNotifyEvents, float currentTime, float clipLength);

	private:
		bool HasPassedNotifyTime(float currentTime, float notifyTime, float clipLength) const;

	public:
		EventPublisher<AnimationNotifyDispatcher, AnimationNotifyEvent> OnNotify;

	private:
		float _previousTime = 0.f;
	};
}
