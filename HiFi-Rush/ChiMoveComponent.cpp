#include "ChiMoveComponent.h"
#include "Application.h"
#include "CameraComponent.h"
#include "GameObject.h"
#include "Input.h"
#include "MathUtil.h"
#include "SkeletalAnimatorComponent.h"
#include "TransformComponent.h"

#include <algorithm>
#include <cmath>

namespace gm
{
	void ChiMoveComponent::OnInitialize()
	{
		MovementComponent::OnInitialize();
		EnableNavigationMovement(true);

		_animatorComponent = GetOwner().GetComponent<SkeletalAnimatorComponent>();
		GM_ASSERT_RETURN(_animatorComponent, "ChiMoveComponent는 SkeletalAnimatorComponent가 필요합니다.");
	}

	void ChiMoveComponent::OnTick(float deltaTime)
	{
		ApplyPendingRootMotion();

		if (GetOwnerTransform() == nullptr || _moveEnabled == false)
		{
			_isMoving = false;
			_moveDirection = Vector3{};
			return;
		}

		_moveDirection = GetInputMoveDirection();
		_isMoving = _moveDirection.LengthSquared() > 0.f;

		if (_isMoving == false)
			return;

		UpdateRotationByMoveDirection(deltaTime);
		const Vector3 desiredDelta = _moveDirection * _moveSpeed * deltaTime;
		Move(desiredDelta);
	}

	void ChiMoveComponent::SetMovementCamera(const CameraComponent& camera)
	{
		_movementCameraOwner = camera.GetOwner().GetWeakPtr();
		_movementCamera = &camera;
	}

	void ChiMoveComponent::MoveAlong(const Vector3& direction, float speed, float deltaTime, bool updateRotation)
	{
		if (GetOwnerTransform() == nullptr)
			return;

		_moveDirection = direction;
		_moveDirection.y = 0.f;
		_isMoving = _moveDirection.LengthSquared() > 0.f;

		if (_isMoving == false)
			return;

		_moveDirection.Normalize();
		if (updateRotation)
			UpdateRotationByMoveDirection(deltaTime);

		const Vector3 desiredDelta = _moveDirection * speed * deltaTime;
		Move(desiredDelta);
	}

	void ChiMoveComponent::ApplyPendingRootMotion()
	{
		if (_animatorComponent == nullptr)
			return;

		// 사용하지 않는 상태에서도 매 프레임 소비해 이전 상태의 delta가 남지 않게 합니다.
		Vector3 rootMotionDelta = _animatorComponent->ConsumeRootMotionDelta();
		if (_rootMotionEnabled == false || GetOwnerTransform() == nullptr)
			return;

		if (_rootMotionYEnabled == false)
			rootMotionDelta.y = 0.f;

		rootMotionDelta *= _rootMotionWeight;
		const Vector3 worldDelta = Vector3::Transform(rootMotionDelta, GetOwnerTransform()->GetRotation());
		Move(worldDelta);
	}

	void ChiMoveComponent::FaceDirectionImmediate(const Vector3& direction)
	{
		if (GetOwnerTransform() == nullptr || direction.LengthSquared() <= 0.f)
			return;

		GetOwnerTransform()->SetRotation(CreateRotationByDirection(direction));
	}

	Vector3 ChiMoveComponent::GetInputMoveDirection() const
	{
		const Vector2 inputDirection = GetMoveInputAxis();
		if (inputDirection.LengthSquared() <= 0.f)
			return Vector3{};

		const Vector3 cameraForward = GetCameraForwardDirection();
		const Vector3 cameraRight = GetCameraRightDirection();
		Vector3 direction = cameraRight * inputDirection.x + cameraForward * inputDirection.y;
		direction.Normalize();
		return direction;
	}

	Vector2 ChiMoveComponent::GetMoveInputAxis() const
	{
		return APPLICATION.GetInput().GetAxis2D(KeyCode::D, KeyCode::A, KeyCode::W, KeyCode::S);
	}

	Vector3 ChiMoveComponent::GetForwardDirection() const
	{
		if (GetOwnerTransform() == nullptr)
			return Vector3{ 0.f, 0.f, 1.f };

		Vector3 direction = Math::GetLookVector(GetOwnerTransform()->GetRotation());
		direction.y = 0.f;
		direction.Normalize();
		return direction;
	}

	Vector3 ChiMoveComponent::GetRightDirection() const
	{
		if (GetOwnerTransform() == nullptr)
			return Vector3{ 1.f, 0.f, 0.f };

		Vector3 direction = Math::GetRightVector(GetOwnerTransform()->GetRotation());
		direction.y = 0.f;
		direction.Normalize();
		return direction;
	}

	Vector3 ChiMoveComponent::GetCameraForwardDirection() const
	{
		Vector3 direction{ 0.f, 0.f, 1.f };
		if (_movementCameraOwner.IsValid() && _movementCamera)
		{
			const TransformComponent* cameraTransform = _movementCamera->GetOwner().GetTransform();
			GM_ASSERT_RETURN_VAL(cameraTransform, Vector3{}, "이동 기준 Camera의 TransformComponent가 존재하지 않습니다.");

			direction = Math::GetLookVector(cameraTransform->GetRotation());
			direction.y = 0.f;

			if (direction.LengthSquared() <= 0.000001f)
				direction = Vector3{ 0.f, 0.f, 1.f };
			else
				direction.Normalize();
		}

		return direction;
	}

	Vector3 ChiMoveComponent::GetCameraRightDirection() const
	{
		const Vector3 cameraForward = GetCameraForwardDirection();
		return Vector3{ cameraForward.z, 0.f, -cameraForward.x };
	}

	void ChiMoveComponent::UpdateRotationByMoveDirection(float deltaTime)
	{
		const Quaternion targetRotation = CreateRotationByDirection(_moveDirection);
		const float ratio = std::clamp(deltaTime * _rotationInterpSpeed, 0.f, 1.f);

		TransformComponent* ownerTransform = GetOwnerTransform();
		GM_ASSERT_RETURN(ownerTransform, "ChiMoveComponent는 TransformComponent가 필요합니다.");
		ownerTransform->SetRotation(Quaternion::Slerp(ownerTransform->GetRotation(), targetRotation, ratio));
	}

	Quaternion ChiMoveComponent::CreateRotationByDirection(const Vector3& direction) const
	{
		const float yaw = std::atan2(direction.x, direction.z);
		return Quaternion::CreateFromAxisAngle(Vector3{ 0.f, 1.f, 0.f }, yaw + _rotationYawOffset);
	}
}
