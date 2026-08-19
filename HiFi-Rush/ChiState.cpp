#include "ChiState.h"
#include "Application.h"
#include "BeatSystem.h"
#include "ChiAnimationSettings.h"
#include "ChiMoveComponent.h"
#include "ChiStateMachineComponent.h"
#include "Input.h"
#include "ReverbComponent.h"
#include "Rigidbody3DComponent.h"
#include "SkeletalAnimatorComponent.h"

#include <algorithm>
#include <cmath>

namespace gm
{
	ChiState::ChiState(ChiStateId stateId, ChiAnimationClipId animationClipId, bool isLoop)
		: _stateId(stateId), _animationClipId(animationClipId)
	{
		_playOption.loopOverride = isLoop;
	}

	ChiState::ChiState(ChiStateId stateId, ChiAnimationClipId animationClipId, const AnimationPlayOption& playOption)
		: _stateId(stateId), _animationClipId(animationClipId), _playOption(playOption)
	{}

	void ChiState::Enter(ChiStateContext& context)
	{
		const ChiAnimationSetting& setting = context.animationSettings->Get(_animationClipId);
		const float blendDuration = context.blendDuration.value_or(setting.blendDuration);
		AnimationPlayOption playOption = _playOption;
		playOption.blendDuration = blendDuration;
		InitializeBeatTiming(context, blendDuration);
		PlayAnimation(context, _animationClipId, playOption);
	}

	void ChiState::PlayAnimation(ChiStateContext& context, ChiAnimationClipId animationClipId, const AnimationPlayOption& playOption) const
	{
		const ChiAnimationSetting& settings = context.animationSettings->Get(animationClipId);
		context.moveComponent->SetMoveEnabled(settings.lockInputMovement == false);
		context.moveComponent->SetRootMotionWeight(settings.rootMotionWeight);
		context.rigidbodyComponent->SetUseGravity(settings.useGravity);
		if (settings.useGravity == false)
			context.rigidbodyComponent->ClearVerticalVelocity();

		context.animatorComponent->Play(GetChiAnimationClipName(animationClipId), playOption);
	}

	bool ChiState::IsMoveInputPressed(const ChiStateContext& context) const
	{
		if (context.stateMachine->IsInputEnabled() == false)
			return false;

		const Input& input = APPLICATION.GetInput();
		return input.IsKeyRepeat(KeyCode::W) || input.IsKeyRepeat(KeyCode::A) || input.IsKeyRepeat(KeyCode::S) || input.IsKeyRepeat(KeyCode::D);
	}

	void ChiState::ReturnToIdleOrRun(ChiStateContext& context) const
	{
		if (IsMoveInputPressed(context))
			context.stateMachine->ChangeState(ChiStateId::Run);
		else
			context.stateMachine->ChangeState(ChiStateId::Idle);
	}

	void ChiState::OnGroundContact(ChiStateContext& context)
	{
		context.stateMachine->ChangeState(ChiStateId::JumpLanding);
	}

	void ChiState::OnGroundLost(ChiStateContext& context, const NavigationGroundLostEvent&)
	{
		context.stateMachine->ChangeState(ChiStateId::JumpDown);
	}

	bool ChiState::IsAnimationCompleted(const ChiStateContext& context) const
	{
		return context.animatorComponent->GetState() == AnimationState::Completed;
	}

	void ChiState::ChangeDashStateByInput(ChiStateContext& context, const RhythmJudgeResult* judgeResult) const
	{
		auto changeState = [&context, judgeResult](ChiStateId stateId)
			{
				if (judgeResult)
					context.stateMachine->ChangeState(stateId, *judgeResult);
				else
					context.stateMachine->ChangeState(stateId);
			};

		const Vector3 inputDirection = context.moveComponent->GetInputMoveDirection();
		if (inputDirection.LengthSquared() <= 0.f)
		{
			changeState(ChiStateId::DashFront);
			return;
		}

		const float forwardAmount = inputDirection.Dot(context.moveComponent->GetForwardDirection());
		const float rightAmount = inputDirection.Dot(context.moveComponent->GetRightDirection());
		if (std::abs(rightAmount) > std::abs(forwardAmount))
		{
			changeState(rightAmount > 0.f ? ChiStateId::DashRight : ChiStateId::DashLeft);
			return;
		}

		changeState(forwardAmount < 0.f ? ChiStateId::DashBack : ChiStateId::DashFront);
	}

