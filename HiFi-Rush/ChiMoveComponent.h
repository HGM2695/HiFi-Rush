#pragma once

#include "CharacterMovementComponent.h"
#include "WeakGameObjectPtr.h"

namespace gm
{
	class CameraComponent;

	struct ChiJumpPhysicsSettings
	{
		float jumpImpulse = 13.f;
		float doubleJumpImpulse = 12.5f;
		float riseGravityScale = 3.5f;
		float apexGravityScale = 2.f;
		float fallGravityScale = 2.f;
		float apexVelocityThreshold = 2.f;
		float downTransitionVelocity = -8.f;
	};

	class ChiMoveComponent : public CharacterMovementComponent
	{
	public:
		ChiMoveComponent();

		void			OnTick(float deltaTime) override;

		void			SetMoveEnabled(bool enabled) { _inputMovementEnabled = enabled; }
		void			SetMovementCamera(const CameraComponent& camera);
		void			SetJumpPhysicsSettings(const ChiJumpPhysicsSettings& settings) { _jumpPhysicsSettings = settings; }

		bool			IsMoveEnabled() const { return _inputMovementEnabled && IsMovementEnabled(); }
		Vector2			GetMoveInputAxis() const;
		Vector3			GetInputMoveDirection() const;
		const ChiJumpPhysicsSettings& GetJumpPhysicsSettings() const { return _jumpPhysicsSettings; }

	private:
		Vector3			GetPlanarCameraForwardDirection() const;
		Vector3			GetPlanarCameraRightDirection() const;

	private:
		WeakGameObjectPtr			_movementCameraOwner{};
		const CameraComponent*		_movementCamera = nullptr;
		ChiJumpPhysicsSettings		_jumpPhysicsSettings{};
		bool						_inputMovementEnabled = true;
	};
}
