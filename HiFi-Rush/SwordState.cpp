#include "SwordState.h"

#include "AudioStatics.h"
#include "BeatMath.h"
#include "BeatSystem.h"
#include "CharacterMovementComponent.h"
#include "GameObject.h"
#include "HitBoxComponent.h"
#include "HiFiRushAnimationNotifyNames.h"
#include "HiFiRushAudio.h"
#include "MonsterCombatComponent.h"
#include "MonsterStateMachineComponent.h"
#include "Random.h"
#include "SkeletalAnimationClip.h"
#include "SkeletalAnimatorComponent.h"
#include "SwordAnimationTypes.h"
#include "SwordEffectComponent.h"

namespace gm
{
	namespace
	{
		constexpr float SwordDashBackDistance = 2.1f;
		constexpr float SwordDashFrontDistance = 6.f;
	}

	// SwordIdleState /////////////////////////////////////////////////////////////////////////
	void SwordIdleState::Enter(MonsterStateContext& context)
	{
		SetRootMotionEnabled(context, false);
		PlayAnimation(context, GetSwordAnimationClipName(SwordAnimationId::Idle), true);
	}

	void SwordIdleState::Tick(MonsterStateContext& context, float)
	{
		if (context.combatComponent != nullptr && context.combatComponent->HasTarget())
			context.stateMachine->ChangeState(MonsterStateId::Move);
	}

	// SwordMoveState /////////////////////////////////////////////////////////////////////////
	SwordMoveState::SwordMoveState(float attackRangeMin, float attackRangeMax)
		: _attackRangeMin(attackRangeMin), _attackRangeMax(attackRangeMax)
	{
		GM_ASSERT(attackRangeMin >= 0.f, "Sword Attack Range Min은 0 이상이어야 합니다.");
		GM_ASSERT(attackRangeMax >= attackRangeMin, "Sword Attack Range Max는 Min 이상이어야 합니다.");
	}

	void SwordMoveState::Enter(MonsterStateContext& context)
	{
		_moveType = MoveType::None;
		_lastFootstepBeatIndex = context.beatSystem ? context.beatSystem->GetCurrentBeatIndex() : -1;
		SetRootMotionEnabled(context, false);
		if (context.combatComponent != nullptr)
			SelectMove(context, context.combatComponent->GetTargetDistance());
	}

	void SwordMoveState::Tick(MonsterStateContext& context, float deltaTime)
	{
		if (context.combatComponent == nullptr || context.moveComponent == nullptr || context.animatorComponent == nullptr)
			return;

		if (context.combatComponent->HasTarget() == false || (IsDash() && IsAnimationCompleted(context)))
		{
			context.stateMachine->ChangeState(MonsterStateId::Idle);
			return;
		}

		const float distance = context.combatComponent->GetTargetDistance();
		if (CanAttack(context, distance))
		{
			context.stateMachine->ChangeState(MonsterStateId::Attack);
			return;
		}

		SelectMove(context, distance);
		if (IsDash() == false && context.beatSystem != nullptr)
		{
			const int64 currentBeatIndex = context.beatSystem->GetCurrentBeatIndex();
			if (currentBeatIndex != _lastFootstepBeatIndex)
			{
				_lastFootstepBeatIndex = currentBeatIndex;
				PlaySound2D(HiFiRushSound::MonsterFootsteps[_footstepIndex], 0.3f);
				_footstepIndex = (_footstepIndex + 1) % HiFiRushSound::MonsterFootsteps.size();
			}
		}

		FaceTarget(context, deltaTime);
		const Vector3 targetDirection = context.combatComponent->GetTargetDirection();
		Vector3 moveDirection{};
		switch (_moveType)
		{
		case MoveType::WalkFront:
			moveDirection = targetDirection;
			break;
		case MoveType::WalkBack:
			moveDirection = -targetDirection;
			break;
		case MoveType::WalkLeft:
			moveDirection = context.moveComponent->GetRightDirection();
			break;
		case MoveType::WalkRight:
			moveDirection = -context.moveComponent->GetRightDirection();
			break;
		default:
			return;
		}

		context.moveComponent->MoveAlong(moveDirection, deltaTime, false);
	}

	void SwordMoveState::Exit(MonsterStateContext& context)
	{
		SetRootMotionEnabled(context, false);
		_moveType = MoveType::None;
	}

	bool SwordMoveState::IsInAttackRange(float distance) const
	{
		return distance >= _attackRangeMin && distance < _attackRangeMax;
	}