	bool ChiState::TryChangeGroundAction(ChiStateContext& context) const
	{
		if (TryChangeHibiki(context))
			return true;

		if (context.weakAttackInput)
		{
			context.stateMachine->ChangeState(ChiStateId::AttackWeak0, context.weakAttackInput.value());
			return true;
		}

		if (context.strongAttackInput)
		{
			context.stateMachine->ChangeState(ChiStateId::AttackStrong0_0, context.strongAttackInput.value());
			return true;
		}

		if (context.jumpInput)
		{
			context.stateMachine->ChangeState(ChiStateId::JumpUp, context.jumpInput.value());
			return true;
		}

		if (context.dashInput)
		{
			ChangeDashStateByInput(context, &context.dashInput.value());
			return true;
		}

		return false;
	}

	bool ChiState::TryChangeAirDashOrStump(ChiStateContext& context) const
	{
		if (context.dashInput)
		{
			context.stateMachine->ChangeState(ChiStateId::DashSky, context.dashInput.value());
			return true;
		}

		if (context.strongAttackInput)
		{
			context.stateMachine->ChangeState(ChiStateId::AttackStump0, context.strongAttackInput.value());
			return true;
		}

		return false;
	}

	bool ChiState::TryChangeAirAction(ChiStateContext& context, bool canDoubleJump, std::optional<float> weakAttackStartBeat) const
	{
		if (TryChangeAirDashOrStump(context))
			return true;

		const bool canChangeWeakAttack = weakAttackStartBeat.has_value() == false || GetElapsedBeatAfterBlend(context) > weakAttackStartBeat.value();
		if (canChangeWeakAttack && context.weakAttackInput)
		{
			context.stateMachine->ChangeState(ChiStateId::AttackSky0, context.weakAttackInput.value());
			return true;
		}

		if (canDoubleJump && context.jumpInput)
		{
			context.stateMachine->ChangeState(ChiStateId::JumpDoubleUp, context.jumpInput.value());
			return true;
		}

		return false;
	}

	bool ChiState::TryChangeHibiki(ChiStateContext& context) const
	{
		if (context.stateMachine->IsInputEnabled() == false || APPLICATION.GetInput().IsKeyDown(KeyCode::R) == false)
			return false;
		if (context.reverbComponent == nullptr || context.reverbComponent->IsFull() == false)
			return false;
		if (context.reverbComponent->ConsumeReverb(context.reverbComponent->GetMaxReverb()) == false)
			return false;

		context.stateMachine->ChangeState(ChiStateId::HibikiReady);
		return true;
	}

	void ChiState::InitializeBeatTiming(ChiStateContext& context, float blendDuration)
	{
		_stateStartBeat = context.beatSystem->GetCurrentBeat();
		const float secondsPerBeat = context.beatSystem->GetSecondsPerBeat();
		const float blendDurationBeats = secondsPerBeat > 0.f ? blendDuration / secondsPerBeat : 0.f;
		_blendEndBeat = _stateStartBeat + std::max(0.f, blendDurationBeats);
	}

	float ChiState::GetStateElapsedBeat(const ChiStateContext& context) const
	{
		return context.beatSystem->GetCurrentBeat() - _stateStartBeat;
	}

	float ChiState::GetElapsedBeatAfterBlend(const ChiStateContext& context) const
	{
		return context.beatSystem->GetCurrentBeat() - _blendEndBeat;
	}

	bool ChiState::IsBlendCompleted(const ChiStateContext& context) const
	{
		return context.beatSystem->GetCurrentBeat() >= _blendEndBeat;
	}

}
