#include "ContinuousRotationComponent.h"
#include "GameObject.h"
#include "MathUtil.h"
#include "TransformComponent.h"

namespace gm
{
	ContinuousRotationComponent::ContinuousRotationComponent(const ContinuousRotationDesc& desc)
		: _desc(desc)
	{}

	void ContinuousRotationComponent::OnInitialize()
	{
		GM_ASSERT_RETURN(_desc.axis.LengthSquared() > 0.000001f, "ContinuousRotationComponent의 Axis는 0벡터일 수 없습니다.");
		_transform = GetOwner().GetTransform();
		GM_ASSERT_RETURN(_transform, "ContinuousRotationComponent는 TransformComponent가 필요합니다.");
		_rotationAxis = _desc.axis;
		_rotationAxis.Normalize();
	}

	void ContinuousRotationComponent::OnTick(float deltaTime)
	{
		if (_transform == nullptr || _desc.angularSpeedDegrees == 0.f)
			return;

		const float angleRadians = Math::DegreesToRadians(_desc.angularSpeedDegrees * deltaTime);
		Quaternion rotation = _transform->GetRotation() * Quaternion::CreateFromAxisAngle(_rotationAxis, angleRadians);
		rotation.Normalize();
		_transform->SetRotation(rotation);
	}
}
