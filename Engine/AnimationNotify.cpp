#include "AnimationNotify.h"

namespace gm
{
	void AnimationNotifyDispatcher::Reset(float currentTime)
	{
		_previousTime = currentTime;
	}

	void AnimationNotifyDispatcher::Dispatch(const std::vector<AnimationNotifyEvent>& clipNotifyEvents, float currentTime, float clipLength)
	{
		for (const AnimationNotifyEvent& notifyEvent : clipNotifyEvents)
		{
			if (HasPassedNotifyTime(currentTime, notifyEvent.time, clipLength) == false)
				continue;

			OnNotify.Publish(notifyEvent);
		}

		_previousTime = currentTime;
	}

	bool AnimationNotifyDispatcher::HasPassedNotifyTime(float currentTime, float notifyTime, float clipLength) const
	{
		// 루프가 발생하여 현재 시간이 이전 시간보다 작아진 경우 두 구간을 검사합니다.
		if (_previousTime > currentTime)
			return (_previousTime <= notifyTime && notifyTime < clipLength) || (0.f <= notifyTime && notifyTime < currentTime);

		// currentTime < clipLength는 AnimationController에서 보장합니다.
		return _previousTime <= notifyTime && notifyTime < currentTime;
	}
}
