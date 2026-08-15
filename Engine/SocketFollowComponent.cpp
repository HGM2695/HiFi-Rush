#include "SocketFollowComponent.h"
#include "GameObject.h"
#include "SocketComponent.h"
#include "TransformComponent.h"

namespace gm
{
	void SocketFollowComponent::SetTarget(const GameObject& target, const std::wstring& socketName)
	{
		const SocketComponent* socketComponent = target.GetComponent<SocketComponent>();

		GM_ASSERT_RETURN(socketComponent, "Target GameObject에 SocketComponent가 존재하지 않습니다.");
		GM_ASSERT_RETURN(socketName.empty() == false, "Socket 이름이 비어 있습니다.");
		GM_ASSERT_RETURN(socketComponent->HasSocket(socketName), "Target GameObject에 요청한 Socket이 존재하지 않습니다.");

		_target = target.GetWeakPtr();
		_targetSocketComponent = socketComponent;
		_followSocketName = socketName;
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
		{
			if (_destroyWithTarget)
				GetOwner().Destroy();
			return;
		}

		if (_ownerTransform == nullptr || _targetSocketComponent == nullptr)
			return;

		_ownerTransform->SetWorldMatrix(_targetSocketComponent->GetSocketWorldMatrix(_followSocketName));
	}
}
