#include "QamilState.h"

#include "BeatSystem.h"
#include "GameObject.h"
#include "SkeletalAnimationClip.h"
#include "SkeletalAnimatorComponent.h"
#include "TransformComponent.h"

namespace gm
{
	namespace
	{
		constexpr Vector3 QamilBasePlatformPosition{ 0.f, 0.f, -15.f };
		constexpr float QamilPlatformAreaThreshold = -1.f;
	}

	bool QamilState::PlayAnimation(QamilStateContext& context, QamilAnimationId animationId, bool isLoop, float blendDuration) const
	{
		GM_ASSERT_RETURN_VAL(context.animatorComponent, false, "Qamil State에 SkeletalAnimatorComponent가 연결되지 않았습니다.");
		AnimationPlayOption playOption{};
		playOption.loopOverride = isLoop;
		playOption.blendDuration = blendDuration;
		return context.animatorComponent->Play(GetQamilAnimationClipName(animationId), playOption);
	}

	bool QamilState::PlayBeatSyncedAnimation(QamilStateContext& context, QamilAnimationId animationId, bool isLoop, float blendDuration) const
	{
		if (PlayAnimation(context, animationId, isLoop, blendDuration) == false)
			return false;

		context.animatorComponent->SetPlayRate(GetBasePlayRate(context));
		return true;
	}

	bool QamilState::IsAnimationCompleted(const QamilStateContext& context) const
	{
		return context.animatorComponent != nullptr && context.animatorComponent->GetState() == AnimationState::Completed;
	}

	float QamilState::GetAnimationBeat(const QamilStateContext& context) const
	{
		if (context.animatorComponent == nullptr)
			return 0.f;
		const std::shared_ptr<SkeletalAnimationClip> clip = context.animatorComponent->GetCurrentClip();
		if (clip == nullptr || clip->GetTicksPerSecond() <= 0.f)
			return 0.f;
		return context.animatorComponent->GetPlayTime() / (QamilAnimationTicksPerBeat / clip->GetTicksPerSecond());
	}

	float QamilState::GetBasePlayRate(const QamilStateContext& context) const
	{
		if (context.animatorComponent == nullptr || context.beatSystem == nullptr || context.beatSystem->GetSecondsPerBeat() <= 0.f)
			return 1.f;

		const std::shared_ptr<SkeletalAnimationClip> clip = context.animatorComponent->GetCurrentClip();
		if (clip == nullptr || clip->GetTicksPerSecond() <= 0.f)
			return 1.f;

		const float animationSecondsPerBeat = QamilAnimationTicksPerBeat / clip->GetTicksPerSecond();
		return animationSecondsPerBeat / context.beatSystem->GetSecondsPerBeat();
	}

	Vector3 QamilState::GetCurrentPlatformPosition(const QamilStateContext& context) const
	{
		if (context.transformComponent == nullptr)
			return {};

		return Vector3::Transform(QamilBasePlatformPosition, context.transformComponent->GetWorldMatrix());
	}

	bool QamilState::IsTargetInCurrentPlatformArea(const QamilStateContext& context) const
	{
		const GameObject* target = context.target.Get();
		if (target == nullptr || target->GetTransform() == nullptr || context.transformComponent == nullptr)
			return false;

		const Vector3 arenaCenter = context.transformComponent->GetPosition();
		Vector3 platformDirection = GetCurrentPlatformPosition(context) - arenaCenter;
		platformDirection.y = 0.f;
		if (platformDirection.LengthSquared() <= 0.000001f)
			return true;

		platformDirection.Normalize();
		const Vector3 targetOffset = target->GetTransform()->GetPosition() - arenaCenter;
		return targetOffset.Dot(platformDirection) >= QamilPlatformAreaThreshold;
	}

	bool QamilState::SelectClockwiseDirection(const QamilStateContext& context) const
	{
		const GameObject* target = context.target.Get();
		if (target == nullptr || target->GetTransform() == nullptr || context.transformComponent == nullptr)
			return true;

		const Vector3 platformPosition = GetCurrentPlatformPosition(context);
		const Vector3 qamilToCenter = context.transformComponent->GetPosition() - platformPosition;
		const Vector3 qamilToTarget = target->GetTransform()->GetPosition() - platformPosition;
		return qamilToCenter.Cross(qamilToTarget).y < 0.f;
	}
}
