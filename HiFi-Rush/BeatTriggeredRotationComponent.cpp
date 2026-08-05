#include "BeatTriggeredRotationComponent.h"
#include "BeatSystem.h"
#include "GameObject.h"
#include "MathUtil.h"
#include "TransformComponent.h"

#if GM_ENABLE_DEBUG_TOOLS
#include "Application.h"
#include "DebugEventPublisher.h"
#include "HiFiRushGameInstance.h"
#endif

#include <algorithm>

namespace gm
{
	BeatTriggeredRotationComponent::BeatTriggeredRotationComponent(const BeatSystem& beatSystem, const BeatTriggeredRotationDesc& desc)
		: _beatSystem(beatSystem), _desc(desc)
	{}

	void BeatTriggeredRotationComponent::Activate()
	{
		GM_ASSERT_RETURN(_transform, "BeatTriggeredRotationComponent는 Initialize 이후 활성화해야 합니다.");

		if (_state != RotationState::Inactive || _beatSystem.HasPlaybackTime() == false)
			return;

		_startBeat = _beatSystem.GetCurrentBeat();
		_startRotation = _transform->GetRotation();
		const Quaternion deltaRotation = Quaternion::CreateFromAxisAngle(_rotationAxis, Math::DegreesToRadians(_desc.angleDegrees));
		_targetRotation = _startRotation * deltaRotation;
		_targetRotation.Normalize();
		_state = RotationState::Rotating;
	}

	void BeatTriggeredRotationComponent::Reset()
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

#if GM_ENABLE_DEBUG_TOOLS
		HiFiRushGameInstance& gameInstance = static_cast<HiFiRushGameInstance&>(APPLICATION.GetGameInstance());
		gameInstance.GetDebugEventPublisher().OnDebugEvent.Subscribe(_debugEventConnection,
			[this](const DebugEvent& event)
			{
				if (event.type == DebugEventType::Activate)
					Activate();
				else if (event.type == DebugEventType::Reset)
					Reset();
			});
#endif
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
