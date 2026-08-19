#include "CollisionTriggerComponent.h"

#include "Collider3DComponent.h"
#include "GameObject.h"
#include "GameplayScene.h"
#include "HiFiRushCollisionLayers.h"
#include "TriggerSystem.h"

namespace gm
{
	CollisionTriggerComponent::CollisionTriggerComponent(const std::wstring& colliderId, const std::wstring& triggerId)
		: _colliderId(colliderId), _triggerId(triggerId)
	{
		GM_ASSERT(colliderId.empty() == false, "CollisionTriggerComponent의 Collider ID는 비어 있을 수 없습니다.");
		GM_ASSERT(triggerId.empty() == false, "CollisionTriggerComponent의 Trigger ID는 비어 있을 수 없습니다.");
	}

	void CollisionTriggerComponent::OnInitialize()
	{
		for (Collider3DComponent* collider : GetOwner().GetColliders3D())
		{
			if (collider->GetColliderId() == _colliderId)
			{
				_collider = collider;
				break;
			}
		}

		GM_ASSERT_RETURN(_collider, "CollisionTriggerComponent가 참조하는 Collider를 찾을 수 없습니다. colliderId=%ls", _colliderId.c_str());
		GM_ASSERT_RETURN(_collider->IsTrigger(), "CollisionTriggerComponent가 참조하는 Collider는 Trigger여야 합니다. colliderId=%ls", _colliderId.c_str());

		GameplayScene* scene = dynamic_cast<GameplayScene*>(GetOwner().GetScene());
		GM_ASSERT_RETURN(scene, "CollisionTriggerComponent는 GameplayScene에서만 사용할 수 있습니다.");
		_triggerSystem = &scene->GetTriggerSystem();

		_collider->OnCollisionEnter.Subscribe(_collisionEnterConnection,
			[this](const Collision3DEvent& event)
			{
				HandleCollisionEnter(event);
			});
	}

	void CollisionTriggerComponent::HandleCollisionEnter(const Collision3DEvent& event)
	{
		if (event.type != CollisionType::Trigger || event.otherCollider == nullptr)
			return;

		if (event.otherCollider->GetCollisionLayer() != HiFiRushCollisionLayer::Player)
			return;

		GM_ASSERT_RETURN(_triggerSystem->Activate(_triggerId), "Collision Trigger 실행에 실패했습니다. triggerId=%ls", _triggerId.c_str());
	}
}
