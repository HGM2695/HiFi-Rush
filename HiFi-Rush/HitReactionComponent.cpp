#include "HitReactionComponent.h"
#include "CombatTypes.h"
#include "GameObject.h"
#include "GameplayScene.h"
#include "HurtBoxComponent.h"
#include "SkeletalAnimatorComponent.h"
#include "TriggerSequenceSystem.h"

namespace gm
{
	HitReactionComponent::HitReactionComponent(const std::wstring& completionSequenceId, const std::wstring& reactionAnimationClipName)
		: _completionSequenceId(completionSequenceId), _reactionAnimationClipName(reactionAnimationClipName)
	{
		GM_ASSERT(completionSequenceId.empty() == false, "HitReactionComponent의 완료 Sequence ID는 비어 있을 수 없습니다.");
	}

	void HitReactionComponent::OnInitialize()
	{
		GameplayScene* scene = dynamic_cast<GameplayScene*>(GetOwner().GetScene());
		GM_ASSERT_RETURN(scene, "HitReactionComponent는 GameplayScene에서만 사용할 수 있습니다.");
		_triggerSequenceSystem = &scene->GetTriggerSequenceSystem();
		GM_ASSERT_RETURN(_triggerSequenceSystem->HasSequence(_completionSequenceId), "HitReactionComponent가 참조하는 완료 Sequence가 없습니다. sequenceId=%ls", _completionSequenceId.c_str());

		if (_reactionAnimationClipName.empty() == false)
		{
			_animator = GetOwner().GetComponent<SkeletalAnimatorComponent>();
			GM_ASSERT_RETURN(_animator, "반응 Animation을 사용하는 HitReactionComponent에는 SkeletalAnimatorComponent가 필요합니다.");
			GM_ASSERT_RETURN(_animator->HasClip(_reactionAnimationClipName), "HitReactionComponent가 요청한 반응 Animation Clip이 없습니다. clip=%ls", _reactionAnimationClipName.c_str());

			AnimationPlayOption playOption{};
			playOption.loopOverride = false;
			GM_ASSERT_RETURN(_animator->Play(_reactionAnimationClipName, playOption), "HitReactionComponent의 초기 Animation 설정에 실패했습니다. clip=%ls", _reactionAnimationClipName.c_str());
			_animator->Pause();
		}

		GetOwner().ForEachComponent(
			[this](Component& component)
			{
				auto* hurtBox = dynamic_cast<HurtBoxComponent*>(&component);
				if (hurtBox == nullptr)
					return;

				auto connection = std::make_unique<EventConnection>();
				hurtBox->OnHurt.Subscribe(*connection,
					[this](const HitEvent& event)
					{
						HandleHurt(event);
					});
				_hurtConnections.push_back(std::move(connection));
			});

		GM_ASSERT_RETURN(_hurtConnections.empty() == false, "HitReactionComponent를 가진 GameObject에는 하나 이상의 HurtBoxComponent가 필요합니다.");
	}

	void HitReactionComponent::OnTick(float)
	{
		if (_state != State::PlayingAnimation || _animator == nullptr || _animator->GetState() != AnimationState::Completed)
			return;

		ActivateSequence();
	}

	void HitReactionComponent::HandleHurt(const HitEvent& event)
	{
		if (_state != State::WaitingForHit || event.damageResult.state != DamageState::Applied || _triggerSequenceSystem == nullptr)
			return;

		if (_animator == nullptr)
		{
			ActivateSequence();
			return;
		}

		AnimationPlayOption playOption{};
		playOption.loopOverride = false;
		GM_ASSERT_RETURN(_animator->Play(_reactionAnimationClipName, playOption), "HitReactionComponent의 반응 Animation 재생에 실패했습니다. clip=%ls", _reactionAnimationClipName.c_str());
		_state = State::PlayingAnimation;
	}

	void HitReactionComponent::ActivateSequence()
	{
		GM_ASSERT_RETURN(_triggerSequenceSystem, "HitReactionComponent의 TriggerSequenceSystem이 유효하지 않습니다.");
		GM_ASSERT_RETURN(_triggerSequenceSystem->Activate(_completionSequenceId), "HitReactionComponent의 완료 Sequence 실행에 실패했습니다. sequenceId=%ls", _completionSequenceId.c_str());
		_state = State::Completed;
	}
}
