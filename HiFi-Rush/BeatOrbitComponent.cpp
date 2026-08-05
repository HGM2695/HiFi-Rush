#include "BeatOrbitComponent.h"
#include "BeatSystem.h"
#include "GameObject.h"
#include "MathUtil.h"
#include "TransformComponent.h"

#include <cmath>

namespace gm
{
	BeatOrbitComponent::BeatOrbitComponent(const BeatSystem& beatSystem, const BeatOrbitDesc& desc)
		: _beatSystem(beatSystem), _desc(desc)
	{}

	void BeatOrbitComponent::OnInitialize()
	{
		GM_ASSERT_RETURN(_desc.interpolationSpeed > 0.f, "BeatOrbitComponent의 interpolationSpeed는 0보다 커야 합니다.");

		_transform = GetOwner().GetTransform();
		GM_ASSERT_RETURN(_transform, "BeatOrbitComponent는 TransformComponent가 필요합니다.");

		const Vector3 initialPosition = _transform->GetPosition();
		Vector3 centerDirection = initialPosition - _desc.center;
		_heightOffset = centerDirection.y;
		centerDirection.y = 0.f;
		_radius = centerDirection.Length();
		_currentOrbitAngle = std::atan2(centerDirection.z, centerDirection.x);
		_targetOrbitAngle = _currentOrbitAngle;
	}

	void BeatOrbitComponent::OnTick(float deltaTime)
	{
		if (_transform == nullptr || _beatSystem.HasPlaybackTime() == false)
			return;

		if (_beatSystem.DidCrossBeatBoundary())
			UpdateTargetAngle();

		const float ratio = Math::CalcExponentialSmoothingRatio(_desc.interpolationSpeed, deltaTime);
		_currentOrbitAngle += (_targetOrbitAngle - _currentOrbitAngle) * ratio;
		ApplyTransform();
	}

	void BeatOrbitComponent::UpdateTargetAngle()
	{
		const bool isEvenBeat = _beatSystem.GetCurrentBeatIndex() % 2 == 0;
		const float deltaDegrees = isEvenBeat ? _desc.evenBeatDeltaDegrees : _desc.oddBeatDeltaDegrees;
		_targetOrbitAngle = _currentOrbitAngle + Math::DegreesToRadians(deltaDegrees);
	}

	void BeatOrbitComponent::ApplyTransform()
	{
		Vector3 position = _desc.center;
		position.x += std::cos(_currentOrbitAngle) * _radius;
		position.y += _heightOffset;
		position.z += std::sin(_currentOrbitAngle) * _radius;
		_transform->SetPosition(position);

		if (_desc.faceCenter)
		{
			Vector3 centerDirection = _desc.center - position;
			centerDirection.y = 0.f;
			if (centerDirection.LengthSquared() > 0.000001f)
				_transform->SetRotationY(std::atan2(centerDirection.x, centerDirection.z));
		}
	}
}
