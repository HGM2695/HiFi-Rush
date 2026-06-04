#include "ChiMoveComponent.h"
#include "Application.h"
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
		_ownerTransform = GetOwner().GetTransform();
		GM_ASSERT_RETURN(_ownerTransform, "ChiMoveComponent는 TransformComponent가 필요합니다.");

		_animatorComponent = GetOwner().GetComponent<SkeletalAnimatorComponent>();
		GM_ASSERT_RETURN(_animatorComponent, "ChiMoveComponent는 SkeletalAnimatorComponent가 필요합니다.");
	}

	void ChiMoveComponent::OnTick(float deltaTime)
	{
		ApplyPendingRootMotion();

		if (_ownerTransform == nullptr || _moveEnabled == false)
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
		_ownerTransform->Translate(_moveDirection * _moveSpeed * deltaTime);
	}

	void ChiMoveComponent::MoveAlong(const Vector3& direction, float speed, float deltaTime, bool updateRotation)
	{
		if (_ownerTransform == nullptr)
			return;

		_moveDirection = direction;
		_moveDirection.y = 0.f;
		_isMoving = _moveDirection.LengthSquared() > 0.f;

		if (_isMoving == false)
			return;

		_moveDirection.Normalize();
		if (updateRotation)
			UpdateRotationByMoveDirection(deltaTime);

		_ownerTransform->Translate(_moveDirection * speed * deltaTime);
	}

	void ChiMoveComponent::ApplyPendingRootMotion()
	{
		if (_animatorComponent == nullptr)
			return;

		// 사용하지 않는 상태에서도 매 프레임 소비해 이전 상태의 delta가 남지 않게 합니다.
		Vector3 rootMotionDelta = _animatorComponent->ConsumeRootMotionDelta();
		if (_rootMotionEnabled == false || _ownerTransform == nullptr)
			return;

		if (_rootMotionYEnabled == false)
			rootMotionDelta.y = 0.f;

		rootMotionDelta *= _rootMotionWeight;
		const Vector3 worldDelta = Vector3::Transform(rootMotionDelta, _ownerTransform->GetRotation());
		_ownerTransform->Translate(worldDelta);
	}

	void ChiMoveComponent::FaceDirectionImmediate(const Vector3& direction)
	{
		if (_ownerTransform == nullptr || direction.LengthSquared() <= 0.f)
			return;

		_ownerTransform->SetRotation(CreateRotationByDirection(direction));
	}

	Vector3 ChiMoveComponent::GetInputMoveDirection() const
	{
		const Vector2 inputDirection = APPLICATION.GetInput().GetAxis2D(KeyCode::D, KeyCode::A, KeyCode::W, KeyCode::S);
		Vector3 direction{ inputDirection.x, 0.f, inputDirection.y };

		if (direction.LengthSquared() > 0.f)
			direction.Normalize();

		return direction;
	}

	Vector3 ChiMoveComponent::GetForwardDirection() const
	{
		if (_ownerTransform == nullptr)
			return Vector3{ 0.f, 0.f, 1.f };

		Vector3 direction = Math::GetLookVector(_ownerTransform->GetRotation());
		direction.y = 0.f;
		direction.Normalize();
		return direction;
	}

	Vector3 ChiMoveComponent::GetRightDirection() const
	{
		if (_ownerTransform == nullptr)
			return Vector3{ 1.f, 0.f, 0.f };

		Vector3 direction = Math::GetRightVector(_ownerTransform->GetRotation());
		direction.y = 0.f;
		direction.Normalize();
		return direction;
	}

	void ChiMoveComponent::UpdateRotationByMoveDirection(float deltaTime)
	{
		const Quaternion targetRotation = CreateRotationByDirection(_moveDirection);
		const float ratio = std::clamp(deltaTime * _rotationInterpSpeed, 0.f, 1.f);

		_ownerTransform->SetRotation(Quaternion::Slerp(_ownerTransform->GetRotation(), targetRotation, ratio));
	}

	Quaternion ChiMoveComponent::CreateRotationByDirection(const Vector3& direction) const
	{
		const float yaw = std::atan2(direction.x, direction.z);
		return Quaternion::CreateFromAxisAngle(Vector3{ 0.f, 1.f, 0.f }, yaw + _rotationYawOffset);
	}
}
