#include "BeatMoveComponent.h"
#include "BeatSystem.h"
#include "GameObject.h"
#include "MathUtil.h"
#include "TransformComponent.h"

#include <algorithm>
#include <cmath>

namespace gm
{
	BeatMoveComponent::BeatMoveComponent(const BeatSystem& beatSystem, const BeatMoveDesc& desc)
		: _beatSystem(beatSystem), _desc(desc)
	{}

	void BeatMoveComponent::Activate()
	{
		GM_ASSERT_RETURN(_transform, "BeatMoveComponent는 Initialize 이후에 활성화해야 합니다.");

		if (_state != MoveState::Inactive || _beatSystem.HasPlaybackTime() == false)
			return;

		if (_beatSystem.HasPlaybackTime())
			Schedule(std::floor(_beatSystem.GetCurrentBeat()) + 1.f);
	}

	void BeatMoveComponent::Schedule(float startBeat)
	{
		GM_ASSERT_RETURN(_transform, "BeatMoveComponent는 Initialize 이후 예약해야 합니다.");

		if (_state != MoveState::Inactive)
			return;

		_startPosition = _transform->GetPosition();
		_startBeat = startBeat;
		_state = MoveState::Scheduled;
	}

	void BeatMoveComponent::Reset()
	{
		GM_ASSERT_RETURN(_transform, "BeatMoveComponent는 Initialize 이후에 초기화해야 합니다.");

		_transform->SetPosition(_initialPosition);
		_startPosition = _initialPosition;
		_startBeat = 0.f;
		_state = MoveState::Inactive;
	}

	void BeatMoveComponent::OnInitialize()
	{
		GM_ASSERT_RETURN(_desc.durationBeats > 0.f, "BeatMoveComponent의 durationBeats는 0보다 커야 합니다.");

		_transform = GetOwner().GetTransform();
		GM_ASSERT_RETURN(_transform, "BeatMoveComponent는 TransformComponent가 필요합니다.");
		_initialPosition = _transform->GetPosition();
	}

	void BeatMoveComponent::OnTick(float)
	{
		if (_transform == nullptr || _state == MoveState::Inactive || _beatSystem.HasPlaybackTime() == false)
			return;

		const float elapsedBeats = _beatSystem.GetCurrentBeat() - _startBeat;
		if (elapsedBeats < 0.f)
			return;

		const float progress = std::clamp(elapsedBeats / _desc.durationBeats, 0.f, 1.f);
		const float easedProgress = std::sin(progress * Math::GM_PI * 0.5f);
		_transform->SetPosition(Vector3::Lerp(_startPosition, _desc.targetPosition, easedProgress));

		if (progress >= 1.f)
		{
			_transform->SetPosition(_desc.targetPosition);
			_state = MoveState::Inactive;
		}
	}
}
