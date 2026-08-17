#include "BeatAudioLevelMoveComponent.h"
#include "BeatMath.h"
#include "BeatSystem.h"
#include "GameObject.h"
#include "TransformComponent.h"

namespace gm
{
	BeatAudioLevelMoveComponent::BeatAudioLevelMoveComponent(const BeatSystem& beatSystem, const BeatAudioLevelMoveDesc& desc)
		: _beatSystem(beatSystem), _desc(desc)
	{}

	void BeatAudioLevelMoveComponent::OnInitialize()
	{
		GM_ASSERT_RETURN(_desc.cycleBeats > 0.f, "BeatAudioLevelMoveComponent의 cycleBeats는 0보다 커야 합니다.");
		GM_ASSERT_RETURN(_desc.direction.LengthSquared() > 0.000001f, "BeatAudioLevelMoveComponent의 direction은 0벡터일 수 없습니다.");

		_transform = GetOwner().GetTransform();
		GM_ASSERT_RETURN(_transform, "BeatAudioLevelMoveComponent는 TransformComponent가 필요합니다.");

		_initialPosition = _transform->GetPosition();
		_direction = _desc.direction;
		_direction.Normalize();
	}

	void BeatAudioLevelMoveComponent::OnTick(float)
	{
		if (_transform == nullptr || _beatSystem.HasPlaybackTime() == false)
			return;

		const float wave = BeatMath::EvaluateBeatPulse(_beatSystem.GetCurrentBeat(), _desc.cycleBeats, _desc.phaseOffsetBeats);
		float musicPeakSq = _beatSystem.GetMusicPeak();
		musicPeakSq *= musicPeakSq;
		const float distance = wave * musicPeakSq * _desc.maxDistance * 13.f;
		_transform->SetPosition(_initialPosition + _direction * distance);
	}
}
