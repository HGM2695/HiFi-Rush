#include "SwordStateMachineComponent.h"

#include "GameObject.h"
#include "HitBoxComponent.h"
#include "MonsterResourceInfo.h"
#include "MonsterTypes.h"
#include "SkeletalAnimatorComponent.h"
#include "SwordAnimationTypes.h"
#include "SwordState.h"

namespace gm
{
	SwordStateMachineComponent::SwordStateMachineComponent(float attackRangeMin, float attackRangeMax)
		: _attackRangeMin(attackRangeMin), _attackRangeMax(attackRangeMax)
	{
	}

	void SwordStateMachineComponent::OnInitialize()
	{
		GM_ASSERT_RETURN(RegisterAnimationClips(), "Sword Animation Clip 등록에 실패했습니다.");

		HitBoxComponent* hitBox = GetOwner().GetComponent<HitBoxComponent>();
		GM_ASSERT_RETURN(hitBox, "SwordStateMachineComponent에 필요한 HitBoxComponent가 없습니다.");
		GM_ASSERT_RETURN(RegisterStates(*hitBox), "Sword State 등록에 실패했습니다.");

		SetInitialState(MonsterStateId::Idle);
		MonsterStateMachineComponent::OnInitialize();
	}

	bool SwordStateMachineComponent::RegisterAnimationClips()
	{
		SkeletalAnimatorComponent* animator = GetOwner().GetComponent<SkeletalAnimatorComponent>();
		GM_ASSERT_RETURN_VAL(animator, false, "SwordStateMachineComponent에 SkeletalAnimatorComponent가 필요합니다.");

		for (uint32 animationIndex = 0; animationIndex < SwordAnimationIdCount; ++animationIndex)
		{
			const SwordAnimationId animationId = static_cast<SwordAnimationId>(animationIndex);
			const std::wstring clipName = GetSwordAnimationClipName(animationId);
			if (animator->HasClip(clipName))
				continue;

			const std::wstring animationKey = GetMonsterAnimationClipKey(MonsterType::Sword, animationIndex);
			GM_ASSERT_RETURN_VAL(animator->AddClip(clipName, animationKey), false, "Sword Animation Clip을 찾을 수 없습니다. key=%ls", animationKey.c_str());
		}

		return true;
	}

	bool SwordStateMachineComponent::RegisterStates(HitBoxComponent& hitBox)
	{
		GM_ASSERT_RETURN_VAL(RegisterState(std::make_unique<SwordIdleState>()), false, "Sword Idle State 등록에 실패했습니다.");
		GM_ASSERT_RETURN_VAL(RegisterState(std::make_unique<SwordMoveState>(_attackRangeMin, _attackRangeMax)), false, "Sword Move State 등록에 실패했습니다.");
		GM_ASSERT_RETURN_VAL(RegisterState(std::make_unique<SwordAttackState>(hitBox)), false, "Sword Attack State 등록에 실패했습니다.");
		GM_ASSERT_RETURN_VAL(RegisterState(std::make_unique<SwordDamageState>()), false, "Sword Damage State 등록에 실패했습니다.");
		GM_ASSERT_RETURN_VAL(RegisterState(std::make_unique<MonsterAirborneState>(
			GetSwordAnimationClipName(SwordAnimationId::AirDamageStart),
			GetSwordAnimationClipName(SwordAnimationId::AirDamageFall),
			std::array<std::wstring, 3>{
				GetSwordAnimationClipName(SwordAnimationId::AirDamage0),
				GetSwordAnimationClipName(SwordAnimationId::AirDamage1),
				GetSwordAnimationClipName(SwordAnimationId::AirDamage2) })), false, "Sword Airborne State 등록에 실패했습니다.");
		GM_ASSERT_RETURN_VAL(RegisterState(std::make_unique<MonsterDownState>(GetSwordAnimationClipName(SwordAnimationId::DownFall))), false, "Sword Down State 등록에 실패했습니다.");
		GM_ASSERT_RETURN_VAL(RegisterState(std::make_unique<MonsterWakeUpState>(GetSwordAnimationClipName(SwordAnimationId::DownWakeUp))), false, "Sword WakeUp State 등록에 실패했습니다.");
		GM_ASSERT_RETURN_VAL(RegisterState(std::make_unique<SwordDeadState>()), false, "Sword Dead State 등록에 실패했습니다.");
		return true;
	}
}
