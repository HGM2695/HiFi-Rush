#pragma once

#include "Component.h"
#include "Event.h"

namespace gm
{
	class Rigidbody3DComponent;
	class TransformComponent;

	struct NavigationGroundContactEvent final : EventType
	{
		Vector3	position{};
		int32	cellIndex = -1;
	};

	struct NavigationGroundLostEvent final : EventType
	{
		Vector3	position{};
		float	groundHeight = 0.f;
		int32	cellIndex = -1;
	};

	class NavMeshControllerComponent : public Component
	{
	public:
		virtual TickGroup	GetTickGroup() const override { return TickGroup::PostPhysics; }

		void				SetMovementEnabled(bool enabled) { _movementEnabled = enabled; }
		bool				IsMovementEnabled() const { return IsEnabled() && _movementEnabled; }
		void				SetGroundCollisionEnabled(bool enabled) { _groundCollisionEnabled = enabled; _isGroundStateInitialized = false; _isGrounded = false; }
		bool				IsGroundCollisionEnabled() const { return _groundCollisionEnabled; }
		bool				IsGrounded() const { return _isGrounded; }
		void				SetMaxGroundSnapDownDistance(float distance);
		float				GetMaxGroundSnapDownDistance() const { return _maxGroundSnapDownDistance; }

		int32				GetCurrentCellIndex() const { return _currentCellIndex; }
		void				SetCurrentCellIndex(int32 cellIndex) { _currentCellIndex = cellIndex; }
		bool				RefreshCellIndex();
		bool				Move(const Vector3& desiredDelta);

		EventPublisher<NavMeshControllerComponent, NavigationGroundContactEvent>	OnGroundContact;
		EventPublisher<NavMeshControllerComponent, NavigationGroundLostEvent>		OnGroundLost;

	protected:
		virtual void		OnInitialize() override;
		virtual void		OnTick(float deltaTime) override;

	private:
		void				CheckGroundCollision();

	private:
		TransformComponent*		_transform = nullptr;
		Rigidbody3DComponent*	_rigidbody = nullptr;
		int32					_currentCellIndex = -1;
		float					_maxGroundSnapDownDistance = 0.1f;
		bool					_movementEnabled = true;
		bool					_groundCollisionEnabled = false;
		bool					_isGroundStateInitialized = false;
		bool					_isGrounded = false;
	};
}
