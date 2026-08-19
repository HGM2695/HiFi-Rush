#pragma once

#include "CollisionTypes.h"
#include "Component.h"
#include "Event.h"

#include <string>

namespace gm
{
	class Collider3DComponent;
	class TransformComponent;
	struct Collision3DEvent;

	class MovementBaseComponent : public Component
	{
	public:
		MovementBaseComponent(std::wstring colliderId, CollisionMask passengerMask);

		TickGroup	GetTickGroup() const override { return TickGroup::PostPhysics; }
		Vector3		CalculateMovementDelta(const Vector3& worldPosition) const;

		const std::wstring& GetColliderId() const { return _colliderId; }
		CollisionMask		GetPassengerMask() const { return _passengerMask; }

	protected:
		void OnInitialize() override;
		void OnTick(float deltaTime) override;

	private:
		void HandleContact(const Collision3DEvent& event);
		void HandleExit(const Collision3DEvent& event);
		bool IsPassenger(const Collider3DComponent& collider) const;
		bool IsTopContact(const CollisionContact& contact) const;

	private:
		std::wstring			_colliderId{};
		CollisionMask			_passengerMask = 0;
		TransformComponent*		_transform = nullptr;
		Collider3DComponent*	_collider = nullptr;
		Matrix					_previousWorld = Matrix::Identity;
		EventConnection			_collisionEnterConnection{};
		EventConnection			_collisionStayConnection{};
		EventConnection			_collisionExitConnection{};
	};
}
