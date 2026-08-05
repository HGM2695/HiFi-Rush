#pragma once

#include "MovementComponent.h"
#include "WeakGameObjectPtr.h"

namespace gm
{
	class CameraComponent;
	class ChiMoveComponent;
	class ChiStateMachineComponent;
	class NavMeshControllerComponent;
	class Rigidbody3DComponent;

	class FreeFlyMoveComponent final : public MovementComponent
	{
	public:
		void			SetMovementCamera(const CameraComponent& camera);
		void			SetMoveSpeed(float moveSpeed) { _moveSpeed = moveSpeed; }
		void			SetBoostMultiplier(float boostMultiplier) { _boostMultiplier = boostMultiplier; }

	protected:
		virtual void	OnInitialize() override;
		virtual void	OnTick(float deltaTime) override;

	private:
		void			SetFreeFlyEnabled(bool enabled);
		Vector3			GetInputDirection() const;

	private:
		ChiMoveComponent*			_chiMoveComponent = nullptr;
		ChiStateMachineComponent*	_stateMachineComponent = nullptr;
		NavMeshControllerComponent*	_navMeshController = nullptr;
		Rigidbody3DComponent*		_rigidbody = nullptr;

		WeakGameObjectPtr			_movementCameraOwner{};
		const CameraComponent*		_movementCamera = nullptr;

		float			_moveSpeed = 8.f;
		float			_boostMultiplier = 3.f;
		bool			_isFreeFlyEnabled = false;

		bool			_previousChiMoveEnabled = true;
		bool			_previousStateMachineEnabled = true;
		bool			_previousNavigationMovementEnabled = true;
		bool			_previousGroundCollisionEnabled = true;
		bool			_previousUseGravity = true;
		bool			_previousKinematic = false;
	};
}
