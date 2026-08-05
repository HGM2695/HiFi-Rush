#include "BeatSkeletalAnimationSyncComponent.h"
#include "BeatMath.h"
#include "BeatSystem.h"
#include "SkeletalAnimationClip.h"
#include "SkeletalAnimatorComponent.h"

namespace gm
{
	BeatSkeletalAnimationSyncComponent::BeatSkeletalAnimationSyncComponent(const BeatSystem& beatSystem, SkeletalAnimatorComponent& animator, const BeatSkeletalAnimationSyncDesc& desc)
		: _beatSystem(beatSystem), _animator(animator), _desc(desc)
	{}

	void BeatSkeletalAnimationSyncComponent::OnInitialize()
	{
		GM_ASSERT_RETURN(_desc.cycleBeats > 0.f, "BeatSkeletalAnimationSyncComponent의 cycleBeats는 0보다 커야 합니다.");
	}

	void BeatSkeletalAnimationSyncComponent::OnTick(float)
	{
		if (_beatSystem.HasPlaybackTime() == false)
			return;

		const std::shared_ptr<SkeletalAnimationClip> clip = _animator.GetCurrentClip();
		if (clip == nullptr)
			return;

		const float cycleProgress = BeatMath::EvaluateCycleProgress(_beatSystem.GetCurrentBeat(), _desc.cycleBeats, _desc.phaseOffsetBeats);
		_animator.SetExternalPlayTime(clip->GetLength() * cycleProgress);
	}
}
