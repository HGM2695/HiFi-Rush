#pragma once

#include "CharacterMovementComponent.h"
#include "WeakGameObjectPtr.h"

namespace gm
{
	class CameraComponent;

	class ChiMoveComponent : public CharacterMovementComponent
	{
	public:
		ChiMoveComponent();

		void			OnTick(float deltaTime) override;

		void			SetMoveEnabled(bool enabled) { _inputMovementEnabled = enabled; }
		void			SetMovementCamera(const CameraComponent& camera);

		bool			IsMoveEnabled() const { return _inputMovementEnabled && IsMovementEnabled(); }
		Vector2			GetMoveInputAxis() const;
		Vector3			GetInputMoveDirection() const;

	private:
		Vector3			GetPlanarCameraForwardDirection() const;
		Vector3			GetPlanarCameraRightDirection() const;

	private:
		WeakGameObjectPtr			_movementCameraOwner{};
		const CameraComponent*		_movementCamera = nullptr;
		bool						_inputMovementEnabled = true;
	};
}
