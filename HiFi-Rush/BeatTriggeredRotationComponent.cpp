#include "BeatTriggeredRotationComponent.h"
#include "BeatSystem.h"
#include "GameObject.h"
#include "GameplayScene.h"
#include "MathUtil.h"
#include "TransformComponent.h"

#include <algorithm>

namespace gm
{
	BeatTriggeredRotationComponent::BeatTriggeredRotationComponent(const BeatSystem& beatSystem, const BeatTriggeredRotationDesc& desc)
		: _beatSystem(beatSystem), _desc(desc)
	{}

	void BeatTriggeredRotationComponent::Schedule(float startBeat)
	{
		GM_ASSERT_RETURN(_transform, "BeatTriggeredRotationComponent는 Initialize 이후 예약해야 합니다.");

		if (_state != RotationState::Inactive || _beatSystem.HasPlaybackTime() == false)
			return;

		_startBeat = startBeat;
		_startRotation = _transform->GetRotation();
		const Quaternion deltaRotation = Quaternion::CreateFromAxisAngle(_rotationAxis, Math::DegreesToRadians(_desc.angleDegrees));
		_targetRotation = _startRotation * deltaRotation;
		_targetRotation.Normalize();
		_state = RotationState::Rotating;
	}

	void BeatTriggeredRotationComponent::ResetAction()
	{
		GM_ASSERT_RETURN(_transform, "BeatTriggeredRotationComponent는 Initialize 이후 초기화해야 합니다.");

		_transform->SetRotation(_initialRotation);
		_startRotation = _initialRotation;
		_targetRotation = _initialRotation;
		_startBeat = 0.f;
		_state = RotationState::Inactive;
	}

	void BeatTriggeredRotationComponent::OnInitialize()
	{
		GM_ASSERT_RETURN(_desc.durationBeats > 0.f, "BeatTriggeredRotationComponent의 durationBeats는 0보다 커야 합니다.");
		GM_ASSERT_RETURN(_desc.axis.LengthSquared() > 0.000001f, "BeatTriggeredRotationComponent의 axis는 0벡터일 수 없습니다.");

		_transform = GetOwner().GetTransform();
		GM_ASSERT_RETURN(_transform, "BeatTriggeredRotationComponent는 TransformComponent가 필요합니다.");

		_rotationAxis = _desc.axis;
		_rotationAxis.Normalize();
		_initialRotation = _transform->GetRotation();
		_startRotation = _initialRotation;
		_targetRotation = _initialRotation;

		GameplayScene* scene = dynamic_cast<GameplayScene*>(GetOwner().GetScene());
		GM_ASSERT_RETURN(scene, "BeatTriggeredRotationComponent는 GameplayScene에서만 사용할 수 있습니다.");
		GM_ASSERT_RETURN(_triggerBinding.Bind(scene->GetTriggerSystem(), _desc.triggerId, _desc.beatOffset,
			[this](float startBeat) { Schedule(startBeat); },
			[this]() { ResetAction(); }), "BeatTriggeredRotationComponent의 Trigger Binding에 실패했습니다.");
	}

	void BeatTriggeredRotationComponent::OnTick(float)
	{
		if (_transform == nullptr || _state != RotationState::Rotating || _beatSystem.HasPlaybackTime() == false)
			return;

		const float elapsedBeats = _beatSystem.GetCurrentBeat() - _startBeat;
		const float progress = std::clamp(elapsedBeats / _desc.durationBeats, 0.f, 1.f);
		_transform->SetRotation(Quaternion::Slerp(_startRotation, _targetRotation, progress));

		if (progress >= 1.f)
		{
			_transform->SetRotation(_targetRotation);
			_state = RotationState::Completed;
		}
	}
}
