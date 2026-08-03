#pragma once

#include "Types.h"

#include <algorithm>
#include <functional>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

namespace gm
{
	struct EventType
	{
	};

	template <typename TOwner, typename TEvent>
	class EventPublisher;

	class EventConnection;

	class EventPublisherBase
	{
	public:
		EventPublisherBase();
		virtual ~EventPublisherBase() = default;

		EventPublisherBase(const EventPublisherBase&) = delete;
		EventPublisherBase& operator=(const EventPublisherBase&) = delete;
		EventPublisherBase(EventPublisherBase&&) = delete;
		EventPublisherBase& operator=(EventPublisherBase&&) = delete;

	protected:
		std::weak_ptr<void>	GetLifetimeToken() const { return _lifetimeToken; }
		void				ResetLifetimeToken();

	private:
		friend class EventConnection;

		virtual void		RemoveListener(uint64 listenerId) = 0;

		std::shared_ptr<void> _lifetimeToken;
	};

	class EventConnection
	{
	public:
		EventConnection() = default;
		~EventConnection() { Disconnect(); }

		EventConnection(const EventConnection&) = delete;
		EventConnection& operator=(const EventConnection&) = delete;
		EventConnection(EventConnection&&) = delete;
		EventConnection& operator=(EventConnection&&) = delete;

		void Disconnect();

	private:
		template <typename TOwner, typename TEvent>
		friend class EventPublisher;

		void Connect(EventPublisherBase* publisher, const std::weak_ptr<void>& lifetimeToken, uint64 listenerId);

		EventPublisherBase*		_publisher = nullptr;
		std::weak_ptr<void>		_lifetimeToken;
		uint64					_listenerId = 0;
	};

	template <typename TOwner, typename TEvent>
	class EventPublisher final : private EventPublisherBase
	{
		friend TOwner;

	public:
		static_assert(std::is_class_v<TOwner>, "EventPublisher의 TOwner는 클래스 타입이어야 합니다.");
		static_assert(std::is_base_of_v<EventType, TEvent>, "EventPublisher의 TEvent는 EventType을 상속해야 합니다.");

		using Listener = std::function<void(const TEvent&)>;

	private:
		struct ListenerEntry
		{
			uint64		listenerId = 0;
			Listener	listener;
			bool		isActive = false;
		};

	public:
		// Connection이 살아 있는 동안 구독을 유지합니다.
		void Subscribe(EventConnection& connection, Listener listener)
		{
			connection.Disconnect();

			if (listener == nullptr)
				return;

			const uint64 listenerId = _nextListenerId++;
			_pendingListeners.push_back({ listenerId, std::move(listener), true });
			connection.Connect(this, GetLifetimeToken(), listenerId);
		}

	private:
		void Publish(const TEvent& event)
		{
			PrepareListeners();

			for (ListenerEntry& entry : _listeners)
			{
				if (entry.isActive)
					entry.listener(event);
			}
		}

		void Clear()
		{
			for (ListenerEntry& entry : _listeners)
				entry.isActive = false;

			_pendingListeners.clear();
			ResetLifetimeToken();
		}

		virtual void RemoveListener(uint64 listenerId) override
		{
			DeactivateListener(_listeners, listenerId);
			DeactivateListener(_pendingListeners, listenerId);
		}

		void DeactivateListener(std::vector<ListenerEntry>& listeners, uint64 listenerId)
		{
			const auto iter = std::find_if(listeners.begin(), listeners.end(),
				[listenerId](const ListenerEntry& entry)
				{
					return entry.listenerId == listenerId;
				});

			if (iter != listeners.end())
				iter->isActive = false;
		}

		void RemoveInactiveListeners(std::vector<ListenerEntry>& listeners)
		{
			listeners.erase(std::remove_if(listeners.begin(), listeners.end(),
				[](const ListenerEntry& entry)
				{
					return entry.isActive == false;
				}), listeners.end());
		}

		void PrepareListeners()
		{
			RemoveInactiveListeners(_listeners);
			RemoveInactiveListeners(_pendingListeners);

			for (ListenerEntry& entry : _pendingListeners)
				_listeners.push_back(std::move(entry));

			_pendingListeners.clear();
		}

	private:
		std::vector<ListenerEntry>	_listeners;
		std::vector<ListenerEntry>	_pendingListeners;
		uint64						_nextListenerId = 0;
	};
}
