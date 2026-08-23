#include "ChiAttackState.h"
#include "AudioStatics.h"
#include "BeatSystem.h"
#include "ChiAnimationSettings.h"
#include "ChiAttackHitBoxSpawner.h"
#include "ChiBeatHitBoxSpawner.h"
#include "ChiEffectComponent.h"
#include "ChiMoveComponent.h"
#include "ChiStateMachineComponent.h"
#include "HiFiRushAnimationNotifyNames.h"
#include "HiFiRushAudio.h"
#include "HitBoxComponent.h"
#include "PlayerTargetingComponent.h"
#include "Rigidbody3DComponent.h"
#include "SkeletalAnimationClip.h"
#include "SkeletalAnimatorComponent.h"

#include <algorithm>
#include <cmath>

namespace gm
{
	namespace
	{
		constexpr float AnimationTicksPerBeat = 15.f;
		constexpr float StumpDownImpulse = 45.f;
		constexpr float OnePassAirActionBufferStartBeat = 1.f + 9.f / AnimationTicksPerBeat;
		constexpr float OnePassAirActionStartBeat = 2.f;
		constexpr int32 RightStartAttachedDamage = 5;
		constexpr int32 LeftStartAttachedDamage = 20;
		constexpr float RightStartAttachedRehitInterval = 0.15f;
		constexpr uint32 BeatHitTargetOffset = 2;

		float GetEarliestReachableTargetBeat(float currentBeat, const std::optional<RhythmJudgeResult>& rhythmInput, float animationBeatsUntilImpact, float maxPlaybackRateScale)
		{
			float targetBeat = std::floor(currentBeat) + 1.f;
			if (rhythmInput)
				targetBeat = std::max(targetBeat, static_cast<float>(rhythmInput->judgedBeatIndex + 1));

			while (animationBeatsUntilImpact / (targetBeat - currentBeat) > maxPlaybackRateScale)
				targetBeat += 1.f;

			return targetBeat;
		}
	}

	/// Attack //////////////////////////////////////////////////////////////////////////////
	ChiAttackState::ChiAttackState(ChiStateId stateId, ChiAnimationClipId animationClipId)
		: ChiState(stateId, animationClipId)
	{
	}

