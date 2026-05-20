#include "SocketFollowComponent.h"
#include "GameObject.h"
#include "SocketComponent.h"
#include "Transform.h"

namespace gm
{
	void SocketFollowComponent::SetTarget(const GameObject& target, const std::wstring& socketName)
	{
		_target = target.GetWeakPtr();
		_targetSocketComponent = target.GetComponent<SocketComponent>();
		_followSocketName = socketName;

		GM_ASSERT_RETURN(_targetSocketComponent, "Target GameObject에 SocketComponent가 존재하지 않습니다.");
		GM_ASSERT_RETURN(_followSocketName.empty() == false, "Socket 이름이 비어 있습니다.");
	}

	void SocketFollowComponent::ClearTarget()
	{
		_target.Reset();
		_targetSocketComponent = nullptr;
		_followSocketName.clear();
	}

	void SocketFollowComponent::OnInitialize()
	{
		_ownerTransform = GetOwner().GetTransform();
		GM_ASSERT_RETURN(_ownerTransform, "SocketFollowComponent 소유자의 Transform이 존재하지 않습니다.");
	}

	void SocketFollowComponent::OnTick(float deltaTime)
	{
		if (_target.IsValid() == false)
			return;

		_ownerTransform->SetWorldMatrix(_targetSocketComponent->GetSocketWorldMatrix(_followSocketName));
	}
}
