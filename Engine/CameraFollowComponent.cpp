#include "CameraFollowComponent.h"
#include "GameObject.h"
#include "SocketComponent.h"
#include "TransformComponent.h"
#include "MathUtil.h"

namespace gm
{
	void CameraFollowComponent::SetTarget(const GameObject& target, const std::wstring& socketName)
	{
		_target = target.GetWeakPtr();
		_targetSocketComponent = target.GetComponent<SocketComponent>();
		_socketName = socketName;

		GM_ASSERT_RETURN(_targetSocketComponent, "Target GameObject에 SocketComponent가 존재하지 않습니다.");
		GM_ASSERT_RETURN(_socketName.empty() == false, "CameraFollowComponent Socket 이름이 비어 있습니다.");
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

		const Vector3 targetPosition = GetTargetPosition();
		const Vector3 cameraPosition = CalcCameraPosition(targetPosition);

		Matrix viewMatrix = Math::CreateLookAtLH(cameraPosition, targetPosition, Vector3{ 0.f, 1.f, 0.f });
		Matrix cameraWorldMatrix = viewMatrix.Invert();
		
		_ownerTransform->SetWorldMatrix(cameraWorldMatrix);
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
		const Quaternion orbitRotation = Quaternion::CreateFromYawPitchRoll(_yaw, _pitch, 0.f);
		Vector3 offset = Vector3::Transform(Vector3{ 0.f, 0.f, -_distance }, orbitRotation);
		offset.y += _height;

		return targetPosition + offset;
	}
}
