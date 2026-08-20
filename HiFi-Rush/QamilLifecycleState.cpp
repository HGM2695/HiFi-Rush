#include "QamilLifecycleState.h"

#include "BeatSystem.h"
#include "HealthComponent.h"
#include "QamilStateMachineComponent.h"
#include "SkeletalAnimatorComponent.h"

namespace gm
{
	void QamilInactiveState::Enter(QamilStateContext& context)
	{
		PlayBeatSyncedAnimation(context, QamilAnimationId::Idle, true);
	}

	void QamilIdleState::Enter(QamilStateContext& context)
	{
		PlayBeatSyncedAnimation(context, QamilAnimationId::Idle, true, 0.f);
		_nextActionBeat = context.beatSystem && context.beatSystem->HasPlaybackTime() ? std::optional<float>{ context.beatSystem->GetNextBeat() } : std::nullopt;
	}

	void QamilIdleState::Tick(QamilStateContext& context, float)
	{
		if (context.target.IsValid() == false || context.beatSystem == nullptr || context.beatSystem->HasPlaybackTime() == false)
			return;

		if (_nextActionBeat.has_value() == false)
			_nextActionBeat = context.beatSystem->GetNextBeat();
		if (context.beatSystem->GetCurrentBeat() < _nextActionBeat.value())
			return;

		_nextActionBeat.reset();
		context.stateMachine->ChangeState(IsTargetInCurrentPlatformArea(context) ? context.stateMachine->SelectNextAttackState() : QamilStateId::Move);
	}

	void QamilDeadState::Enter(QamilStateContext& context)
	{
		context.healthComponent->SetInvincible(true);
		context.animatorComponent->Pause();
	}
}
