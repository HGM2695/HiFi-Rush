#pragma once

#include "Component.h"

namespace gm
{
	class Rigidbody3DComponent;
	class TransformComponent;

	class NavMeshControllerComponent : public Component
	{
	public:
		virtual TickGroup	GetTickGroup() const override { return TickGroup::PostPhysics; }

		void				SetMovementEnabled(bool enabled) { _movementEnabled = enabled; }
		bool				IsMovementEnabled() const { return IsEnabled() && _movementEnabled; }
		void				SetGroundCollisionEnabled(bool enabled) { _groundCollisionEnabled = enabled; _isGrounded = false; }
		bool				IsGroundCollisionEnabled() const { return _groundCollisionEnabled; }
		bool				IsGrounded() const { return _isGrounded; }

		int32				GetCurrentCellIndex() const { return _currentCellIndex; }
		void				SetCurrentCellIndex(int32 cellIndex) { _currentCellIndex = cellIndex; }
		bool				RefreshCellIndex();
		bool				Move(const Vector3& desiredDelta);

	protected:
		virtual void		OnInitialize() override;
		virtual void		OnTick(float deltaTime) override;

	private:
		void				CheckGroundCollision();

	private:
		TransformComponent*		_transform = nullptr;
		Rigidbody3DComponent*	_rigidbody = nullptr;
		int32					_currentCellIndex = -1;
		bool					_movementEnabled = true;
		bool					_groundCollisionEnabled = false;
		bool					_isGrounded = false;
	};
}
