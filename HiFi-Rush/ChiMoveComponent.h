#pragma once

#include "MovementComponent.h"
#include "WeakGameObjectPtr.h"

namespace gm
{
	class CameraComponent;
	class SkeletalAnimatorComponent;

	class ChiMoveComponent : public MovementComponent
	{
	public:
		virtual void	OnInitialize() override;
		virtual void	OnTick(float deltaTime) override;

		void			SetMoveSpeed(float moveSpeed) { _moveSpeed = moveSpeed; }
		void			SetMoveEnabled(bool enabled) { _moveEnabled = enabled; }
		void			SetRootMotionEnabled(bool enabled) { _rootMotionEnabled = enabled; }
		void			SetRootMotionWeight(float weight) { _rootMotionWeight = weight; }
		void			SetRootMotionYEnabled(bool enabled) { _rootMotionYEnabled = enabled; }
		void			SetRotationYawOffset(float radians) { _rotationYawOffset = radians; }
		void			SetRotationInterpSpeed(float speed) { _rotationInterpSpeed = speed; }
		void			SetMovementCamera(const CameraComponent& camera);
		void			MoveAlong(const Vector3& direction, float speed, float deltaTime, bool updateRotation);
		void			FaceDirectionImmediate(const Vector3& direction);

		bool			IsMoving() const { return _isMoving; }
		bool			IsMoveEnabled() const { return _moveEnabled; }
		bool			IsRootMotionEnabled() const { return _rootMotionEnabled; }
		const Vector3&	GetMoveDirection() const { return _moveDirection; }
		Vector2			GetMoveInputAxis() const;
		Vector3			GetInputMoveDirection() const;
		Vector3			GetForwardDirection() const;
		Vector3			GetRightDirection() const;

	private:
		void			ApplyPendingRootMotion();
		void			UpdateRotationByMoveDirection(float deltaTime);
		Vector3			GetCameraForwardDirection() const;
		Vector3			GetCameraRightDirection() const;
		Quaternion		CreateRotationByDirection(const Vector3& direction) const;

	private:
		SkeletalAnimatorComponent*	_animatorComponent = nullptr;
		WeakGameObjectPtr			_movementCameraOwner{};
		const CameraComponent*		_movementCamera = nullptr;

		bool				_isMoving = false;
		bool				_moveEnabled = true;
		float				_moveSpeed = 4.f;
		Vector3				_moveDirection{};

		float				_rotationYawOffset = 0.f;
		float				_rotationInterpSpeed = 9.f;

		float				_rootMotionWeight = 1.f;
		bool				_rootMotionEnabled = false;
		bool				_rootMotionYEnabled = true;
	};
}
