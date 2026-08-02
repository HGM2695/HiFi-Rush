#include "CameraFollowComponent.h"
#include "Application.h"
#include "GameObject.h"
#include "Input.h"
#include "SocketComponent.h"
#include "TransformComponent.h"

#include <algorithm>
#include <cmath>

namespace gm
{
	void CameraFollowComponent::SetPitch(float pitchRadians)
	{
		_pitch = std::clamp(pitchRadians, _minPitch, _maxPitch);
	}

	void CameraFollowComponent::SetPitchRange(float minPitchRadians, float maxPitchRadians)
	{
		GM_ASSERT_RETURN(minPitchRadians <= maxPitchRadians, "Camera Pitch의 최소 범위가 최대 범위보다 큽니다.");

		_minPitch = minPitchRadians;
		_maxPitch = maxPitchRadians;
		_pitch = std::clamp(_pitch, _minPitch, _maxPitch);
	}

	void CameraFollowComponent::SetTarget(const GameObject& target, const std::wstring& socketName)
	{
		const SocketComponent* socketComponent = target.GetComponent<SocketComponent>();

		GM_ASSERT_RETURN(socketComponent, "Target GameObject에 SocketComponent가 존재하지 않습니다.");
		GM_ASSERT_RETURN(socketName.empty() == false, "CameraFollowComponent Socket 이름이 비어 있습니다.");
		GM_ASSERT_RETURN(socketComponent->HasSocket(socketName), "Target GameObject에 요청한 Camera Socket이 존재하지 않습니다.");

		_target = target.GetWeakPtr();
		_targetSocketComponent = socketComponent;
		_socketName = socketName;
	}

	void CameraFollowComponent::ClearTarget()
	{
		_target.Reset();
		_targetSocketComponent = nullptr;
		_socketName.clear();
	}

	void CameraFollowComponent::OnInitialize()
	{
		_ownerTransform = GetOwner().GetTransform();
		GM_ASSERT_RETURN(_ownerTransform, "CameraFollowComponent 소유자의 TransformComponent가 존재하지 않습니다.");
	}

	void CameraFollowComponent::OnTick(float deltaTime)
	{
		if (_ownerTransform == nullptr || _target.IsValid() == false || _targetSocketComponent == nullptr)
			return;

		UpdateOrbitInput();

		const Vector3 targetPosition = GetTargetPosition();
		const Vector3 cameraPosition = CalcCameraPosition(targetPosition);

		Matrix viewMatrix = Math::CreateLookAtLH(cameraPosition, targetPosition, Vector3{ 0.f, 1.f, 0.f });
		Matrix cameraWorldMatrix = viewMatrix.Invert();
		
		_ownerTransform->SetWorldMatrix(cameraWorldMatrix);
	}

	void CameraFollowComponent::UpdateOrbitInput()
	{
		if (_mouseControlEnabled == false)
			return;

		const Vector2 mouseDelta = APPLICATION.GetInput().GetMouseDelta();
		_yaw += mouseDelta.x * _mouseSensitivity;
		_pitch = std::clamp(_pitch + mouseDelta.y * _mouseSensitivity, _minPitch, _maxPitch);
	}

	Vector3 CameraFollowComponent::GetTargetPosition() const
	{
		Matrix socketWorldMatrix = _targetSocketComponent->GetSocketWorldMatrix(_socketName);
		Vector3 scale{};
		Quaternion rotation{};
		Vector3 position{};
		socketWorldMatrix.Decompose(scale, rotation, position);

		return position;
	}

	Vector3 CameraFollowComponent::CalcCameraPosition(const Vector3& targetPosition) const
	{
		const float horizontalDistance = _distance * std::cos(_pitch);
		Vector3 offset
		{
			-horizontalDistance * std::sin(_yaw),
			_distance * std::sin(_pitch) + _height,
			-horizontalDistance * std::cos(_yaw),
		};

		if (_isLimitBottom)
			offset.y = std::max(offset.y, _bottomDistanceLimit);

		return targetPosition + offset;
	}
}
