#include "FallRespawnTriggerComponent.h"

#include "Collider3DComponent.h"
#include "GameObject.h"
#include "GameplayScene.h"
#include "HiFiRushCollisionLayers.h"

namespace gm
{
	FallRespawnTriggerComponent::FallRespawnTriggerComponent(const std::wstring& colliderId, int32 damage)
		: _colliderId(colliderId), _damage(damage)
	{
		GM_ASSERT(colliderId.empty() == false, "Fall Respawn Trigger의 Collider ID는 비어 있을 수 없습니다.");
		GM_ASSERT(damage > 0, "Fall Respawn Trigger의 Damage는 0보다 커야 합니다.");
	}

	void FallRespawnTriggerComponent::OnInitialize()
	{
		for (Collider3DComponent* collider : GetOwner().GetColliders3D())
		{
			if (collider->GetColliderId() == _colliderId)
			{
				_collider = collider;
				break;
			}
		}

		GM_ASSERT_RETURN(_collider, "FallRespawnTriggerComponent가 참조하는 Collider를 찾을 수 없습니다. colliderId=%ls", _colliderId.c_str());
		GM_ASSERT_RETURN(_collider->IsTrigger(), "FallRespawnTriggerComponent가 참조하는 Collider는 Trigger여야 합니다. colliderId=%ls", _colliderId.c_str());

		_gameplayScene = dynamic_cast<GameplayScene*>(GetOwner().GetScene());
		GM_ASSERT_RETURN(_gameplayScene, "FallRespawnTriggerComponent는 GameplayScene에서만 사용할 수 있습니다.");

		_collider->OnCollisionEnter.Subscribe(_collisionEnterConnection,
			[this](const Collision3DEvent& event)
			{
				HandleCollisionEnter(event);
			});
	}

	void FallRespawnTriggerComponent::HandleCollisionEnter(const Collision3DEvent& event)
	{
		if (event.type != CollisionType::Trigger || event.otherCollider == nullptr)
			return;

		if (event.otherCollider->GetCollisionLayer() != HiFiRushCollisionLayer::Player)
			return;

		_gameplayScene->HandlePlayerFall(_damage);
	}
}