	bool SwordMoveState::CanAttack(const MonsterStateContext& context, float distance) const
	{
		return IsDash() == false && context.combatComponent != nullptr && IsInAttackRange(distance) && context.combatComponent->IsAttackReady();
	}

	void SwordMoveState::SelectMove(MonsterStateContext& context, float distance)
	{
		if (context.animatorComponent == nullptr || IsDash())
			return;
		if (_moveType != MoveType::None && context.animatorComponent->HasLooped() == false)
			return;

		MoveType moveType;
		if (distance < _attackRangeMin)
		{
			const bool canDash = distance < SwordDashBackDistance;
			moveType = canDash && Math::RandomInt(0, 1) ? MoveType::DashBack : MoveType::WalkBack;
		}
		else if (distance >= _attackRangeMax)
		{
			const bool canDash = distance > SwordDashFrontDistance;
			moveType = canDash && Math::RandomInt(0, 1) ? MoveType::DashFront : MoveType::WalkFront;
		}
		else
		{
			moveType = Math::RandomInt(0, 1) ? MoveType::WalkLeft : MoveType::WalkRight;
		}

		SetMove(context, moveType);
	}

	void SwordMoveState::SetMove(MonsterStateContext& context, MoveType moveType)
	{
		if (moveType == MoveType::None || _moveType == moveType)
			return;

		_moveType = moveType;

		SwordAnimationId animationId = SwordAnimationId::WalkFront;
		bool isLoop = true;
		switch (_moveType)
		{
		case MoveType::WalkFront:
			animationId = SwordAnimationId::WalkFront;
			break;
		case MoveType::WalkBack:
			animationId = SwordAnimationId::WalkBack;
			break;
		case MoveType::WalkLeft:
			animationId = SwordAnimationId::WalkLeft;
			break;
		case MoveType::WalkRight:
			animationId = SwordAnimationId::WalkRight;
			break;
		case MoveType::DashFront:
			animationId = SwordAnimationId::DashFront;
			isLoop = false;
			break;
		case MoveType::DashBack:
			animationId = SwordAnimationId::DashBack;
			isLoop = false;
			break;
		default:
			return;
		}

		SetRootMotionEnabled(context, IsDash());
		PlayAnimation(context, GetSwordAnimationClipName(animationId), isLoop);
		if (IsDash())
			PlayRandomSound2D(HiFiRushSound::SwordDashes);
	}

	bool SwordMoveState::IsDash() const
	{
		return _moveType == MoveType::DashFront || _moveType == MoveType::DashBack;
	}

	// SwordAttackState /////////////////////////////////////////////////////////////////////////
	SwordAttackState::SwordAttackState(HitBoxComponent& hitBox)
		: _hitBox(hitBox)
	{
	}

	void SwordAttackState::Enter(MonsterStateContext& context)
	{
		_notifyConnection.Disconnect();
		_hitBox.EndAttack();

		if (context.combatComponent == nullptr || context.combatComponent->TryStartAttack() == false)
		{
			context.stateMachine->ChangeState(MonsterStateId::Move);
			return;
		}

		FaceTargetImmediate(context);
		_hitBox.SetWorldKnockbackDirection(context.moveComponent->GetForwardDirection());
		SetRootMotionEnabled(context, true);

		const bool useSlash = Math::RandomInt(0, 1) == 0;
		_animationId = useSlash ? SwordAnimationId::AttackSlash : SwordAnimationId::AttackJumpSlash;
		PlayAnimation(context, GetSwordAnimationClipName(_animationId), false);

		const std::shared_ptr<SkeletalAnimationClip> clip = context.animatorComponent->GetCurrentClip();
		GM_ASSERT_RETURN(clip, "Sword Attack Animation Clip이 없습니다.");
		GM_ASSERT_RETURN(clip->FindNotify(HiFiRushAnimationNotifyNames::ChargeStart), "Sword Attack Animation에 ChargeStart Notify가 없습니다.");
		GM_ASSERT_RETURN(clip->FindNotify(HiFiRushAnimationNotifyNames::EffectStart), "Sword Attack Animation에 EffectStart Notify가 없습니다.");
		const AnimationNotifyEvent* hitStartNotify = clip->FindNotify(HiFiRushAnimationNotifyNames::HitStart);
		GM_ASSERT_RETURN(hitStartNotify, "Sword Attack Animation에 HitStart Notify가 없습니다.");
		SkeletalAnimatorComponent* animator = context.animatorComponent;
		animator->GetNotifyEvent().Subscribe(_notifyConnection,
			[this, &context, animator](const AnimationNotifyEvent& event)
		{
			HandleAnimationNotify(context, *animator, event);
		});

		if (context.beatSystem != nullptr && context.beatSystem->HasPlaybackTime())
			animator->SetPlayRate(BeatMath::CalcAnimationPlayRate(context.beatSystem->GetCurrentBeat(), context.beatSystem->GetSecondsPerBeat(), hitStartNotify->time));
	}

