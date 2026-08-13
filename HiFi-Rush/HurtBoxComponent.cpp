#include "HurtBoxComponent.h"
#include "Collider3DComponent.h"
#include "GameObject.h"
#include "HealthComponent.h"

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

		_health = GetOwner().GetComponent<HealthComponent>();
		GM_ASSERT_RETURN(_health, "HurtBox를 가진 GameObject에는 HealthComponent가 필요합니다.");
	}

	DamageResult HurtBoxComponent::ReceiveHit(const HitEvent& event)
	{
		HitEvent receivedEvent = event;

		if (_health != nullptr && _health->IsEnabled())
			receivedEvent.damageResult = _health->ApplyDamage(receivedEvent);

		OnHurt.Publish(receivedEvent);
		return receivedEvent.damageResult;
	}
}
