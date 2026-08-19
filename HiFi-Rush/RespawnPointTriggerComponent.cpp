#include "RespawnPointTriggerComponent.h"

#include "Collider3DComponent.h"
#include "GameObject.h"
#include "GameplayScene.h"
#include "HiFiRushCollisionLayers.h"

#include <cmath>

namespace gm
{
	RespawnPointTriggerComponent::RespawnPointTriggerComponent(const std::wstring& colliderId, const Vector3& respawnPosition, float respawnRotationY)
		: _colliderId(colliderId), _respawnPosition(respawnPosition), _respawnRotationY(respawnRotationY)
	{
		GM_ASSERT(colliderId.empty() == false, "Respawn Point Trigger의 Collider ID는 비어 있을 수 없습니다.");
		GM_ASSERT(std::isfinite(respawnPosition.x) && std::isfinite(respawnPosition.y) && std::isfinite(respawnPosition.z), "Respawn Point Trigger의 Respawn Position은 유한한 값이어야 합니다.");
		GM_ASSERT(std::isfinite(respawnRotationY), "Respawn Point Trigger의 Respawn Rotation Y는 유한한 값이어야 합니다.");
	}

	void RespawnPointTriggerComponent::OnInitialize()
	{
		for (Collider3DComponent* collider : GetOwner().GetColliders3D())
		{
			if (collider->GetColliderId() == _colliderId)
			{
				_collider = collider;
				break;
			}
		}

		GM_ASSERT_RETURN(_collider, "RespawnPointTriggerComponent가 참조하는 Collider를 찾을 수 없습니다. colliderId=%ls", _colliderId.c_str());
		GM_ASSERT_RETURN(_collider->IsTrigger(), "RespawnPointTriggerComponent가 참조하는 Collider는 Trigger여야 합니다. colliderId=%ls", _colliderId.c_str());

		_gameplayScene = dynamic_cast<GameplayScene*>(GetOwner().GetScene());
		GM_ASSERT_RETURN(_gameplayScene, "RespawnPointTriggerComponent는 GameplayScene에서만 사용할 수 있습니다.");

		_collider->OnCollisionEnter.Subscribe(_collisionEnterConnection,
			[this](const Collision3DEvent& event)
			{
				HandleCollisionEnter(event);
			});
	}

	void RespawnPointTriggerComponent::HandleCollisionEnter(const Collision3DEvent& event)
	{
		if (event.type != CollisionType::Trigger || event.otherCollider == nullptr)
			return;

		if (event.otherCollider->GetCollisionLayer() != HiFiRushCollisionLayer::Player)
			return;

		_gameplayScene->SetPlayerRespawnPoint(_respawnPosition, _respawnRotationY);
	}
}
