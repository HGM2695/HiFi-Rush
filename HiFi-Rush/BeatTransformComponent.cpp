#include "BeatTransformComponent.h"
#include "BeatMath.h"
#include "BeatSystem.h"
#include "GameObject.h"
#include "TransformComponent.h"

namespace gm
{
	BeatTransformComponent::BeatTransformComponent(const BeatSystem& beatSystem, const BeatTransformDesc& desc)
		: _beatSystem(beatSystem), _desc(desc)
	{}

	void BeatTransformComponent::OnInitialize()
	{
		GM_ASSERT_RETURN(_desc.cycleBeats > 0.f, "BeatTransformComponent의 cycleBeats는 0보다 커야 합니다.");

		_transform = GetOwner().GetTransform();
		GM_ASSERT_RETURN(_transform, "BeatTransformComponent는 TransformComponent가 필요합니다.");

		_initialPosition = _transform->GetPosition();
		_initialScale = _transform->GetScale();
	}

	void BeatTransformComponent::OnTick(float)
	{
		if (_transform == nullptr || _beatSystem.HasPlaybackTime() == false)
			return;

		const float ratio = BeatMath::EvaluateBeatBoundaryPulse(_beatSystem.GetCurrentBeat(), _desc.cycleBeats, _desc.phaseOffsetBeats);
		switch (_desc.type)
		{
		case BeatTransformType::PositionOffset:
			_transform->SetPosition(_initialPosition + _desc.positionOffset * ratio);
			break;

		case BeatTransformType::ScaleMultiplier:
			_transform->SetScale(Vector3{
				_initialScale.x * (1.f + (_desc.maxScaleMultiplier.x - 1.f) * ratio),
				_initialScale.y * (1.f + (_desc.maxScaleMultiplier.y - 1.f) * ratio),
				_initialScale.z * (1.f + (_desc.maxScaleMultiplier.z - 1.f) * ratio) });
			break;
		}
	}
}
