#include "BeatSkeletalAnimationSyncComponent.h"
#include "BeatMath.h"
#include "BeatSystem.h"
#include "SkeletalAnimationClip.h"
#include "SkeletalAnimatorComponent.h"

namespace gm
{
	BeatSkeletalAnimationSyncComponent::BeatSkeletalAnimationSyncComponent(const BeatSystem& beatSystem, SkeletalAnimatorComponent& animator, const BeatSkeletalAnimationSyncDesc& defaultDesc)
		: _beatSystem(beatSystem), _animator(animator), _defaultDesc(defaultDesc)
	{}

	void BeatSkeletalAnimationSyncComponent::OnInitialize()
	{
		GM_ASSERT_RETURN(_defaultDesc.cycleBeats > 0.f || _clipSyncRules.empty() == false, "BeatSkeletalAnimationSyncComponent는 기본 주기 또는 하나 이상의 동기화 규칙이 필요합니다.");
	}

	void BeatSkeletalAnimationSyncComponent::OnTick(float)
	{
		if (_beatSystem.HasPlaybackTime() == false)
			return;

		const std::shared_ptr<SkeletalAnimationClip> clip = _animator.GetCurrentClip();
		if (clip == nullptr)
			return;

		const BeatSkeletalAnimationSyncDesc* syncDesc = FindSyncDesc();
		if (syncDesc == nullptr)
			return;

		const float playTime = BeatMath::EvaluateCycleTime(_beatSystem.GetCurrentBeat(), syncDesc->cycleBeats, clip->GetLength(), syncDesc->phaseOffsetBeats);
		_animator.SetExternalPlayTime(playTime);
	}

	bool BeatSkeletalAnimationSyncComponent::AddClipSyncRule(const std::wstring& clipName, const BeatSkeletalAnimationSyncDesc& desc)
	{
		GM_ASSERT_RETURN_VAL(clipName.empty() == false, false, "동기화할 SkeletalAnimationClip 이름이 비어 있습니다.");
		GM_ASSERT_RETURN_VAL(desc.cycleBeats > 0.f, false, "애니메이션 동기화 주기는 0보다 커야 합니다.");

		const bool inserted = _clipSyncRules.emplace(clipName, desc).second;
		GM_ASSERT_RETURN_VAL(inserted, false, "이미 등록된 애니메이션 동기화 규칙입니다. clip=%ls", clipName.c_str());
		return true;
	}

	const BeatSkeletalAnimationSyncDesc* BeatSkeletalAnimationSyncComponent::FindSyncDesc() const
	{
		if (_clipSyncRules.empty())
			return _defaultDesc.cycleBeats > 0.f ? &_defaultDesc : nullptr;

		auto iter = _clipSyncRules.find(_animator.GetCurrentClipName());
		if (iter != _clipSyncRules.end())
			return &iter->second;

		return nullptr;
	}
}
