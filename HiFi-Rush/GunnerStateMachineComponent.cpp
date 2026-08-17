#include "GunnerStateMachineComponent.h"

#include "GameObject.h"
#include "GunnerAnimationTypes.h"
#include "GunnerState.h"
#include "MonsterResourceInfo.h"
#include "MonsterTypes.h"
#include "SkeletalAnimatorComponent.h"

namespace gm
{
	GunnerStateMachineComponent::GunnerStateMachineComponent(float attackRangeMin, float attackRangeMax, int32 attackDamage)
		: _attackRangeMin(attackRangeMin), _attackRangeMax(attackRangeMax), _attackDamage(attackDamage)
	{
		GM_ASSERT(attackRangeMin >= 0.f, "Gunner Attack Range Min은 0 이상이어야 합니다.");
		GM_ASSERT(attackRangeMax >= attackRangeMin, "Gunner Attack Range Max는 Min 이상이어야 합니다.");
		GM_ASSERT(attackDamage > 0, "Gunner Attack Damage는 0보다 커야 합니다.");
	}

	void GunnerStateMachineComponent::OnInitialize()
	{
		GM_ASSERT_RETURN(RegisterAnimationClips(), "Gunner Animation Clip 등록에 실패했습니다.");
		GM_ASSERT_RETURN(RegisterStates(), "Gunner State 등록에 실패했습니다.");
		SetInitialState(MonsterStateId::Idle);
		MonsterStateMachineComponent::OnInitialize();
	}

	bool GunnerStateMachineComponent::RegisterAnimationClips()
	{
		SkeletalAnimatorComponent* animator = GetOwner().GetComponent<SkeletalAnimatorComponent>();
		GM_ASSERT_RETURN_VAL(animator, false, "GunnerStateMachineComponent에 SkeletalAnimatorComponent가 필요합니다.");

		for (uint32 animationIndex = 0; animationIndex < GunnerAnimationIdCount; ++animationIndex)
		{
			const GunnerAnimationId animationId = static_cast<GunnerAnimationId>(animationIndex);
			const std::wstring clipName = GetGunnerAnimationClipName(animationId);
			if (animator->HasClip(clipName))
				continue;

			const std::wstring animationKey = GetMonsterAnimationClipKey(MonsterType::Gunner, animationIndex);
			GM_ASSERT_RETURN_VAL(animator->AddClip(clipName, animationKey), false, "Gunner Animation Clip을 찾을 수 없습니다. key=%ls", animationKey.c_str());
		}

		return true;
	}

	bool GunnerStateMachineComponent::RegisterStates()
	{
		GM_ASSERT_RETURN_VAL(RegisterState(std::make_unique<GunnerIdleState>()), false, "Gunner Idle State 등록에 실패했습니다.");
		GM_ASSERT_RETURN_VAL(RegisterState(std::make_unique<GunnerMoveState>(_attackRangeMin, _attackRangeMax)), false, "Gunner Move State 등록에 실패했습니다.");
		GM_ASSERT_RETURN_VAL(RegisterState(std::make_unique<GunnerAttackState>(_attackDamage)), false, "Gunner Attack State 등록에 실패했습니다.");
		GM_ASSERT_RETURN_VAL(RegisterState(std::make_unique<GunnerDamageState>()), false, "Gunner Damage State 등록에 실패했습니다.");
		GM_ASSERT_RETURN_VAL(RegisterState(std::make_unique<MonsterAirborneState>(
			GetGunnerAnimationClipName(GunnerAnimationId::AirDamageBasic),
			GetGunnerAnimationClipName(GunnerAnimationId::AirDamageFall),
			std::array<std::wstring, 3>{
				GetGunnerAnimationClipName(GunnerAnimationId::AirDamage0),
				GetGunnerAnimationClipName(GunnerAnimationId::AirDamage1),
				GetGunnerAnimationClipName(GunnerAnimationId::AirDamage2) })), false, "Gunner Airborne State 등록에 실패했습니다.");
		GM_ASSERT_RETURN_VAL(RegisterState(std::make_unique<MonsterDownState>(GetGunnerAnimationClipName(GunnerAnimationId::DownFall))), false, "Gunner Down State 등록에 실패했습니다.");
		GM_ASSERT_RETURN_VAL(RegisterState(std::make_unique<MonsterWakeUpState>(GetGunnerAnimationClipName(GunnerAnimationId::DownWakeUp))), false, "Gunner WakeUp State 등록에 실패했습니다.");
		GM_ASSERT_RETURN_VAL(RegisterState(std::make_unique<GunnerDeadState>()), false, "Gunner Dead State 등록에 실패했습니다.");
		return true;
	}
}
