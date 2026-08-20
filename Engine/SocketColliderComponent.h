#pragma once

#include "Component.h"

#include <string>

namespace gm
{
	class Collider3DComponent;
	class SocketComponent;
	class TransformComponent;

	class SocketColliderComponent final : public Component
	{
	public:
		SocketColliderComponent(Collider3DComponent& collider, std::wstring socketName);

		void SetFollowSocketRotation(bool followSocketRotation) { _followSocketRotation = followSocketRotation; }

		TickGroup GetTickGroup() const override { return TickGroup::Movement; }

	protected:
		void OnInitialize() override;
		void OnTick(float deltaTime) override;

	private:
		void UpdateColliderTransform();

	private:
		Collider3DComponent&	_collider;
		std::wstring			_socketName{};
		SocketComponent*		_socketComponent = nullptr;
		TransformComponent*		_ownerTransform = nullptr;
		Vector3					_baseLocalCenter{};
		Quaternion				_baseLocalRotation{};
		bool					_followSocketRotation = true;
	};
}
