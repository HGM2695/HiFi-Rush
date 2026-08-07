#include "BeatTriggeredSkeletalAnimationComponent.h"
#include "AnimationTypes.h"
#include "BeatSystem.h"
#include "GameObject.h"
#include "SkeletalAnimatorComponent.h"

#include <utility>

namespace gm
{
	BeatTriggeredSkeletalAnimationComponent::BeatTriggeredSkeletalAnimationComponent(const BeatSystem& beatSystem, SkeletalAnimatorComponent& animator, BeatTriggeredSkeletalAnimationDesc desc)
		: _beatSystem(beatSystem), _animator(animator), _desc(std::move(desc))
	{}

	void BeatTriggeredSkeletalAnimationComponent::Schedule(float startBeat)
	{
		if (_state != PlaybackState::Inactive)
			return;

		_startBeat = startBeat;
		_state = PlaybackState::Scheduled;
	}

	void BeatTriggeredSkeletalAnimationComponent::Reset()
	{
		GetOwner().SetRender(false);
		_animator.Reset();
		_startBeat = 0.f;
		_state = PlaybackState::Inactive;
	}

	void BeatTriggeredSkeletalAnimationComponent::OnInitialize()
	{
		GM_ASSERT_RETURN(_desc.clipName.empty() == false, "BeatTriggeredSkeletalAnimationComponent의 Clip 이름이 비어 있습니다.");
		GM_ASSERT_RETURN(_animator.HasClip(_desc.clipName), "BeatTriggeredSkeletalAnimationComponent가 요청한 Clip이 없습니다. clip=%ls", _desc.clipName.c_str());
		Reset();
	}

	void BeatTriggeredSkeletalAnimationComponent::OnTick(float)
	{
		if (_state == PlaybackState::Scheduled)
		{
			if (_beatSystem.HasPlaybackTime() == false || _beatSystem.GetCurrentBeat() < _startBeat)
				return;

			AnimationPlayOption playOption{};
			playOption.loopOverride = false;
			GM_ASSERT_RETURN(_animator.Play(_desc.clipName, playOption), "트리거 스켈레탈 애니메이션 재생에 실패했습니다. clip=%ls", _desc.clipName.c_str());
			GetOwner().SetRender(true);
			_state = PlaybackState::Playing;
			return;
		}

		if (_state != PlaybackState::Playing || _animator.GetState() != AnimationState::Completed)
			return;

		GetOwner().SetRender(false);
		_animator.Reset();
		_state = PlaybackState::Inactive;
	}
}
