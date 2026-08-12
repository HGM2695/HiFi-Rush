#include "HurtBoxComponent.h"
#include "Collider3DComponent.h"

#include <utility>

namespace gm
{
	HurtBoxComponent::HurtBoxComponent(Collider3DComponent& collider)
		: _collider(collider)
	{
		_collider.SetTrigger(true);
	}

	void HurtBoxComponent::OnInitialize()
	{
		GM_ASSERT_RETURN(&_collider.GetOwner() == &GetOwner(), "HurtBox와 Collider는 같은 GameObject에 속해야 합니다.");
	}

	void HurtBoxComponent::ReceiveHit(const HitEvent& event)
	{
		// HitBox를 기준으로 들어온 Contanct를 HurtBox 기준으로 변경
		HitEvent receivedEvent = event;
		std::swap(receivedEvent.contact.selfPoint, receivedEvent.contact.otherPoint);
		receivedEvent.contact.normal = -receivedEvent.contact.normal;
		OnHurt.Publish(receivedEvent);
	}
}
