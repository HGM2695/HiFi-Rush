#include "CharacterMovementComponent.h"

#include "GameObject.h"
#include "MathUtil.h"
#include "MovementBaseComponent.h"
#include "NavMeshControllerComponent.h"
#include "SkeletalAnimatorComponent.h"
#include "TransformComponent.h"

#include <algorithm>

namespace gm
{
	CharacterMovementComponent::CharacterMovementComponent(float moveSpeed, float rotationInterpSpeed)
		: _moveSpeed(moveSpeed), _rotationInterpSpeed(rotationInterpSpeed)
	{
		GM_ASSERT(moveSpeed >= 0.f, "Character 이동 속도는 0 이상이어야 합니다.");
		GM_ASSERT(rotationInterpSpeed >= 0.f, "Character 회전 보간 속도는 0 이상이어야 합니다.");
	}

	void CharacterMovementComponent::SetMoveSpeed(float moveSpeed)
	{
		GM_ASSERT_RETURN(moveSpeed >= 0.f, "Character 이동 속도는 0 이상이어야 합니다.");
		_moveSpeed = moveSpeed;
	}

	void CharacterMovementComponent::SetRotationInterpSpeed(float speed)
	{
		GM_ASSERT_RETURN(speed >= 0.f, "Character 회전 보간 속도는 0 이상이어야 합니다.");
		_rotationInterpSpeed = speed;
	}

	void CharacterMovementComponent::MoveAlong(const Vector3& direction, float deltaTime, bool updateRotation)
	{
		MoveAlong(direction, _moveSpeed, deltaTime, updateRotation);
	}

	void CharacterMovementComponent::MoveAlong(const Vector3& direction, float speed, float deltaTime, bool updateRotation)
	{
		ClearMovementState();
		if (IsMovementEnabled() == false || deltaTime <= 0.f)
			return;

		const Vector3 planarDirection = Math::GetNormalizedXZDirection(direction);
		if (planarDirection.LengthSquared() <= 0.000001f)
			return;

		_moveDirection = planarDirection;
		_isMoving = true;

		if (updateRotation)
			FaceDirection(planarDirection, deltaTime);

		Move(planarDirection * speed * deltaTime);
	}

	void CharacterMovementComponent::FaceDirection(const Vector3& direction, float deltaTime)
	{
		TransformComponent* transform = GetOwnerTransform();
		if (transform == nullptr || deltaTime <= 0.f)
			return;

		const Vector3 planarDirection = Math::GetNormalizedXZDirection(direction);
		if (planarDirection.LengthSquared() <= 0.000001f)
			return;

		const Quaternion targetRotation = Math::CreateRotationByDirection(planarDirection, _rotationYawOffset);
		const float ratio = std::clamp(_rotationInterpSpeed * deltaTime, 0.f, 1.f);
		transform->SetRotation(Quaternion::Slerp(transform->GetRotation(), targetRotation, ratio));
	}

	void CharacterMovementComponent::FaceDirectionImmediate(const Vector3& direction)
	{
		TransformComponent* transform = GetOwnerTransform();
		if (transform == nullptr)
			return;

		const Vector3 planarDirection = Math::GetNormalizedXZDirection(direction);
		if (planarDirection.LengthSquared() <= 0.000001f)
			return;

		transform->SetRotation(Math::CreateRotationByDirection(planarDirection, _rotationYawOffset));
	}

	Vector3 CharacterMovementComponent::GetForwardDirection() const
	{
		const TransformComponent* transform = GetOwnerTransform();
		if (transform == nullptr)
			return Vector3{ 0.f, 0.f, 1.f };

		const Vector3 direction = Math::GetNormalizedXZDirection(Math::GetLookVector(transform->GetRotation()));
		if (direction.LengthSquared() <= 0.000001f)
			return Vector3{ 0.f, 0.f, 1.f };

		return direction;
	}

	Vector3 CharacterMovementComponent::GetRightDirection() const
	{
		const TransformComponent* transform = GetOwnerTransform();
		if (transform == nullptr)
			return Vector3{ 1.f, 0.f, 0.f };

		const Vector3 direction = Math::GetNormalizedXZDirection(Math::GetRightVector(transform->GetRotation()));
		if (direction.LengthSquared() <= 0.000001f)
			return Vector3{ 1.f, 0.f, 0.f };

		return direction;
	}

	void CharacterMovementComponent::SetMovementBase(MovementBaseComponent& movementBase)
	{
		if (_movementBase == &movementBase)
			return;

		_movementBaseOwner = movementBase.GetOwner().GetWeakPtr();
		_movementBase = &movementBase;

		const NavMeshControllerComponent* navMeshController = GetOwner().GetComponent<NavMeshControllerComponent>();
		if (navMeshController == nullptr || navMeshController->IsGrounded() == false)
			OnMovementBaseContact.Publish(MovementBaseContactEvent{});
	}

	void CharacterMovementComponent::ClearMovementBase(const MovementBaseComponent& movementBase)
	{
		if (_movementBase == &movementBase)
			ResetMovementBase();
	}

	void CharacterMovementComponent::OnInitialize()
	{
		MovementComponent::OnInitialize();
		_animatorComponent = GetOwner().GetComponent<SkeletalAnimatorComponent>();
		GM_ASSERT_RETURN(_animatorComponent, "CharacterMovementComponent는 SkeletalAnimatorComponent가 필요합니다.");
	}

	void CharacterMovementComponent::OnTick(float)
	{
		ApplyMovementBase();

		if (_animatorComponent == nullptr)
			return;

		Vector3 rootMotionDelta = _animatorComponent->ConsumeRootMotionDelta();
		if (_rootMotionEnabled == false || IsMovementEnabled() == false || GetOwnerTransform() == nullptr)
			return;

		rootMotionDelta = Vector3{rootMotionDelta.x * _rootMotionWeight.x, rootMotionDelta.y * _rootMotionWeight.y, rootMotionDelta.z * _rootMotionWeight.z};
		rootMotionDelta = Vector3::Transform(rootMotionDelta, GetOwnerTransform()->GetRotation());
		Move(rootMotionDelta);
	}

	void CharacterMovementComponent::ApplyMovementBase()
	{
		if (_movementBase == nullptr)
			return;

		if (_movementBaseOwner.IsValid() == false || _movementBase->IsEnabled() == false || GetOwnerTransform() == nullptr)
		{
			ResetMovementBase();
			return;
		}

		Translate(_movementBase->CalculateMovementDelta(GetOwnerTransform()->GetPosition()));
	}

	void CharacterMovementComponent::ResetMovementBase()
	{
		_movementBaseOwner.Reset();
		_movementBase = nullptr;
	}

	void CharacterMovementComponent::ClearMovementState()
	{
		_isMoving = false;
		_moveDirection = Vector3{};
	}
}
