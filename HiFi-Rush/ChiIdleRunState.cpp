#include "ChiIdleRunState.h"
#include "AudioStatics.h"
#include "BeatSystem.h"
#include "ChiEffectComponent.h"
#include "ChiMoveComponent.h"
#include "ChiStateMachineComponent.h"
#include "HiFiRushAnimationNotifyNames.h"
#include "HiFiRushAudio.h"
#include "SkeletalAnimationClip.h"
#include "SkeletalAnimatorComponent.h"

namespace gm
{
	/// Idle //////////////////////////////////////////////////////////////////////////////
	ChiIdleState::ChiIdleState()
		: ChiState(ChiStateId::Idle, ChiAnimationClipId::Idle, true)
	{
	}

	void ChiIdleState::Enter(ChiStateContext& context)
	{
		_fingerSnapNotifyConnection.Disconnect();
		ChiState::Enter(context);
		const std::shared_ptr<SkeletalAnimationClip> clip = context.animatorComponent->FindClip(GetChiAnimationClipName(GetAnimationClipId()));
		GM_ASSERT_RETURN(clip && clip->FindNotify(HiFiRushAnimationNotifyNames::FingerSnap), "Chi Idle Animation에 FingerSnap Notify가 없습니다.");
		context.animatorComponent->GetNotifyEvent().Subscribe(_fingerSnapNotifyConnection,
			[this, &context](const AnimationNotifyEvent& event)
			{
				HandleAnimationNotify(context, event);
			});
	}

	void ChiIdleState::Tick(ChiStateContext& context, float deltaTime)
	{
		if (TryChangeGroundAction(context))
			return;

		if (IsMoveInputPressed(context))
			context.stateMachine->ChangeState(ChiStateId::Run);
	}

	void ChiIdleState::Exit(ChiStateContext&)
	{
		_fingerSnapNotifyConnection.Disconnect();
	}

	void ChiIdleState::HandleAnimationNotify(ChiStateContext& context, const AnimationNotifyEvent& event)
	{
		if (event.name != HiFiRushAnimationNotifyNames::FingerSnap)
			return;

		context.effectComponent->RequestFingerSnapEffect();
	}

	/// Run //////////////////////////////////////////////////////////////////////////////
	ChiRunState::ChiRunState()
		: ChiState(ChiStateId::Run, ChiAnimationClipId::RunFront, true)
	{
	}

	void ChiRunState::Enter(ChiStateContext& context)
	{
		ChiState::Enter(context);
		_lastFootstepBeatIndex = context.beatSystem->GetCurrentBeatIndex();
	}

	void ChiRunState::Tick(ChiStateContext& context, float deltaTime)
	{
		if (TryChangeGroundAction(context))
			return;

		if (context.moveComponent->IsMoving() == false)
		{
			context.stateMachine->ChangeState(ChiStateId::Idle);
			return;
		}

		const int64 currentBeatIndex = context.beatSystem->GetCurrentBeatIndex();
		if (currentBeatIndex != _lastFootstepBeatIndex)
		{
			_lastFootstepBeatIndex = currentBeatIndex;
			PlaySound2D(HiFiRushSound::ChiFootsteps[_footstepIndex], 0.3f);
			_footstepIndex = (_footstepIndex + 1) % HiFiRushSound::ChiFootsteps.size();
		}
	}
}