	void ChiAttackState::Enter(ChiStateContext& context)
	{
		_temporaryHitBoxNotifyConnection.Disconnect();
		_effectStartNotifyConnection.Disconnect();
		_beatHitStartNotifyConnection.Disconnect();
		_beatHitResultConnection.Disconnect();
		_bufferedRhythmInput.reset();
		_basePlayRate = 1.f;
		_syncPlayRate = 1.f;
		_impactMarkerBeat = 0.f;
		_hasRestoredBasePlayRate = true;

		if (UsesAutoTargeting())
		{
			context.targetingComponent->AcquireTarget(context.moveComponent->GetInputMoveDirection());
			const Vector3 targetDirection = context.targetingComponent->GetTargetDirection();
			if (targetDirection.LengthSquared() > 0.000001f)
				context.moveComponent->FaceDirectionImmediate(targetDirection);
		}

		const ChiAnimationSetting& setting = context.animationSettings->Get(GetAnimationClipId());
		const std::shared_ptr<SkeletalAnimationClip> clip = context.animatorComponent->FindClip(GetChiAnimationClipName(GetAnimationClipId()));
		GM_ASSERT_RETURN(clip, "Chi 공격 Animation Clip을 찾을 수 없습니다.");

		const float secondsPerBeat = context.beatSystem->GetSecondsPerBeat();
		const float ticksPerSecond = clip->GetTicksPerSecond();
		_animationSecondsPerBeat = ticksPerSecond > 0.f ? AnimationTicksPerBeat / ticksPerSecond : 0.f;
		if (secondsPerBeat > 0.f && _animationSecondsPerBeat > 0.f)
			_basePlayRate = _animationSecondsPerBeat / secondsPerBeat;

		const float currentBeat = context.beatSystem->GetCurrentBeat();
		const float blendDuration = context.blendDuration.value_or(setting.blendDuration);
		const float blendDurationBeats = secondsPerBeat > 0.f ? blendDuration / secondsPerBeat : 0.f;
		InitializeBeatTiming(context, blendDuration);
		const float animationBlendStartBeat = std::max(0.f, setting.playbackStartBeats - blendDurationBeats);

		AnimationPlayOption playOption{};
		playOption.startTime = animationBlendStartBeat * _animationSecondsPerBeat;
		playOption.blendDuration = blendDuration;
		playOption.loopOverride = false;
		playOption.playRateOverride = _basePlayRate;

		if (setting.impactMarkerBeat > animationBlendStartBeat && secondsPerBeat > 0.f)
		{
			const float animationBeatsUntilImpact = setting.impactMarkerBeat - animationBlendStartBeat;
			const float targetBeat = GetEarliestReachableTargetBeat(currentBeat, context.transitionRhythmInput, animationBeatsUntilImpact, setting.maxPlaybackRateScale);
			const float beatsUntilTarget = targetBeat - currentBeat;
			if (beatsUntilTarget > 0.f)
			{
				_syncPlayRate = _basePlayRate * animationBeatsUntilImpact / beatsUntilTarget;
				_impactMarkerBeat = setting.impactMarkerBeat;
				playOption.playRateOverride = _syncPlayRate;
				_hasRestoredBasePlayRate = false;
			}
		}

		PlayAnimation(context, GetAnimationClipId(), playOption);

		if (ChiAttackHitBoxSpawner::IsSpawnedByHitStartNotify(GetAnimationClipId()))
		{
			GM_ASSERT_RETURN(clip->FindNotify(HiFiRushAnimationNotifyNames::HitStart), "Chi Temporary HitBox 공격에 HitStart Notify가 없습니다.");
			context.animatorComponent->GetNotifyEvent().Subscribe(_temporaryHitBoxNotifyConnection,
				[this, &context](const AnimationNotifyEvent& event)
				{
					HandleTemporaryHitBoxNotify(context, event);
				});
		}

		if (clip->FindNotify(HiFiRushAnimationNotifyNames::EffectStart))
		{
			context.animatorComponent->GetNotifyEvent().Subscribe(_effectStartNotifyConnection,
				[this, &context](const AnimationNotifyEvent& event)
				{
					HandleEffectStartNotify(context, event);
				});
		}

		if (clip->FindNotify(HiFiRushAnimationNotifyNames::BeatHitStart))
		{
			context.animatorComponent->GetNotifyEvent().Subscribe(_beatHitStartNotifyConnection,
				[this, &context](const AnimationNotifyEvent& event)
				{
					HandleBeatHitStartNotify(context, event);
				});

			context.stateMachine->OnBeatHitResult.Subscribe(_beatHitResultConnection,
				[this, &context](const BeatHitResultEvent& event)
				{
					HandleBeatHitResult(context, event);
				});
		}
	}

	void ChiAttackState::Exit(ChiStateContext& context)
	{
		_temporaryHitBoxNotifyConnection.Disconnect();
		_effectStartNotifyConnection.Disconnect();
		_beatHitStartNotifyConnection.Disconnect();
		_beatHitResultConnection.Disconnect();
		context.animatorComponent->SetPlayRate(1.f);
	}

	void ChiAttackState::HandleTemporaryHitBoxNotify(ChiStateContext& context, const AnimationNotifyEvent& event)
	{
		if (event.name != HiFiRushAnimationNotifyNames::HitStart)
			return;

		GM_ASSERT(ChiAttackHitBoxSpawner::SpawnForAnimation(context, GetAnimationClipId()), "Chi Attack HitBox GameObject 생성에 실패했습니다.");
	}

	void ChiAttackState::HandleEffectStartNotify(ChiStateContext& context, const AnimationNotifyEvent& event)
	{
		if (event.name != HiFiRushAnimationNotifyNames::EffectStart)
			return;

		context.effectComponent->SpawnAttackEffect(GetAnimationClipId());
	}

	void ChiAttackState::HandleBeatHitStartNotify(ChiStateContext& context, const AnimationNotifyEvent& event)
	{
		if (event.name != HiFiRushAnimationNotifyNames::BeatHitStart)
			return;

		context.stateMachine->BeginBeatHit(BeatHitInputType::AnyAttack, BeatHitTargetOffset);
	}

