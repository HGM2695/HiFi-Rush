#include "BeatTriggeredSkeletalAnimationComponent.h"
#include "AnimationTypes.h"
#include "BeatSystem.h"
#include "Collider3DComponent.h"
#include "GameObject.h"
#include "GameplayScene.h"
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

	void BeatTriggeredSkeletalAnimationComponent::ResetAction()
	{
		GetOwner().SetRender(_desc.initiallyVisible);
		for (const ColliderState& colliderState : _colliderStates)
			colliderState.collider->SetEnabled(colliderState.wasEnabled);
		_animator.Reset();
		_startBeat = 0.f;
		_state = PlaybackState::Inactive;
	}

	void BeatTriggeredSkeletalAnimationComponent::OnInitialize()
	{
		GM_ASSERT_RETURN(_desc.clipName.empty() == false, "BeatTriggeredSkeletalAnimationComponent의 Clip 이름이 비어 있습니다.");
		GM_ASSERT_RETURN(_animator.HasClip(_desc.clipName), "BeatTriggeredSkeletalAnimationComponent가 요청한 Clip이 없습니다. clip=%ls", _desc.clipName.c_str());
		if (_desc.disableCollidersWhenCompleted)
		{
			for (Collider3DComponent* collider : GetOwner().GetColliders3D())
				_colliderStates.push_back({ collider, collider->IsEnabled() });
		}
		ResetAction();

		GameplayScene* scene = dynamic_cast<GameplayScene*>(GetOwner().GetScene());
		GM_ASSERT_RETURN(scene, "BeatTriggeredSkeletalAnimationComponent는 GameplayScene에서만 사용할 수 있습니다.");
		GM_ASSERT_RETURN(_triggerBinding.Bind(scene->GetTriggerSystem(), _desc.triggerId, _desc.beatOffset,
			[this](float startBeat) { Schedule(startBeat); },
			[this]() { ResetAction(); }), "BeatTriggeredSkeletalAnimationComponent의 Trigger Binding에 실패했습니다.");
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

		if (_desc.hideWhenCompleted)
		{
			GetOwner().SetRender(false);
			_animator.Reset();
		}
		for (const ColliderState& colliderState : _colliderStates)
			colliderState.collider->SetEnabled(false);
		_state = PlaybackState::Inactive;
	}
}
