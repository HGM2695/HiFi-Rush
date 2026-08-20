#include "BeatTriggeredRotationShakeComponent.h"
#include "BeatSystem.h"
#include "GameObject.h"
#include "GameplayScene.h"
#include "MathUtil.h"
#include "TransformComponent.h"

#include <algorithm>

namespace gm
{
	BeatTriggeredRotationShakeComponent::BeatTriggeredRotationShakeComponent(const BeatSystem& beatSystem, const BeatTriggeredRotationShakeDesc& desc)
		: _beatSystem(beatSystem), _desc(desc)
	{
	}

	void BeatTriggeredRotationShakeComponent::StartShake(float startBeat)
	{
		GM_ASSERT_RETURN(_transform, "BeatTriggeredRotationShakeComponent는 Initialize 이후 실행해야 합니다.");
		if (_beatSystem.HasPlaybackTime() == false)
			return;

		_startBeat = startBeat;
		_isShaking = true;
	}

	void BeatTriggeredRotationShakeComponent::ResetShake()
	{
		GM_ASSERT_RETURN(_transform, "BeatTriggeredRotationShakeComponent는 Initialize 이후 초기화해야 합니다.");
		_transform->SetRotation(_initialRotation);
		_startBeat = 0.f;
		_isShaking = false;
	}

	float BeatTriggeredRotationShakeComponent::EvaluateAngle(float progress) const
	{
		if (progress < 0.25f)
			return _desc.angleDegrees * progress / 0.25f;
		if (progress < 0.75f)
			return _desc.angleDegrees + (-2.f * _desc.angleDegrees) * (progress - 0.25f) / 0.5f;

		return -_desc.angleDegrees + _desc.angleDegrees * (progress - 0.75f) / 0.25f;
	}

	void BeatTriggeredRotationShakeComponent::OnInitialize()
	{
		GM_ASSERT_RETURN(_desc.durationBeats > 0.f, "Rotation Shake Duration은 0보다 커야 합니다.");
		GM_ASSERT_RETURN(_desc.axis.LengthSquared() > 0.000001f, "Rotation Shake Axis는 0벡터일 수 없습니다.");

		_transform = GetOwner().GetTransform();
		GM_ASSERT_RETURN(_transform, "BeatTriggeredRotationShakeComponent는 TransformComponent가 필요합니다.");
		_axis = _desc.axis;
		_axis.Normalize();
		_initialRotation = _transform->GetRotation();

		GameplayScene* scene = dynamic_cast<GameplayScene*>(GetOwner().GetScene());
		GM_ASSERT_RETURN(scene, "BeatTriggeredRotationShakeComponent는 GameplayScene에서만 사용할 수 있습니다.");
		GM_ASSERT_RETURN(_triggerBinding.Bind(scene->GetTriggerSystem(), _desc.triggerId, _desc.beatOffset,
			[this](float startBeat) { StartShake(startBeat); },
			[this]() { ResetShake(); }), "BeatTriggeredRotationShakeComponent의 Trigger Binding에 실패했습니다.");
	}

	void BeatTriggeredRotationShakeComponent::OnTick(float)
	{
		if (_transform == nullptr || _isShaking == false || _beatSystem.HasPlaybackTime() == false)
			return;

		const float progress = std::clamp((_beatSystem.GetCurrentBeat() - _startBeat) / _desc.durationBeats, 0.f, 1.f);
		const Quaternion deltaRotation = Quaternion::CreateFromAxisAngle(_axis, Math::DegreesToRadians(EvaluateAngle(progress)));
		Quaternion rotation = _initialRotation * deltaRotation;
		rotation.Normalize();
		_transform->SetRotation(rotation);

		if (progress >= 1.f)
			ResetShake();
	}
}
