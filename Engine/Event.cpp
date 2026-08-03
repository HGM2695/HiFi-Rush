#include "Event.h"

namespace gm
{
	EventPublisherBase::EventPublisherBase()
		: _lifetimeToken(std::make_shared<int>(0))
	{
	}

	void EventPublisherBase::ResetLifetimeToken()
	{
		_lifetimeToken = std::make_shared<int>(0);
	}

	void EventConnection::Connect(EventPublisherBase* publisher, const std::weak_ptr<void>& lifetimeToken, uint64 listenerId)
	{
		_publisher = publisher;
		_lifetimeToken = lifetimeToken;
		_listenerId = listenerId;
	}

	void EventConnection::Disconnect()
	{
		if (_publisher && _lifetimeToken.lock())
			_publisher->RemoveListener(_listenerId);

		_publisher = nullptr;
		_lifetimeToken.reset();
		_listenerId = 0;
	}
}
