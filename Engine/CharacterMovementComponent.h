#pragma once

#include "MovementComponent.h"

namespace gm
{
	class SkeletalAnimatorComponent;

	class CharacterMovementComponent : public MovementComponent
	{
	public:
		CharacterMovementComponent(float moveSpeed = 0.f, float rotationInterpSpeed = 0.f);

		void			SetMoveSpeed(float moveSpeed);
		float			GetMoveSpeed() const { return _moveSpeed; }
		void			SetRotationInterpSpeed(float speed);
		float			GetRotationInterpSpeed() const { return _rotationInterpSpeed; }
		void			SetRotationYawOffset(float radians) { _rotationYawOffset = radians; }
		float			GetRotationYawOffset() const { return _rotationYawOffset; }

		void			SetMovementEnabled(bool enabled) { _movementEnabled = enabled; }
		bool			IsMovementEnabled() const { return IsEnabled() && _movementEnabled; }
		void			SetRootMotionEnabled(bool enabled) { _rootMotionEnabled = enabled; }
		bool			IsRootMotionEnabled() const { return _rootMotionEnabled; }
		void			SetRootMotionWeight(const Vector3& weight) { _rootMotionWeight = weight; }
		const Vector3&	GetRootMotionWeight() const { return _rootMotionWeight; }

		void			MoveAlong(const Vector3& direction, float deltaTime, bool updateRotation = true);
		void			MoveAlong(const Vector3& direction, float speed, float deltaTime, bool updateRotation);
		void			FaceDirection(const Vector3& direction, float deltaTime);
		void			FaceDirectionImmediate(const Vector3& direction);
		bool			IsMoving() const { return _isMoving; }
		const Vector3&	GetMoveDirection() const { return _moveDirection; }
		Vector3			GetForwardDirection() const;
		Vector3			GetRightDirection() const;

	protected:
		void			OnInitialize() override;
		void			OnTick(float deltaTime) override;
		void			ClearMovementState();

	private:
		SkeletalAnimatorComponent*	_animatorComponent = nullptr;
		Vector3						_moveDirection{};
		float						_moveSpeed = 0.f;
		float						_rotationInterpSpeed = 0.f;
		float						_rotationYawOffset = 0.f;
		Vector3						_rootMotionWeight{ 1.f, 1.f, 1.f };
		bool						_isMoving = false;
		bool						_movementEnabled = true;
		bool						_rootMotionEnabled = true;
	};
}
