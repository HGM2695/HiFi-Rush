#include "ChiMoveComponent.h"
#include "Application.h"
#include "CameraComponent.h"
#include "GameObject.h"
#include "Input.h"
#include "MathUtil.h"

namespace gm
{
	ChiMoveComponent::ChiMoveComponent()
		: CharacterMovementComponent(4.f, 9.f)
	{}

	void ChiMoveComponent::OnTick(float deltaTime)
	{
		CharacterMovementComponent::OnTick(deltaTime);

		if (GetOwnerTransform() == nullptr || _inputMovementEnabled == false)
		{
			ClearMovementState();
			return;
		}

		MoveAlong(GetInputMoveDirection(), deltaTime);
	}

	void ChiMoveComponent::SetMovementCamera(const CameraComponent& camera)
	{
		_movementCameraOwner = camera.GetOwner().GetWeakPtr();
		_movementCamera = &camera;
	}

	Vector3 ChiMoveComponent::GetInputMoveDirection() const
	{
		const Vector2 inputDirection = GetMoveInputAxis();
		if (inputDirection.LengthSquared() <= 0.f)
			return Vector3{};

		const Vector3 cameraForward = GetPlanarCameraForwardDirection();
		const Vector3 cameraRight = GetPlanarCameraRightDirection();
		Vector3 direction = cameraRight * inputDirection.x + cameraForward * inputDirection.y;
		direction.Normalize();
		return direction;
	}

	Vector2 ChiMoveComponent::GetMoveInputAxis() const
	{
		return APPLICATION.GetInput().GetAxis2D(KeyCode::D, KeyCode::A, KeyCode::W, KeyCode::S);
	}

	Vector3 ChiMoveComponent::GetPlanarCameraForwardDirection() const
	{
		Vector3 direction{ 0.f, 0.f, 1.f };
		if (_movementCameraOwner.IsValid() && _movementCamera)
		{
			direction = Math::GetNormalizedXZDirection(_movementCamera->GetForwardDirection());
			if (direction.LengthSquared() <= 0.000001f)
				direction = Vector3{ 0.f, 0.f, 1.f };
		}

		return direction;
	}

	Vector3 ChiMoveComponent::GetPlanarCameraRightDirection() const
	{
		const Vector3 cameraForward = GetPlanarCameraForwardDirection();
		return Vector3{ cameraForward.z, 0.f, -cameraForward.x };
	}
}
