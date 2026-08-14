#include "MonsterState.h"

#include "AnimationTypes.h"
#include "CombatTypes.h"
#include "MonsterStateMachineComponent.h"
#include "SkeletalAnimatorComponent.h"

namespace gm
{
	void MonsterState::OnDamaged(MonsterStateContext& context, const HitEvent& event)
	{
		if (event.damageResult.state != DamageState::Applied)
			return;

		const MonsterStateId nextStateId = event.damageResult.isDead ? MonsterStateId::Dead : MonsterStateId::Damage;
		if (context.stateMachine->HasState(nextStateId))
			context.stateMachine->ChangeState(nextStateId, true);
	}

	bool MonsterState::PlayAnimation(MonsterStateContext& context, const std::wstring& clipName, bool isLoop, float blendDuration) const
	{
		GM_ASSERT_RETURN_VAL(context.animatorComponent, false, "Monster State에 SkeletalAnimatorComponent가 연결되지 않았습니다.");

		AnimationPlayOption playOption{};
		playOption.loopOverride = isLoop;
		playOption.blendDuration = blendDuration;
		return context.animatorComponent->Play(clipName, playOption);
	}

	bool MonsterState::IsAnimationCompleted(const MonsterStateContext& context) const
	{
		return context.animatorComponent != nullptr && context.animatorComponent->GetState() == AnimationState::Completed;
	}
}
