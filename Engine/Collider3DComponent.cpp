#include "Collider3DComponent.h"

namespace gm
{
	void Collider3DComponent::SetCollisionFilter(const CollisionFilter& filter)
	{
		GM_ASSERT_RETURN(IsSingleCollisionLayer(filter.layer), "Collision Layer는 하나의 비트만 설정할 수 있습니다.");
		_collisionFilter = filter;
	}

	void Collider3DComponent::SetCollisionLayer(CollisionLayer layer)
	{
		GM_ASSERT_RETURN(IsSingleCollisionLayer(layer), "Collision Layer는 하나의 비트만 설정할 수 있습니다.");
		_collisionFilter.layer = layer;
	}

	void Collider3DComponent::AddCollisionLayerToMask(CollisionLayer layer)
	{
		GM_ASSERT_RETURN(IsSingleCollisionLayer(layer), "Collision Mask에 추가할 Layer는 하나의 비트여야 합니다.");
		_collisionFilter.mask |= layer;
	}

	void Collider3DComponent::RemoveCollisionLayerFromMask(CollisionLayer layer)
	{
		GM_ASSERT_RETURN(IsSingleCollisionLayer(layer), "Collision Mask에서 제거할 Layer는 하나의 비트여야 합니다.");
		_collisionFilter.mask &= ~layer;
	}

	void Collider3DComponent::DispatchCollisionEvent(CollisionState state, CollisionType type, const CollisionContact& contact, Collider3DComponent& otherCollider)
	{
		Collision3DEvent event{};
		event.selfCollider = this;
		event.otherCollider = &otherCollider;
		event.type = type;
		event.contact = contact;

		switch (state)
		{
		case CollisionState::Enter:
			OnCollisionEnter.Publish(event);
			return;
		case CollisionState::Stay:
			OnCollisionStay.Publish(event);
			return;
		case CollisionState::Exit:
			OnCollisionExit.Publish(event);
			return;
		}
	}
}
