#include "SocketColliderComponent.h"

#include "Collider3DComponent.h"
#include "GameObject.h"
#include "SocketComponent.h"
#include "TransformComponent.h"

#include <utility>

namespace gm
{
	SocketColliderComponent::SocketColliderComponent(Collider3DComponent& collider, std::wstring socketName)
		: _collider(collider), _socketName(std::move(socketName))
	{
		GM_ASSERT(_socketName.empty() == false, "Socket Collider가 참조할 Socket 이름은 비어 있을 수 없습니다.");
	}

	void SocketColliderComponent::OnInitialize()
	{
		GM_ASSERT_RETURN(&_collider.GetOwner() == &GetOwner(), "SocketColliderComponent와 Collider는 같은 GameObject에 속해야 합니다.");

		_socketComponent = GetOwner().GetComponent<SocketComponent>();
		_ownerTransform = GetOwner().GetTransform();
		GM_ASSERT_RETURN(_socketComponent, "SocketColliderComponent에 SocketComponent가 없습니다.");
		GM_ASSERT_RETURN(_socketComponent->HasSocket(_socketName), "SocketColliderComponent가 참조하는 Socket을 찾을 수 없습니다. socket=%ls", _socketName.c_str());
		GM_ASSERT_RETURN(_ownerTransform, "SocketColliderComponent에 TransformComponent가 없습니다.");
		_baseLocalCenter = _collider.GetLocalCenter();
		_baseLocalRotation = _collider.GetLocalRotation();

		UpdateColliderTransform();
	}

	void SocketColliderComponent::OnTick(float)
	{
		UpdateColliderTransform();
	}

	void SocketColliderComponent::UpdateColliderTransform()
	{
		if (_socketComponent == nullptr || _ownerTransform == nullptr)
			return;

		const Matrix socketWorld = _followSocketRotation ? _socketComponent->GetSocketWorldMatrix(_socketName) : _socketComponent->GetSocketAnchorWorldMatrix(_socketName);
		Matrix socketLocal = socketWorld * _ownerTransform->GetWorldMatrix().Invert();

		Vector3 scale{};
		Quaternion rotation{};
		Vector3 position{};
		if (socketLocal.Decompose(scale, rotation, position) == false)
			return;

		if (_followSocketRotation)
		{
			_collider.SetLocalCenter(position);
			_collider.SetLocalRotation(rotation);
		}
		else
		{
			const Socket* socket = _socketComponent->FindSocket(_socketName);
			if (socket == nullptr)
				return;

			_collider.SetLocalCenter(position + socket->position + _baseLocalCenter);
			_collider.SetLocalRotation(_baseLocalRotation);
		}
	}
}
