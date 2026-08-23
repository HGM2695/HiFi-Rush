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
		_worldPositionMove.reset();
		_target.Reset();
		_targetSocketComponent = nullptr;
		_socketName.clear();
	}

	void CameraFollowComponent::StartWorldPositionMove(const Vector3& targetPosition, float duration)
	{
		_worldPositionMove = WorldPositionMove{ targetPosition, std::max(0.f, duration), 0.f };
	}

	void CameraFollowComponent::StopWorldPositionMove()
	{
		if (_worldPositionMove == std::nullopt)
			return;

		if (_ownerTransform && _target.IsValid() && _targetSocketComponent)
			SyncOrbitFromWorldPosition(_ownerTransform->GetPosition(), GetFollowTargetPosition());
		_worldPositionMove.reset();
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

		if (_worldPositionMove == std::nullopt)
			UpdateOrbitInput();
		const float distanceInterpolationRatio = 1.f - std::exp(-6.32163094f * std::max(0.f, deltaTime));
		_distance += (_targetDistance - _distance) * distanceInterpolationRatio;

		const Vector3 targetPosition = GetFollowTargetPosition();
		const Vector3 cameraPosition = _worldPositionMove ? UpdateWorldPositionMove(deltaTime, targetPosition) : CalcCameraPosition(targetPosition);

		Matrix viewMatrix = Math::CreateLookAtLH(cameraPosition, targetPosition, Vector3{ 0.f, 1.f, 0.f });
		Matrix cameraWorldMatrix = viewMatrix.Invert();
		
		_ownerTransform->SetWorldMatrix(cameraWorldMatrix);
	}

	Vector3 CameraFollowComponent::UpdateWorldPositionMove(float deltaTime, const Vector3& targetPosition)
	{
		WorldPositionMove& move = _worldPositionMove.value();
		move.elapsed += std::max(0.f, deltaTime);
		const float interpolationRatio = move.duration > 0.f ? 1.f - std::exp(-6.32163094f * std::max(0.f, deltaTime)) : 1.f;
		Vector3 cameraPosition = Vector3::Lerp(_ownerTransform->GetPosition(), move.targetPosition, interpolationRatio);
		if (_isLimitBottom)
			cameraPosition.y = std::max(cameraPosition.y, targetPosition.y + _bottomDistanceLimit);
		if (move.elapsed >= move.duration)
		{
			SyncOrbitFromWorldPosition(cameraPosition, targetPosition);
			_worldPositionMove.reset();
		}
		return cameraPosition;
	}

	void CameraFollowComponent::SyncOrbitFromWorldPosition(const Vector3& cameraPosition, const Vector3& targetPosition)
	{
		Vector3 offset = cameraPosition - targetPosition;
		offset.y -= _height;
		const float distance = offset.Length();
		if (distance <= 0.0001f)
			return;

		_distance = distance;
		_yaw = std::atan2(-offset.x, -offset.z);
		_pitch = std::clamp(std::asin(std::clamp(offset.y / distance, -1.f, 1.f)), _minPitch, _maxPitch);
	}

	void CameraFollowComponent::UpdateOrbitInput()
	{
		if (_mouseControlEnabled == false)
			return;

		const Vector2 mouseDelta = APPLICATION.GetInput().GetMouseDelta();
		_yaw += mouseDelta.x * _mouseSensitivity;
		_pitch = std::clamp(_pitch + mouseDelta.y * _mouseSensitivity, _minPitch, _maxPitch);
	}

	Vector3 CameraFollowComponent::GetFollowTargetPosition() const
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
