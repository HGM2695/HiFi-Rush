#pragma once

#include "AnimationTypes.h"
#include <memory>
#include <vector>

namespace gm
{
	struct AnimationNotifyEvent
	{
		float			time = 0.f;
		std::wstring	name;
	};

	class AnimationNotifyDispatcher
	{
	public:
		AnimationNotifyDispatcher();

		[[nodiscard]] 
		class NotifyConnection	BindNotifyListener(const AnimationNotifyListener& notifyListener);
		void					RemoveNotifyListener(int id);
		void					ClearNotifyListeners() { _notifyListeners.clear(); }

		void					Reset(float currentTime = 0.f);
		void					Dispatch(const std::vector<AnimationNotifyEvent>& clipNotifyEvents, float currentTime, float clipLength);

	private:
		bool					HasPassedNotifyTime(float currentTime, float notifyTime, float clipLength) const;

	private:
		struct NotifyListenerEntry
		{
			int id = -1;
			AnimationNotifyListener listener;
		};

		std::vector<NotifyListenerEntry>		_notifyListeners;
		float									_previousTime = 0.f;
		int										_nextNotifyListenerId = 0;
		// NotifyConnection이 디스패처 생존 여부를 확인할 수 있도록 shared lifetime token을 유지합니다.
		std::shared_ptr<void>					_lifetimeToken;
	};

	// BindNotifyListener 함수가 반환한 NotifyConnection이 소멸될 때,
	// 디스패처에 등록된 리스너가 자동으로 해제됩니다.
	class NotifyConnection
	{
	public:
		NotifyConnection() = default;
		NotifyConnection(AnimationNotifyDispatcher* dispatcher, const std::weak_ptr<void>& lifetimeToken, int id)
			: _dispatcher(dispatcher), _lifetimeToken(lifetimeToken), _id(id) {}
		~NotifyConnection() { Disconnect(); }

		NotifyConnection(const NotifyConnection&) = delete;
		NotifyConnection& operator=(const NotifyConnection&) = delete;

		NotifyConnection(NotifyConnection&& other) noexcept
			: _dispatcher(other._dispatcher), _lifetimeToken(std::move(other._lifetimeToken)), _id(other._id)
		{
			other._dispatcher = nullptr;
			other._id = -1;
		}

		NotifyConnection& operator=(NotifyConnection&& other) noexcept;

		void Disconnect();

	private:
		AnimationNotifyDispatcher*	_dispatcher = nullptr;
		// 디스패처가 먼저 파괴되었는지 확인하기 위한 weak lifetime token입니다.
		std::weak_ptr<void>			_lifetimeToken{};
		int							_id = -1;
	};
}
