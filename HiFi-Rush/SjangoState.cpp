#include "SjangoState.h"

#include "CombatTypes.h"
#include "HiFiRushAudio.h"
#include "MonsterStateMachineComponent.h"
#include "SjangoAnimationTypes.h"

namespace gm
{
	void SjangoState::OnDamaged(MonsterStateContext& context, const HitEvent& event)
	{
		const DamageState damageState = event.damageResult.state;
		if (damageState != DamageState::Applied && damageState != DamageState::Blocked)
			return;

		if (damageState == DamageState::Blocked)
			PlayRandomSound2D(HiFiRushSound::MonsterHitImpacts);

		context.stateMachine->ChangeState(MonsterStateId::Damage, true);
	}

	// Idle /////////////////////////////////////////////////////////////////////////
	void SjangoIdleState::Enter(MonsterStateContext& context)
	{
		PlayAnimation(context, GetSjangoAnimationClipName(SjangoAnimationId::Idle), true);
	}

	// Damage /////////////////////////////////////////////////////////////////////////
	void SjangoDamageState::Enter(MonsterStateContext& context)
	{
		PlayAnimation(context, GetSjangoAnimationClipName(SjangoAnimationId::Damage), false);
	}

	void SjangoDamageState::Tick(MonsterStateContext& context, float)
	{
		if (IsAnimationCompleted(context))
			context.stateMachine->ChangeState(MonsterStateId::Idle);
	}
}
