#include "SceneTransitionTriggerComponent.h"

#include "Application.h"
#include "Collider3DComponent.h"
#include "GameObject.h"
#include "HiFiRushCollisionLayers.h"
#include "SceneManager.h"

namespace gm
{
	SceneTransitionTriggerComponent::SceneTransitionTriggerComponent(const std::wstring& colliderId, const std::wstring& targetSceneName)
		: _colliderId(colliderId), _targetSceneName(targetSceneName)
	{
		GM_ASSERT(colliderId.empty() == false, "Scene Transition의 Collider ID는 비어 있을 수 없습니다.");
		GM_ASSERT(targetSceneName.empty() == false, "Scene Transition의 Target Scene 이름은 비어 있을 수 없습니다.");
	}

	void SceneTransitionTriggerComponent::OnInitialize()
	{
		for (Collider3DComponent* collider : GetOwner().GetColliders3D())
		{
			if (collider->GetColliderId() == _colliderId)
			{
				_collider = collider;
				break;
			}
		}

		GM_ASSERT_RETURN(_collider, "SceneTransitionTriggerComponent가 참조하는 Collider를 찾을 수 없습니다. colliderId=%ls", _colliderId.c_str());
		GM_ASSERT_RETURN(_collider->IsTrigger(), "SceneTransitionTriggerComponent의 Collider는 Trigger여야 합니다. colliderId=%ls", _colliderId.c_str());

		_collider->OnCollisionEnter.Subscribe(_collisionEnterConnection,
			[this](const Collision3DEvent& event)
			{
				HandleCollisionEnter(event);
			});
	}

	void SceneTransitionTriggerComponent::HandleCollisionEnter(const Collision3DEvent& event)
	{
		if (_hasTriggered || event.type != CollisionType::Trigger || event.otherCollider == nullptr)
			return;

		if (event.otherCollider->GetCollisionLayer() != HiFiRushCollisionLayer::Player)
			return;

		_hasTriggered = true;
		APPLICATION.GetSceneManager().RequestSceneChange(_targetSceneName, L"CommonLoadingScene");
	}
}
