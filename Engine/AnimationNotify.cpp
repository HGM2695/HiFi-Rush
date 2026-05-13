#include "AnimationNotify.h"
#include "GMAssert.h"
#include <algorithm>

namespace gm
{
	AnimationNotifyDispatcher::AnimationNotifyDispatcher()
		: _lifetimeToken(std::make_shared<int>(0))
	{
	}

	void AnimationNotifyDispatcher::Reset(float currentTime)
	{
		_previousTime = currentTime;
	}

	NotifyConnection AnimationNotifyDispatcher::BindNotifyListener(const AnimationNotifyListener& notifyListener)
	{
		const int id = _nextNotifyListenerId++;
		_notifyListeners.emplace_back(id, notifyListener);
		return NotifyConnection(this, _lifetimeToken, id);
	}

	void AnimationNotifyDispatcher::RemoveNotifyListener(int id)
	{
		_notifyListeners.erase(std::remove_if(_notifyListeners.begin(), _notifyListeners.end(),
				[id](const NotifyListenerEntry& entry)
				{
					return entry.id == id;
				}),
			_notifyListeners.end());
	}

	void AnimationNotifyDispatcher::Dispatch(const std::vector<AnimationNotifyEvent>& clipNotifyEvents, float currentTime, float clipLength)
	{
		for (const auto& notifyEvent : clipNotifyEvents)
		{
			if (HasPassedNotifyTime(currentTime, notifyEvent.time, clipLength) == false)
				continue;

			for (const auto& notifyListener : _notifyListeners)
				notifyListener.listener(notifyEvent.name);
		}

		_previousTime = currentTime;
	}

	bool AnimationNotifyDispatcher::HasPassedNotifyTime(float currentTime, float notifyTime, float clipLength) const
	{
		// 루프가 발생 하여, 현재 시간이 이전 시간이 작아진 경우 : [_previousTime, clipLength), [0, currentTime) 두 구간을 검사
		if (_previousTime > currentTime)
			return (_previousTime <= notifyTime && notifyTime < clipLength) || (0.f <= notifyTime && notifyTime < currentTime);

		// currentTime < _clipLength는 AnimationContoller에서 보장
		return _previousTime <= notifyTime && notifyTime < currentTime;
	}

	NotifyConnection& NotifyConnection::operator=(NotifyConnection&& other) noexcept
	{
		if (this == &other)
			return *this;

		Disconnect();

		_dispatcher = other._dispatcher;
		_lifetimeToken = std::move(other._lifetimeToken);
		_id = other._id;

		other._dispatcher = nullptr;
		return *this;
	}

	void NotifyConnection::Disconnect()
	{
		if (_dispatcher == nullptr)
			return;

		if (_lifetimeToken.lock())
			_dispatcher->RemoveNotifyListener(_id);

		_lifetimeToken.reset();
		_dispatcher = nullptr;
	}
}