	void ChiAttackState::HandleBeatHitResult(ChiStateContext& context, const BeatHitResultEvent& event)
	{
		GM_ASSERT(ChiBeatHitBoxSpawner::Spawn(context, GetAnimationClipId(), event), "Chi Beat HitBox GameObject 생성에 실패했습니다.");
	}

	void ChiAttackState::RestoreBasePlayRateAfterImpact(ChiStateContext& context)
	{
		if (_hasRestoredBasePlayRate || GetAnimationBeat(context) < _impactMarkerBeat)
			return;

		context.animatorComponent->SetPlayRate(_basePlayRate);
		_hasRestoredBasePlayRate = true;
	}

	void ChiAttackState::BufferMouseInput(ChiStateContext& context, float bufferStartBeat, bool allowWeak, bool allowStrong)
	{
		if (_bufferedRhythmInput || GetStateElapsedBeat(context) <= bufferStartBeat)
			return;

		if (allowWeak && context.weakAttackInput)
			_bufferedRhythmInput = context.weakAttackInput;
		else if (allowStrong && context.strongAttackInput)
			_bufferedRhythmInput = context.strongAttackInput;
	}

	bool ChiAttackState::IsActionCancelAllowed(const ChiStateContext& context) const
	{
		const ChiAnimationSetting& setting = context.animationSettings->Get(GetAnimationClipId());
		return GetAnimationBeat(context) >= setting.actionCancelStartBeats;
	}

