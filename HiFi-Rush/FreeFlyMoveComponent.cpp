#include "FreeFlyMoveComponent.h"

#include "Application.h"
#include "CameraComponent.h"
#include "ChiMoveComponent.h"
#include "ChiStateMachineComponent.h"
#include "DebugInputHandler.h"
#include "GameObject.h"
#include "Input.h"
#include "MathUtil.h"
#include "NavMeshControllerComponent.h"
#include "Rigidbody3DComponent.h"
#include "TransformComponent.h"

namespace gm
{
	namespace
	{
		constexpr wchar_t FreeFlyDebugType[] = L"PlayerFreeFly";
	}

	void FreeFlyMoveComponent::SetMovementCamera(const CameraComponent& camera)
	{
		_movementCameraOwner = camera.GetOwner().GetWeakPtr();
		_movementCamera = &camera;
	}

	void FreeFlyMoveComponent::OnInitialize()
	{
		MovementComponent::OnInitialize();

		_chiMoveComponent = GetOwner().GetComponent<ChiMoveComponent>();
		_stateMachineComponent = GetOwner().GetComponent<ChiStateMachineComponent>();
		_navMeshController = GetOwner().GetComponent<NavMeshControllerComponent>();
		_rigidbody = GetOwner().GetComponent<Rigidbody3DComponent>();

		GM_ASSERT_RETURN(_chiMoveComponent, "FreeFlyMoveComponent는 ChiMoveComponent가 필요합니다.");
		GM_ASSERT_RETURN(_stateMachineComponent, "FreeFlyMoveComponent는 ChiStateMachineComponent가 필요합니다.");
		GM_ASSERT_RETURN(_navMeshController, "FreeFlyMoveComponent는 NavMeshControllerComponent가 필요합니다.");
		GM_ASSERT_RETURN(_rigidbody, "FreeFlyMoveComponent는 Rigidbody3DComponent가 필요합니다.");
		GM_ASSERT_RETURN(_movementCameraOwner.IsValid() && _movementCamera, "FreeFlyMoveComponent는 이동 기준 Camera가 필요합니다.");

#if GM_ENABLE_DEBUG_TOOLS
		DebugInputHandler::RegisterDebugType(FreeFlyDebugType, true);
#endif
	}

	void FreeFlyMoveComponent::OnTick(float deltaTime)
	{
#if GM_ENABLE_DEBUG_TOOLS
		if (DebugInputHandler::IsTriggered(FreeFlyDebugType, KeyCode::F10))
			SetFreeFlyEnabled(_isFreeFlyEnabled == false);

		if (_isFreeFlyEnabled == false)
			return;

		const Vector3 direction = GetInputDirection();
		if (direction.LengthSquared() <= 0.f)
			return;

		const Input& input = APPLICATION.GetInput();
		const float speedMultiplier = input.IsKeyRepeat(KeyCode::LeftShift) ? _boostMultiplier : 1.f;
		Translate(direction * _moveSpeed * speedMultiplier * deltaTime);
#else
		(void)deltaTime;
#endif
	}

	void FreeFlyMoveComponent::SetFreeFlyEnabled(bool enabled)
	{
		if (_isFreeFlyEnabled == enabled)
			return;

		_isFreeFlyEnabled = enabled;
		if (enabled)
		{
			_previousChiMoveEnabled = _chiMoveComponent->IsEnabled();
			_previousStateMachineEnabled = _stateMachineComponent->IsEnabled();
			_previousNavigationMovementEnabled = _navMeshController->IsEnabled();
			_previousUseGroundCollision = _navMeshController->IsUseGroundCollision();
			_previousUseGravity = _rigidbody->IsUseGravity();
			_previousKinematic = _rigidbody->IsKinematic();

			_chiMoveComponent->SetEnabled(false);
			_stateMachineComponent->SetEnabled(false);
			_navMeshController->SetEnabled(false);
			_navMeshController->SetUseGroundCollision(false);
			_rigidbody->SetUseGravity(false);
			_rigidbody->SetKinematic(true);
			_rigidbody->SetVelocity(Vector3{});
			_rigidbody->ClearForces();
			GM_LOG("자유 이동 모드를 활성화했습니다. 이동=WASD, 상승/하강=E/Q, 가속=Shift, 종료=F10");
			return;
		}

		_rigidbody->SetVelocity(Vector3{});
		_rigidbody->ClearForces();
		_rigidbody->SetUseGravity(_previousUseGravity);
		_rigidbody->SetKinematic(_previousKinematic);
		_navMeshController->SetUseGroundCollision(_previousUseGroundCollision);
		_navMeshController->SetEnabled(_previousNavigationMovementEnabled);
		_stateMachineComponent->SetEnabled(_previousStateMachineEnabled);
		_chiMoveComponent->SetEnabled(_previousChiMoveEnabled);

		if (_previousNavigationMovementEnabled && _navMeshController->RefreshCellIndex() == false)
			GM_LOG("자유 이동 종료 위치에서 Navigation Cell을 찾지 못했습니다.");

		GM_LOG("자유 이동 모드를 비활성화했습니다.");
	}

	Vector3 FreeFlyMoveComponent::GetInputDirection() const
	{
		if (_movementCameraOwner.IsValid() == false || _movementCamera == nullptr)
			return Vector3{};

		const TransformComponent* cameraTransform = _movementCamera->GetOwner().GetTransform();
		if (cameraTransform == nullptr)
			return Vector3{};

		const Input& input = APPLICATION.GetInput();
		const Vector2 horizontalInput = input.GetAxis2D(KeyCode::D, KeyCode::A, KeyCode::W, KeyCode::S);
		const float verticalInput = input.GetAxis(KeyCode::E, KeyCode::Q);
		const Quaternion cameraRotation = cameraTransform->GetRotation();

		Vector3 direction = Math::GetRightVector(cameraRotation) * horizontalInput.x;
		direction += Math::GetLookVector(cameraRotation) * horizontalInput.y;
		direction += Math::GetUpVector(cameraRotation) * verticalInput;
		if (direction.LengthSquared() > 0.f)
			direction.Normalize();

		return direction;
	}
}
