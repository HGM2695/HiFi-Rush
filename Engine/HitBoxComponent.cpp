#include "HitBoxComponent.h"
#include "Collider3DComponent.h"
#include "GameObject.h"
#include "HurtBoxComponent.h"

#include <algorithm>

namespace gm
{
	HitBoxComponent::HitBoxComponent(Collider3DComponent& collider)
		: _collider(collider)
	{
		_collider.SetTrigger(true);
		_collider.SetEnabled(false);
	}

	void HitBoxComponent::BeginAttack()
	{
		_hitTargets.clear();
		_isAttackActive = true;
		_collider.SetEnabled(true);
	}

	void HitBoxComponent::EndAttack()
	{
		_isAttackActive = false;
		_collider.SetEnabled(false);
		_hitTargets.clear();
	}

	void HitBoxComponent::OnInitialize()
	{
		GM_ASSERT_RETURN(&_collider.GetOwner() == &GetOwner(), "HitBox와 Collider는 같은 GameObject에 속해야 합니다.");

		_collider.OnCollisionEnter.Subscribe(_collisionEnterConnection,
			[this](const Collision3DEvent& event)
			{
				HandleCollisionEvent(event);
			});

		_collider.OnCollisionStay.Subscribe(_collisionStayConnection,
			[this](const Collision3DEvent& event)
			{
				HandleCollisionEvent(event);
			});
	}

	void HitBoxComponent::HandleCollisionEvent(const Collision3DEvent& event)
	{
		if (_isAttackActive == false || IsEnabled() == false || event.type != CollisionType::Trigger || event.otherCollider == nullptr)
			return;

		const WeakGameObjectPtr target = event.otherCollider->GetOwner().GetWeakPtr();
		if (IsAlreadyHit(target))
			return;

		HurtBoxComponent* hurtBox = FindHurtBox(*event.otherCollider);
		if (hurtBox == nullptr || hurtBox->IsEnabled() == false)
			return;

		_hitTargets.push_back(target);

		HitEvent hitEvent{};
		hitEvent.hitBox = this;
		hitEvent.hurtBox = hurtBox;
		hitEvent.contact = event.contact;
		hurtBox->ReceiveHit(hitEvent);
		OnHit.Publish(hitEvent);
	}

	HurtBoxComponent* HitBoxComponent::FindHurtBox(Collider3DComponent& collider) const
	{
		HurtBoxComponent* result = nullptr;
		collider.GetOwner().ForEachComponent([&collider, &result](Component& component)
		{
			if (result != nullptr)
				return;

			auto* hurtBox = dynamic_cast<HurtBoxComponent*>(&component);
			if (hurtBox != nullptr && &hurtBox->GetCollider() == &collider)
				result = hurtBox;
		});

		return result;
	}

	bool HitBoxComponent::IsAlreadyHit(const WeakGameObjectPtr& target) const
	{
		return std::find(_hitTargets.begin(), _hitTargets.end(), target) != _hitTargets.end();
	}
}