	bool ChiAttackState::TryCancelAttack(ChiStateContext& context)
	{
		if (IsActionCancelAllowed(context) == false)
			return false;

		if (IsMoveInputPressed(context))
		{
			context.stateMachine->ChangeState(ChiStateId::Run);
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

	void ChiAttackState::ChangeStateWithBufferedInput(ChiStateContext& context, ChiStateId nextStateId) const
	{
		if (_bufferedRhythmInput)
			context.stateMachine->ChangeState(nextStateId, _bufferedRhythmInput.value());
		else
			context.stateMachine->ChangeState(nextStateId);
	}

	float ChiAttackState::GetAnimationBeat(const ChiStateContext& context) const
	{
		if (_animationSecondsPerBeat <= 0.f)
			return 0.f;
		return context.animatorComponent->GetPlayTime() / _animationSecondsPerBeat;
	}

	ChiWeaponHitBoxAttackState::ChiWeaponHitBoxAttackState(ChiStateId stateId, ChiAnimationClipId animationClipId, HitBoxComponent* weaponHitBox, int32 damage, float rehitInterval)
		: ChiAttackState(stateId, animationClipId), _weaponHitBox(weaponHitBox), _damage(damage), _rehitInterval(rehitInterval)
	{
	}

	void ChiWeaponHitBoxAttackState::Enter(ChiStateContext& context)
	{
		_notifyConnection.Disconnect();
		if (_weaponHitBox)
		{
			_weaponHitBox->EndAttack();
			_weaponHitBox->SetDamage(_damage);
			_weaponHitBox->SetHitReactionType(HitReactionType::WeakKnockback);
			_weaponHitBox->SetWorldKnockbackDirection(context.moveComponent->GetForwardDirection());
			_weaponHitBox->SetRehitInterval(_rehitInterval);
		}

		ChiAttackState::Enter(context);
		if (_weaponHitBox == nullptr)
			return;

		const std::shared_ptr<SkeletalAnimationClip> clip = context.animatorComponent->GetCurrentClip();
		GM_ASSERT_RETURN(clip, "Chi Weapon HitBox 공격 Animation Clip이 없습니다.");
		GM_ASSERT_RETURN(clip->FindNotify(HiFiRushAnimationNotifyNames::HitStart), "Chi Weapon HitBox 공격에 HitStart Notify가 없습니다.");
		GM_ASSERT_RETURN(clip->FindNotify(HiFiRushAnimationNotifyNames::HitEnd), "Chi Weapon HitBox 공격에 HitEnd Notify가 없습니다.");

		context.animatorComponent->GetNotifyEvent().Subscribe(_notifyConnection,
			[this](const AnimationNotifyEvent& event)
			{
				HandleAnimationNotify(event);
			});
	}

	void ChiWeaponHitBoxAttackState::Exit(ChiStateContext& context)
	{
		_notifyConnection.Disconnect();
		if (_weaponHitBox)
			_weaponHitBox->EndAttack();
		ChiAttackState::Exit(context);
	}

	void ChiWeaponHitBoxAttackState::HandleAnimationNotify(const AnimationNotifyEvent& event)
	{
		if (event.name == HiFiRushAnimationNotifyNames::HitStart)
		{
			_weaponHitBox->BeginAttack();
			if (GetAnimationClipId() == ChiAnimationClipId::AttackStrongToWeak2)
				PlaySound2D(HiFiRushSound::ChiRightBranch);
			else if (GetAnimationClipId() == ChiAnimationClipId::AttackWeakToStrong2)
				PlaySound2D(HiFiRushSound::ChiLeftBranch);
		}
		else if (event.name == HiFiRushAnimationNotifyNames::HitEnd)
			_weaponHitBox->EndAttack();
	}

	/// WeakAttack //////////////////////////////////////////////////////////////////////////////
	ChiWeakAttackState::ChiWeakAttackState(ChiStateId stateId, ChiAnimationClipId animationClipId, ChiStateId nextWeakAttackState, ChiStateId nextStrongAttackState)
		: ChiAttackState(stateId, animationClipId), _nextWeakAttackState(nextWeakAttackState), _nextStrongAttackState(nextStrongAttackState)
	{
	}

	void ChiWeakAttackState::Tick(ChiStateContext& context, float)
	{
		RestoreBasePlayRateAfterImpact(context);
		if (GetStateId() == ChiStateId::AttackWeak3)
		{
			if (TryCancelAttack(context))
				return;

			if (IsAnimationCompleted(context))
				ReturnToIdleOrRun(context);

			return;
		}

		BufferMouseInput(context, 0.2f, _nextWeakAttackState != ChiStateId::None, _nextStrongAttackState != ChiStateId::None);
		if (TryCancelAttack(context))
			return;

		if (GetAnimationBeat(context) >= 1.f)
		{
			if (HasBufferedWeakInput() && _nextWeakAttackState != ChiStateId::None)
			{
				const ChiStateId nextState = GetStateId() == ChiStateId::AttackWeak0 && GetStateElapsedBeat(context) >= 1.3f ? ChiStateId::AttackDelayedWeak1 : _nextWeakAttackState;
				ChangeStateWithBufferedInput(context, nextState);
				return;
			}

			if (HasBufferedStrongInput() && _nextStrongAttackState != ChiStateId::None)
			{
				ChangeStateWithBufferedInput(context, _nextStrongAttackState);
				return;
			}
		}

		if (IsAnimationCompleted(context))
			ReturnToIdleOrRun(context);
	}

	ChiWeak0AttackState::ChiWeak0AttackState()
		: ChiWeakAttackState(ChiStateId::AttackWeak0, ChiAnimationClipId::AttackWeak0, ChiStateId::AttackWeak1, ChiStateId::AttackWeakToStrong1)
	{
	}

	ChiWeak1AttackState::ChiWeak1AttackState()
		: ChiWeakAttackState(ChiStateId::AttackWeak1, ChiAnimationClipId::AttackWeak1, ChiStateId::AttackWeak2)
	{
	}

	ChiWeak2AttackState::ChiWeak2AttackState()
		: ChiWeakAttackState(ChiStateId::AttackWeak2, ChiAnimationClipId::AttackWeak2, ChiStateId::AttackWeak3)
	{
	}

	ChiWeak3AttackState::ChiWeak3AttackState()
		: ChiWeakAttackState(ChiStateId::AttackWeak3, ChiAnimationClipId::AttackWeak3)
	{
	}

	ChiWeakDashAttackState::ChiWeakDashAttackState()
		: ChiAttackState(ChiStateId::AttackWeakDash, ChiAnimationClipId::AttackWeakDash)
	{
	}

	void ChiWeakDashAttackState::Tick(ChiStateContext& context, float)
	{
		RestoreBasePlayRateAfterImpact(context);
		if (GetAnimationBeat(context) >= 1.8f)
			BufferMouseInput(context, -1.f);
		if (TryCancelAttack(context))
			return;

		if (GetAnimationBeat(context) >= 1.8f)
		{
			if (HasBufferedWeakInput())
			{
				ChangeStateWithBufferedInput(context, ChiStateId::AttackDelayedWeak1);
				return;
			}
			if (HasBufferedStrongInput())
			{
				ChangeStateWithBufferedInput(context, ChiStateId::AttackStrong0_0);
				return;
			}
		}

		if (IsAnimationCompleted(context))
			ReturnToIdleOrRun(context);
	}

	/// StrongAttack //////////////////////////////////////////////////////////////////////////////
	ChiStrongAttackState::ChiStrongAttackState(ChiStateId stateId, ChiAnimationClipId animationClipId, ChiStateId nextWeakAttackState, ChiStateId nextStrongAttackState, ChiStateId autoNextState)
		: ChiAttackState(stateId, animationClipId)
		, _nextWeakAttackState(nextWeakAttackState)
		, _nextStrongAttackState(nextStrongAttackState)
		, _autoNextState(autoNextState)
	{
	}

	void ChiStrongAttackState::Tick(ChiStateContext& context, float)
	{
		RestoreBasePlayRateAfterImpact(context);
		if (GetStateId() == ChiStateId::AttackStrong0_0)
		{
			if (context.jumpInput)
			{
				context.stateMachine->ChangeState(ChiStateId::JumpUp, context.jumpInput.value());
				return;
			}
			if (context.dashInput)
			{
				ChangeDashStateByInput(context, &context.dashInput.value());
				return;
			}
		}
		else if (GetStateId() == ChiStateId::AttackStrong0_1)
		{
			BufferMouseInput(context, 0.2f, _nextWeakAttackState != ChiStateId::None, _nextStrongAttackState != ChiStateId::None);
			if (TryCancelAttack(context))
				return;
			if (GetStateElapsedBeat(context) >= 1.f)
			{
				if (HasBufferedStrongInput() && _nextStrongAttackState != ChiStateId::None)
				{
					ChangeStateWithBufferedInput(context, _nextStrongAttackState);
					return;
				}
				if (HasBufferedWeakInput() && _nextWeakAttackState != ChiStateId::None)
				{
					ChangeStateWithBufferedInput(context, _nextWeakAttackState);
					return;
				}
			}
		}
		else if (GetStateId() == ChiStateId::AttackStrong1)
		{
			BufferMouseInput(context, 1.4f, false, _nextStrongAttackState != ChiStateId::None);
			if (TryCancelAttack(context))
				return;
			if (GetStateElapsedBeat(context) >= 2.f && HasBufferedStrongInput() && _nextStrongAttackState != ChiStateId::None)
			{
				ChangeStateWithBufferedInput(context, _nextStrongAttackState);
				return;
			}
		}
		else if (TryCancelAttack(context))
		{
			return;
		}

		if (IsAnimationCompleted(context))
		{
			if (_autoNextState != ChiStateId::None)
				context.stateMachine->ChangeState(_autoNextState);
			else
				ReturnToIdleOrRun(context);
		}
	}

	ChiStrong0_0AttackState::ChiStrong0_0AttackState()
		: ChiStrongAttackState(ChiStateId::AttackStrong0_0, ChiAnimationClipId::AttackStrong0_0, ChiStateId::None, ChiStateId::None, ChiStateId::AttackStrong0_1)
	{
	}

	ChiStrong0_1AttackState::ChiStrong0_1AttackState()
		: ChiStrongAttackState(ChiStateId::AttackStrong0_1, ChiAnimationClipId::AttackStrong0_1, ChiStateId::AttackStrongToWeak1, ChiStateId::AttackStrong1)
	{
	}

	ChiStrong1AttackState::ChiStrong1AttackState()
		: ChiStrongAttackState(ChiStateId::AttackStrong1, ChiAnimationClipId::AttackStrong1, ChiStateId::None, ChiStateId::AttackStrong2)
	{
	}

	ChiStrong2AttackState::ChiStrong2AttackState()
		: ChiStrongAttackState(ChiStateId::AttackStrong2, ChiAnimationClipId::AttackStrong2)
	{
	}

	ChiStrongDashAttackState::ChiStrongDashAttackState()
		: ChiAttackState(ChiStateId::AttackStrongDash, ChiAnimationClipId::AttackStrongDash)
	{
	}

	void ChiStrongDashAttackState::Enter(ChiStateContext& context)
	{
		ChiAttackState::Enter(context);
		_fallStartRequested = false;

		const std::shared_ptr<SkeletalAnimationClip> clip = context.animatorComponent->FindClip(GetChiAnimationClipName(GetAnimationClipId()));
		GM_ASSERT_RETURN(clip && clip->FindNotify(HiFiRushAnimationNotifyNames::FallStart), "Chi Strong Dash 공격에 FallStart Notify가 없습니다.");
		context.animatorComponent->GetNotifyEvent().Subscribe(_fallStartNotifyConnection,
			[this](const AnimationNotifyEvent& event)
			{
				if (event.name == HiFiRushAnimationNotifyNames::FallStart)
					_fallStartRequested = true;
			});
	}

	void ChiStrongDashAttackState::Tick(ChiStateContext& context, float)
	{
		RestoreBasePlayRateAfterImpact(context);
		if (GetAnimationBeat(context) >= 1.4f && TryChangeAirAction(context, false))
			return;

		if (_fallStartRequested)
		{
			context.stateMachine->ChangeState(ChiStateId::JumpDown);
			return;
		}

		if (TryCancelAttack(context))
			return;
	}

	void ChiStrongDashAttackState::Exit(ChiStateContext& context)
	{
		_fallStartNotifyConnection.Disconnect();
		_fallStartRequested = false;
		ChiAttackState::Exit(context);
	}

	/// BranchAttack //////////////////////////////////////////////////////////////////////////////
	ChiBranchAttackState::ChiBranchAttackState(ChiStateId stateId, ChiAnimationClipId animationClipId, ChiStateId nextAttackState)
		: ChiAttackState(stateId, animationClipId)
		, _nextAttackState(nextAttackState)
	{
	}

	void ChiBranchAttackState::Tick(ChiStateContext& context, float)
	{
		RestoreBasePlayRateAfterImpact(context);
		float comboTransitionBeat = 1.f;
		if (GetStateId() == ChiStateId::AttackWeakToStrong1)
		{
			if (GetAnimationBeat(context) >= 1.f)
				BufferMouseInput(context, -1.f);

			comboTransitionBeat = 2.f;
		}
		else if (GetStateId() == ChiStateId::AttackDelayedWeak1)
		{
			BufferMouseInput(context, 0.2f, true, false);
		}
		else
		{
			BufferMouseInput(context, 0.2f);
		}

		if (TryCancelAttack(context))
			return;

		if (GetAnimationBeat(context) >= comboTransitionBeat && _nextAttackState != ChiStateId::None && (HasBufferedWeakInput() || HasBufferedStrongInput()))
		{
			ChangeStateWithBufferedInput(context, _nextAttackState);
			return;
		}

		if (IsAnimationCompleted(context))
			ReturnToIdleOrRun(context);
	}

	ChiStrongToWeak1AttackState::ChiStrongToWeak1AttackState()
		: ChiBranchAttackState(ChiStateId::AttackStrongToWeak1, ChiAnimationClipId::AttackStrongToWeak1, ChiStateId::AttackStrongToWeak2)
	{
	}

	ChiStrongToWeak2AttackState::ChiStrongToWeak2AttackState(HitBoxComponent* weaponHitBox)
		: ChiWeaponHitBoxAttackState(ChiStateId::AttackStrongToWeak2, ChiAnimationClipId::AttackStrongToWeak2, weaponHitBox, RightStartAttachedDamage, RightStartAttachedRehitInterval)
	{
	}

	void ChiStrongToWeak2AttackState::Tick(ChiStateContext& context, float)
	{
		RestoreBasePlayRateAfterImpact(context);
		if (context.beatHitInput)
		{
			context.stateMachine->ChangeState(ChiStateId::AttackStrongToWeakBeatHit, context.beatHitInput.value());
			return;
		}

		if (TryCancelAttack(context))
			return;

		if (IsAnimationCompleted(context))
			ReturnToIdleOrRun(context);
	}

	ChiStrongToWeakBeatHitAttackState::ChiStrongToWeakBeatHitAttackState()
		: ChiAttackState(ChiStateId::AttackStrongToWeakBeatHit, ChiAnimationClipId::RightStartBitHeat)
	{
	}

	void ChiStrongToWeakBeatHitAttackState::Tick(ChiStateContext& context, float)
	{
		if (TryCancelAttack(context))
			return;
		if (IsAnimationCompleted(context))
			ReturnToIdleOrRun(context);
	}

	ChiWeakToStrong1AttackState::ChiWeakToStrong1AttackState()
		: ChiBranchAttackState(ChiStateId::AttackWeakToStrong1, ChiAnimationClipId::AttackWeakToStrong1, ChiStateId::AttackWeakToStrong2)
	{
	}

	ChiWeakToStrong2AttackState::ChiWeakToStrong2AttackState(HitBoxComponent* weaponHitBox)
		: ChiWeaponHitBoxAttackState(ChiStateId::AttackWeakToStrong2, ChiAnimationClipId::AttackWeakToStrong2, weaponHitBox, LeftStartAttachedDamage)
	{
	}

	void ChiWeakToStrong2AttackState::Tick(ChiStateContext& context, float)
	{
		RestoreBasePlayRateAfterImpact(context);
		if (IsAnimationCompleted(context))
			ReturnToIdleOrRun(context);
	}

	ChiDelayedWeak1AttackState::ChiDelayedWeak1AttackState()
		: ChiBranchAttackState(ChiStateId::AttackDelayedWeak1, ChiAnimationClipId::AttackDelayedWeak1, ChiStateId::AttackDelayedWeak2)
	{
	}

	ChiDelayedWeak2AttackState::ChiDelayedWeak2AttackState()
		: ChiAttackState(ChiStateId::AttackDelayedWeak2, ChiAnimationClipId::AttackDelayedWeak2)
	{
	}

	void ChiDelayedWeak2AttackState::Enter(ChiStateContext& context)
	{
		ChiAttackState::Enter(context);
		_jumpDownTransitionRequested = false;

		const std::shared_ptr<SkeletalAnimationClip> clip = context.animatorComponent->FindClip(GetChiAnimationClipName(GetAnimationClipId()));
		GM_ASSERT_RETURN(clip && clip->FindNotify(HiFiRushAnimationNotifyNames::JumpDownTransition), "Chi One Pass 공격에 JumpDownTransition Notify가 없습니다.");
		context.animatorComponent->GetNotifyEvent().Subscribe(_jumpDownTransitionNotifyConnection,
			[this](const AnimationNotifyEvent& event)
			{
				if (event.name == HiFiRushAnimationNotifyNames::JumpDownTransition)
					_jumpDownTransitionRequested = true;
			});
	}

	void ChiDelayedWeak2AttackState::Tick(ChiStateContext& context, float)
	{
		RestoreBasePlayRateAfterImpact(context);
		const float animationBeat = GetAnimationBeat(context);
		if (animationBeat >= OnePassAirActionBufferStartBeat)
			BufferMouseInput(context, -1.f);

		if (animationBeat >= OnePassAirActionStartBeat)
		{
			if (HasBufferedWeakInput())
			{
				ChangeStateWithBufferedInput(context, ChiStateId::AttackSky0);
				return;
			}
			if (HasBufferedStrongInput())
			{
				ChangeStateWithBufferedInput(context, ChiStateId::AttackStump0);
				return;
			}
			if (TryChangeAirDashOrStump(context))
				return;
		}

		if (_jumpDownTransitionRequested)
		{
			context.stateMachine->ChangeState(ChiStateId::JumpDown);
			return;
		}
		if (TryCancelAttack(context))
			return;
	}

	void ChiDelayedWeak2AttackState::Exit(ChiStateContext& context)
	{
		_jumpDownTransitionNotifyConnection.Disconnect();
		_jumpDownTransitionRequested = false;
		ChiAttackState::Exit(context);
	}

	/// AirAttack //////////////////////////////////////////////////////////////////////////////
	ChiAirAttackState::ChiAirAttackState(ChiStateId stateId, ChiAnimationClipId animationClipId, ChiStateId nextAirAttackState)
		: ChiAttackState(stateId, animationClipId)
		, _nextAirAttackState(nextAirAttackState)
	{
	}

	void ChiAirAttackState::Tick(ChiStateContext& context, float)
	{
		RestoreBasePlayRateAfterImpact(context);
		if (IsActionCancelAllowed(context) && TryChangeAirDashOrStump(context))
			return;

		if (_nextAirAttackState != ChiStateId::None)
		{
			BufferMouseInput(context, 0.2f, true, false);
			if (GetAnimationBeat(context) >= 1.f && HasBufferedWeakInput())
			{
				ChangeStateWithBufferedInput(context, _nextAirAttackState);
				return;
			}
		}

		if (IsAnimationCompleted(context))
			context.stateMachine->ChangeState(ChiStateId::JumpDoubleDown);
	}

	ChiSky0AttackState::ChiSky0AttackState()
		: ChiAirAttackState(ChiStateId::AttackSky0, ChiAnimationClipId::AttackSky0, ChiStateId::AttackSky1)
	{
	}

	ChiSky1AttackState::ChiSky1AttackState()
		: ChiAirAttackState(ChiStateId::AttackSky1, ChiAnimationClipId::AttackSky1, ChiStateId::AttackSky2)
	{
	}

	ChiSky2AttackState::ChiSky2AttackState()
		: ChiAirAttackState(ChiStateId::AttackSky2, ChiAnimationClipId::AttackSky2, ChiStateId::AttackSky3)
	{
	}

	ChiSky3AttackState::ChiSky3AttackState()
		: ChiAirAttackState(ChiStateId::AttackSky3, ChiAnimationClipId::AttackSky3)
	{
	}

	/// StumpAttack //////////////////////////////////////////////////////////////////////////////
	ChiStump0AttackState::ChiStump0AttackState()
		: ChiAttackState(ChiStateId::AttackStump0, ChiAnimationClipId::AttackStump0)
	{
	}

	void ChiStump0AttackState::Tick(ChiStateContext& context, float)
	{
		if (IsAnimationCompleted(context))
			context.stateMachine->ChangeState(ChiStateId::AttackStump1);
	}

	ChiStump1AttackState::ChiStump1AttackState()
		: ChiAttackState(ChiStateId::AttackStump1, ChiAnimationClipId::AttackStump1)
	{
	}

	void ChiStump1AttackState::Enter(ChiStateContext& context)
	{
		ChiAttackState::Enter(context);
		context.rigidbodyComponent->ClearVerticalVelocity();
		context.rigidbodyComponent->AddImpulse(Vector3{ 0.f, -StumpDownImpulse, 0.f });
	}

	void ChiStump1AttackState::Tick(ChiStateContext&, float)
	{
	}

	void ChiStump1AttackState::OnGroundContact(ChiStateContext& context)
	{
		GM_ASSERT(ChiAttackHitBoxSpawner::SpawnForAnimation(context, GetAnimationClipId()), "Chi Stump HitBox GameObject 생성에 실패했습니다.");
		context.effectComponent->SpawnStumpEffect();
		PlayRandomSound2D(HiFiRushSound::ChiStumps);
		context.stateMachine->ChangeState(ChiStateId::AttackStump2);
	}

	ChiStump2AttackState::ChiStump2AttackState()
		: ChiAttackState(ChiStateId::AttackStump2, ChiAnimationClipId::AttackStump2)
	{
	}

	void ChiStump2AttackState::Tick(ChiStateContext& context, float)
	{
		if (TryCancelAttack(context))
			return;
		if (IsAnimationCompleted(context))
			ReturnToIdleOrRun(context);
	}
}