	void SwordAttackState::Tick(MonsterStateContext& context, float)
	{
		if (context.animatorComponent == nullptr)
			return;

		if (IsAnimationCompleted(context))
			context.stateMachine->ChangeState(MonsterStateId::Idle);
	}

	void SwordAttackState::Exit(MonsterStateContext& context)
	{
		_notifyConnection.Disconnect();
		_hitBox.EndAttack();
		if (context.animatorComponent != nullptr)
			context.animatorComponent->SetPlayRate(1.f);
		SetRootMotionEnabled(context, false);
	}

	void SwordAttackState::HandleAnimationNotify(MonsterStateContext& context, SkeletalAnimatorComponent& animator, const AnimationNotifyEvent& event)
	{
		if (event.name == HiFiRushAnimationNotifyNames::ChargeStart)
		{
			PlaySound2D(HiFiRushSound::SwordCharge);
			SwordEffectComponent* effectComponent = context.stateMachine->GetOwner().GetComponent<SwordEffectComponent>();
			GM_ASSERT_RETURN(effectComponent, "Sword Attack Effect를 재생하려면 SwordEffectComponent가 필요합니다.");
			effectComponent->SpawnChargeEffect(_animationId);
		}
		else if (event.name == HiFiRushAnimationNotifyNames::EffectStart)
		{
			SwordEffectComponent* effectComponent = context.stateMachine->GetOwner().GetComponent<SwordEffectComponent>();
			GM_ASSERT_RETURN(effectComponent, "Sword Attack Effect를 재생하려면 SwordEffectComponent가 필요합니다.");
			effectComponent->SpawnSlashEffect(_animationId);
		}
		else if (event.name == HiFiRushAnimationNotifyNames::HitStart)
		{
			_hitBox.BeginAttack();
			PlayRandomSound2D(HiFiRushSound::SwordSwings, 0.5f);
			animator.SetPlayRate(1.f);
		}
		else if (event.name == HiFiRushAnimationNotifyNames::HitEnd)
		{
			_hitBox.EndAttack();
		}
	}

	// SwordDamageState /////////////////////////////////////////////////////////////////////////
	void SwordDamageState::Enter(MonsterStateContext& context)
	{
		SetRootMotionEnabled(context, true);

		SwordAnimationId animationId = SwordAnimationId::DamageMiddle;
		if (context.lastHitReactionType != HitReactionType::StrongKnockback)
		{
			const uint32 damageOffset = static_cast<uint32>(Math::RandomInt(0, 2));
			animationId = static_cast<SwordAnimationId>(ToIndex(SwordAnimationId::DamageLow0) + damageOffset);
		}

		PlayAnimation(context, GetSwordAnimationClipName(animationId), false);
		PlayRandomSound2D(HiFiRushSound::SwordDamageVoices, 0.3f);
	}

	void SwordDamageState::Tick(MonsterStateContext& context, float)
	{
		if (IsAnimationCompleted(context))
			context.stateMachine->ChangeState(MonsterStateId::Idle);
	}

	void SwordDamageState::Exit(MonsterStateContext& context)
	{
		SetRootMotionEnabled(context, false);
	}

	// SwordDeadState /////////////////////////////////////////////////////////////////////////
	void SwordDeadState::Enter(MonsterStateContext& context)
	{
		if (context.combatComponent != nullptr)
			context.combatComponent->SetCombatEnabled(false);

		SetRootMotionEnabled(context, true);
		PlayAnimation(context, GetSwordAnimationClipName(SwordAnimationId::Die), false);
		PlayRandomSound2D(HiFiRushSound::SwordDeathVoices);
	}

	void SwordDeadState::Tick(MonsterStateContext& context, float)
	{
		if (IsAnimationCompleted(context))
			context.stateMachine->CompleteDeathAnimation();
	}
}
