#pragma once

#include "Component.h"

namespace gm
{
	class SkeletalAnimatorComponent;
	class TransformComponent;

	class ChiMoveComponent : public Component
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
		void			MoveAlong(const Vector3& direction, float speed, float deltaTime, bool updateRotation);
		void			FaceDirectionImmediate(const Vector3& direction);

		bool			IsMoving() const { return _isMoving; }
		bool			IsMoveEnabled() const { return _moveEnabled; }
		bool			IsRootMotionEnabled() const { return _rootMotionEnabled; }
		const Vector3&	GetMoveDirection() const { return _moveDirection; }
		Vector3			GetInputMoveDirection() const;
		Vector3			GetForwardDirection() const;
		Vector3			GetRightDirection() const;

	private:
		void			ApplyPendingRootMotion();
		void			UpdateRotationByMoveDirection(float deltaTime);
		Quaternion		CreateRotationByDirection(const Vector3& direction) const;

	private:
		TransformComponent*			_ownerTransform = nullptr;
		SkeletalAnimatorComponent*	_animatorComponent = nullptr;

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
