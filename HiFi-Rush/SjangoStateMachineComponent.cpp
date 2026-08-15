#include "SjangoStateMachineComponent.h"

#include "GameObject.h"
#include "MonsterCombatComponent.h"
#include "MonsterResourceInfo.h"
#include "SjangoAnimationTypes.h"
#include "SjangoState.h"
#include "SkeletalAnimatorComponent.h"

namespace gm
{
	void SjangoStateMachineComponent::OnInitialize()
	{
		GM_ASSERT_RETURN(RegisterAnimationClips(), "Sjango Animation Clip 등록에 실패했습니다.");
		GM_ASSERT_RETURN(RegisterStates(), "Sjango State 등록에 실패했습니다.");

		MonsterCombatComponent* combatComponent = GetOwner().GetComponent<MonsterCombatComponent>();
		GM_ASSERT_RETURN(combatComponent, "SjangoStateMachineComponent에 MonsterCombatComponent가 필요합니다.");
		combatComponent->SetCombatEnabled(false);

		SetInitialState(MonsterStateId::Idle);
		MonsterStateMachineComponent::OnInitialize();
	}

	bool SjangoStateMachineComponent::RegisterAnimationClips()
	{
		SkeletalAnimatorComponent* animator = GetOwner().GetComponent<SkeletalAnimatorComponent>();
		GM_ASSERT_RETURN_VAL(animator, false, "SjangoStateMachineComponent에 SkeletalAnimatorComponent가 필요합니다.");

		for (uint32 animationIndex = 0; animationIndex < SjangoAnimationIdCount; ++animationIndex)
		{
			const SjangoAnimationId animationId = static_cast<SjangoAnimationId>(animationIndex);
			const std::wstring clipName = GetSjangoAnimationClipName(animationId);
			if (animator->HasClip(clipName))
				continue;

			const std::wstring animationKey = GetMonsterAnimationClipKey(MonsterType::Sjango, animationIndex);
			GM_ASSERT_RETURN_VAL(animator->AddClip(clipName, animationKey), false, "Sjango Animation Clip을 찾을 수 없습니다. key=%ls", animationKey.c_str());
		}

		return true;
	}

	bool SjangoStateMachineComponent::RegisterStates()
	{
		GM_ASSERT_RETURN_VAL(RegisterState(std::make_unique<SjangoIdleState>()), false, "Sjango Idle State 등록에 실패했습니다.");
		GM_ASSERT_RETURN_VAL(RegisterState(std::make_unique<SjangoDamageState>()), false, "Sjango Damage State 등록에 실패했습니다.");
		return true;
	}
}
