#include "HurtBoxComponent.h"
#include "Collider3DComponent.h"
#include "GameObject.h"
#include "HealthComponent.h"

namespace gm
{
	HurtBoxComponent::HurtBoxComponent(const std::wstring& colliderId)
		: _colliderId(colliderId)
	{
		GM_ASSERT(colliderId.empty() == false, "HurtBox가 참조할 Collider ID는 비어 있을 수 없습니다.");
	}

	void HurtBoxComponent::OnInitialize()
	{
		const std::vector<Collider3DComponent*>& colliders = GetOwner().GetColliders3D();
		for (Collider3DComponent* collider : colliders)
		{
			if (collider->GetColliderId() != _colliderId)
				continue;

			GM_ASSERT_RETURN(_collider == nullptr, "HurtBox가 참조하는 Collider ID가 중복되었습니다. colliderId=%ls", _colliderId.c_str());
			_collider = collider;
		}

		GM_ASSERT_RETURN(_collider, "HurtBox가 참조하는 Collider를 찾을 수 없습니다. colliderId=%ls", _colliderId.c_str());

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
