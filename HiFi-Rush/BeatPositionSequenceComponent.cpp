#include "BeatPositionSequenceComponent.h"
#include "BeatSystem.h"
#include "GameObject.h"
#include "MathUtil.h"
#include "TransformComponent.h"

#include <algorithm>
#include <cmath>

namespace gm
{
	BeatPositionSequenceComponent::BeatPositionSequenceComponent(const BeatSystem& beatSystem, const BeatPositionSequenceDesc& desc)
		: _beatSystem(beatSystem), _desc(desc)
	{}

	void BeatPositionSequenceComponent::OnInitialize()
	{
		GM_ASSERT_RETURN(_desc.positionOffsets.empty() == false, "BeatPositionSequenceComponent는 하나 이상의 위치 오프셋이 필요합니다.");
		GM_ASSERT_RETURN(_desc.beatsPerStep > 0.f, "BeatPositionSequenceComponent의 beatsPerStep은 0보다 커야 합니다.");
		GM_ASSERT_RETURN(_desc.interpolationSpeed > 0.f, "BeatPositionSequenceComponent의 interpolationSpeed는 0보다 커야 합니다.");

		_transform = GetOwner().GetTransform();
		GM_ASSERT_RETURN(_transform, "BeatPositionSequenceComponent는 TransformComponent가 필요합니다.");
		_initialPosition = _transform->GetPosition();
	}

	void BeatPositionSequenceComponent::OnTick(float deltaTime)
	{
		if (_transform == nullptr || _beatSystem.HasPlaybackTime() == false)
			return;

		const float sequenceBeats = static_cast<float>(_desc.positionOffsets.size()) * _desc.beatsPerStep;
		float sequenceBeat = std::fmod(_beatSystem.GetCurrentBeat(), sequenceBeats);
		if (sequenceBeat < 0.f)
			sequenceBeat += sequenceBeats;

		const size_t stepIndex = static_cast<size_t>(sequenceBeat / _desc.beatsPerStep);
		const Vector3 targetPosition = _initialPosition + _desc.positionOffsets[stepIndex];
		const float interpolationRatio = Math::CalcExponentialSmoothingRatio(_desc.interpolationSpeed, deltaTime);
		_transform->SetPosition(Vector3::Lerp(_transform->GetPosition(), targetPosition, interpolationRatio));
	}
}
