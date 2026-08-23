#include "GunnerState.h"

#include "AudioStatics.h"
#include "BeatMath.h"
#include "BeatSystem.h"
#include "CharacterMovementComponent.h"
#include "GameObject.h"
#include "GunnerAnimationTypes.h"
#include "GunnerEffectComponent.h"
#include "HiFiRushAnimationNotifyNames.h"
#include "HiFiRushCollisionLayers.h"
#include "HiFiRushAudio.h"
#include "MathUtil.h"
#include "MonsterCombatComponent.h"
#include "MonsterStateMachineComponent.h"
#include "NavMeshControllerComponent.h"
#include "Random.h"
#include "Rigidbody3DComponent.h"
#include "Scene.h"
#include "SkeletalAnimationClip.h"
#include "SkeletalAnimatorComponent.h"
#include "TemporaryHitBoxObject.h"
#include "TransformComponent.h"

#include <algorithm>
#include <cmath>

namespace gm
{
	namespace
	{
		constexpr float GunnerDashBackDistanceRatio = 0.7f;
		constexpr float GunnerDashFrontDistanceRatio = 1.5f;
		constexpr float GunnerLaserMinimumLength = 14.f;
		constexpr float GunnerLaserExtraLength = 3.f;
		constexpr float GunnerLaserWidth = 2.f;
		constexpr float GunnerLaserHeight = 1.5f;
		constexpr float GunnerLaserLifetime = 0.2f;
		constexpr float GunnerLaserVisualMinimumLength = 7.f;
		constexpr float GunnerAnimationTicksPerBeat = 15.f;
		constexpr float GunnerGroundReadyGuideHalfWidth = 1.f;
		constexpr float GunnerGroundReadyGuideNarrowHalfWidth = 0.7f;
		constexpr float GunnerGroundAttackGuideHalfWidth = 0.5f;
		constexpr float GunnerSkyAttackGuideHalfWidth = 0.8f;
		constexpr float GunnerGuideNarrowSpeedPerBeat = 1.5f;
		constexpr float GunnerShootSoundBeat = 1.f + 6.f / GunnerAnimationTicksPerBeat;
	}

	// GunnerIdleState /////////////////////////////////////////////////////////////////////////
	void GunnerIdleState::Enter(MonsterStateContext& context)
	{
		SetRootMotionEnabled(context, false);
		PlayAnimation(context, GetGunnerAnimationClipName(GunnerAnimationId::Idle), true);
	}

	void GunnerIdleState::Tick(MonsterStateContext& context, float)
	{
		if (context.combatComponent != nullptr && context.combatComponent->HasTarget())
			context.stateMachine->ChangeState(MonsterStateId::Move);
	}

	// GunnerMoveState /////////////////////////////////////////////////////////////////////////
	GunnerMoveState::GunnerMoveState(float attackRangeMin, float attackRangeMax)
		: _attackRangeMin(attackRangeMin), _attackRangeMax(attackRangeMax)
	{
	}

	void GunnerMoveState::Enter(MonsterStateContext& context)
	{
		_moveType = MoveType::None;
		_lastFootstepBeatIndex = context.beatSystem ? context.beatSystem->GetCurrentBeatIndex() : -1;
		SetRootMotionEnabled(context, false);
		if (context.combatComponent != nullptr)
			SelectMove(context, context.combatComponent->GetTargetDistance());
	}

	void GunnerMoveState::Tick(MonsterStateContext& context, float deltaTime)
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

	void GunnerMoveState::Exit(MonsterStateContext& context)
	{
		SetRootMotionEnabled(context, false);
		_moveType = MoveType::None;
	}

	bool GunnerMoveState::IsInAttackRange(float distance) const
	{
		return distance >= _attackRangeMin && distance < _attackRangeMax;
	}

	bool GunnerMoveState::CanAttack(const MonsterStateContext& context, float distance) const
	{
		return IsDash() == false && context.combatComponent != nullptr && IsInAttackRange(distance) && context.combatComponent->IsAttackReady();
	}

	void GunnerMoveState::SelectMove(MonsterStateContext& context, float distance)
	{
		if (context.animatorComponent == nullptr || IsDash())
			return;
		if (_moveType != MoveType::None && context.animatorComponent->HasLooped() == false)
			return;

		MoveType moveType;
		if (distance < _attackRangeMin)
		{
			const bool canDash = distance < _attackRangeMin * GunnerDashBackDistanceRatio;
			moveType = canDash && Math::RandomInt(0, 1) ? MoveType::DashBack : MoveType::WalkBack;
		}
		else if (distance >= _attackRangeMax)
		{
			const bool canDash = distance > _attackRangeMax * GunnerDashFrontDistanceRatio;
			moveType = canDash && Math::RandomInt(0, 1) ? MoveType::DashFront : MoveType::WalkFront;
		}
		else
		{
			moveType = Math::RandomInt(0, 1) ? MoveType::WalkLeft : MoveType::WalkRight;
		}

		SetMove(context, moveType);
	}

	void GunnerMoveState::SetMove(MonsterStateContext& context, MoveType moveType)
	{
		if (moveType == MoveType::None || _moveType == moveType)
			return;

		_moveType = moveType;
		GunnerAnimationId animationId = GunnerAnimationId::WalkFront;
		bool isLoop = true;
		switch (_moveType)
		{
		case MoveType::WalkFront:
			animationId = GunnerAnimationId::WalkFront;
			break;
		case MoveType::WalkBack:
			animationId = GunnerAnimationId::WalkBack;
			break;
		case MoveType::WalkLeft:
			animationId = GunnerAnimationId::WalkRight;
			break;
		case MoveType::WalkRight:
			animationId = GunnerAnimationId::WalkLeft;
			break;
		case MoveType::DashFront:
			animationId = GunnerAnimationId::DashFront;
			isLoop = false;
			break;
		case MoveType::DashBack:
			animationId = GunnerAnimationId::DashBack;
			isLoop = false;
			break;
		default:
			return;
		}

		SetRootMotionEnabled(context, IsDash());
		PlayAnimation(context, GetGunnerAnimationClipName(animationId), isLoop);
		if (IsDash())
			PlayRandomSound2D(HiFiRushSound::GunnerDashes);
	}

	bool GunnerMoveState::IsDash() const
	{
		return _moveType == MoveType::DashFront || _moveType == MoveType::DashBack;
	}

	// GunnerAttackState /////////////////////////////////////////////////////////////////////////
	GunnerAttackState::GunnerAttackState(int32 damage)
		: _damage(damage)
	{
	}

	void GunnerAttackState::Enter(MonsterStateContext& context)
	{
		_notifyConnection.Disconnect();
		_isAimLocked = false;
		_hasSpawnedLaser = false;
		_hasStartedLaserGuide = false;
		_hasPlayedShootSound = false;
		_lockedAimDirection = Vector3{ 0.f, 0.f, 1.f };
		_lockedTargetDistance = 0.f;

		if (context.combatComponent == nullptr || context.combatComponent->TryStartAttack() == false)
		{
			context.stateMachine->ChangeState(MonsterStateId::Move);
			return;
		}

		FaceTargetImmediate(context);
		_attackType = Math::RandomInt(0, 1) ? AttackType::Ground : AttackType::Sky;
		_attackPhase = AttackPhase::Ready;
		if (_attackType == AttackType::Sky)
		{
			Rigidbody3DComponent* rigidbody = context.stateMachine->GetOwner().GetRigidbody3D();
			GM_ASSERT_RETURN(rigidbody, "Gunner Sky Attack에는 Rigidbody3DComponent가 필요합니다.");
			_previousUseGravity = rigidbody->IsUseGravity();
			rigidbody->SetUseGravity(false);
			rigidbody->ClearVerticalVelocity();
			_overrodeSkyMovement = true;
		}
		SetRootMotionEnabled(context, true);
		const GunnerAnimationId animationId = _attackType == AttackType::Ground ? GunnerAnimationId::GroundAttackReady : GunnerAnimationId::SkyAttackReady;
		PlayAnimation(context, GetGunnerAnimationClipName(animationId), false);
		GunnerEffectComponent* effectComponent = GetEffectComponent(context);
		if (_attackType == AttackType::Ground && effectComponent)
		{
			effectComponent->StartGroundReadyLaserGuide();
			_hasStartedLaserGuide = true;
		}
		UpdateLaserGuide(context);
	}

	void GunnerAttackState::Tick(MonsterStateContext& context, float deltaTime)
	{
		if (context.animatorComponent == nullptr)
			return;

		if (_attackPhase == AttackPhase::Ready)
		{
			FaceTarget(context, deltaTime);
			if (_attackType == AttackType::Sky && _hasStartedLaserGuide == false)
			{
				NavMeshControllerComponent* navMeshController = context.stateMachine->GetOwner().GetComponent<NavMeshControllerComponent>();
				if (navMeshController && navMeshController->IsGrounded() == false)
				{
					GunnerEffectComponent* effectComponent = GetEffectComponent(context);
					if (effectComponent)
					{
						effectComponent->StartSkyReadyLaserGuide();
						_hasStartedLaserGuide = true;
					}
				}
			}
		}
		if (_attackPhase == AttackPhase::Shoot && _hasPlayedShootSound == false && GetAnimationBeat(context) >= GunnerShootSoundBeat)
		{
			PlaySound2D(HiFiRushSound::GunnerShoot);
			_hasPlayedShootSound = true;
		}
		UpdateLaserGuide(context);

		if (IsAnimationCompleted(context) == false)
			return;

		switch (_attackPhase)
		{
		case AttackPhase::Ready:
			BeginShoot(context);
			break;
		case AttackPhase::Shoot:
			BeginLanding(context);
			break;
		case AttackPhase::Landing:
			context.stateMachine->ChangeState(MonsterStateId::Idle);
			break;
		}
	}

	void GunnerAttackState::Exit(MonsterStateContext& context)
	{
		_notifyConnection.Disconnect();
		_isAimLocked = false;
		GunnerEffectComponent* effectComponent = GetEffectComponent(context);
		if (effectComponent)
			effectComponent->StopLaserGuide();
		if (context.animatorComponent != nullptr)
			context.animatorComponent->SetPlayRate(1.f);
		if (_overrodeSkyMovement)
		{
			Rigidbody3DComponent* rigidbody = context.stateMachine->GetOwner().GetRigidbody3D();
			if (rigidbody != nullptr)
				rigidbody->SetUseGravity(_previousUseGravity);
			_overrodeSkyMovement = false;
		}
		SetRootMotionEnabled(context, false);
	}

	void GunnerAttackState::BeginShoot(MonsterStateContext& context)
	{
		_attackPhase = AttackPhase::Shoot;
		_isAimLocked = false;
		const GunnerAnimationId animationId = _attackType == AttackType::Ground ? GunnerAnimationId::GroundAttackShoot : GunnerAnimationId::SkyAttackShoot;
		PlayAnimation(context, GetGunnerAnimationClipName(animationId), false);
		LockAim(context);
		GunnerEffectComponent* effectComponent = GetEffectComponent(context);
		if (effectComponent)
		{
			if (_attackType == AttackType::Ground)
				effectComponent->StartGroundAttackLaserGuide();
			else
				effectComponent->StartSkyAttackLaserGuide();
			_hasStartedLaserGuide = true;
		}
		UpdateLaserGuide(context);

		const std::shared_ptr<SkeletalAnimationClip> clip = context.animatorComponent->GetCurrentClip();
		GM_ASSERT_RETURN(clip, "Gunner Shoot Animation Clip이 없습니다.");
		const AnimationNotifyEvent* hitStartNotify = clip->FindNotify(HiFiRushAnimationNotifyNames::HitStart);
		GM_ASSERT_RETURN(hitStartNotify, "Gunner Shoot Animation에 HitStart Notify가 없습니다.");
		SkeletalAnimatorComponent* animator = context.animatorComponent;
		animator->GetNotifyEvent().Subscribe(_notifyConnection,
			[this, &context, animator](const AnimationNotifyEvent& event)
			{
				HandleAnimationNotify(context, *animator, event);
			});

		if (context.beatSystem != nullptr && context.beatSystem->HasPlaybackTime())
			animator->SetPlayRate(BeatMath::CalcAnimationPlayRate(context.beatSystem->GetCurrentBeat(), context.beatSystem->GetSecondsPerBeat(), hitStartNotify->time));
	}

	void GunnerAttackState::LockAim(MonsterStateContext& context)
	{
		if (_isAimLocked)
			return;

		_isAimLocked = true;
		if (context.moveComponent != nullptr)
			_lockedAimDirection = context.moveComponent->GetForwardDirection();
		if (context.combatComponent == nullptr || context.combatComponent->HasTarget() == false)
			return;

		GameObject& owner = context.stateMachine->GetOwner();
		TransformComponent* ownerTransform = owner.GetTransform();
		GameObject* target = context.combatComponent->GetTarget();
		TransformComponent* targetTransform = target != nullptr ? target->GetTransform() : nullptr;
		if (ownerTransform == nullptr || targetTransform == nullptr)
			return;

		Vector3 start = ownerTransform->GetPosition();
		Vector3 targetPosition = targetTransform->GetPosition();
		if (_attackType == AttackType::Sky)
		{
			GunnerEffectComponent* effectComponent = GetEffectComponent(context);
			start = effectComponent ? effectComponent->GetLaserSocketPosition() : start + Vector3{ 0.f, 1.5f, 0.f };
			targetPosition.y += 1.5f;
		}
		const Vector3 offset = targetPosition - start;
		const float distance = offset.Length();
		if (distance <= 0.000001f)
			return;

		_lockedAimDirection = offset / distance;
		_lockedTargetDistance = distance;
	}

	void GunnerAttackState::BeginLanding(MonsterStateContext& context)
	{
		_notifyConnection.Disconnect();
		_attackPhase = AttackPhase::Landing;
		GunnerEffectComponent* effectComponent = GetEffectComponent(context);
		if (effectComponent)
			effectComponent->StopLaserGuide();
		if (context.animatorComponent != nullptr)
			context.animatorComponent->SetPlayRate(1.f);
		const GunnerAnimationId animationId = _attackType == AttackType::Ground ? GunnerAnimationId::GroundAttackLanding : GunnerAnimationId::SkyAttackLanding;
		PlayAnimation(context, GetGunnerAnimationClipName(animationId), false);
	}

	void GunnerAttackState::HandleAnimationNotify(MonsterStateContext& context, SkeletalAnimatorComponent& animator, const AnimationNotifyEvent& event)
	{
		if (event.name == HiFiRushAnimationNotifyNames::HitStart)
		{
			LockAim(context);
			SpawnLaser(context);
			_hasSpawnedLaser = true;
			animator.SetPlayRate(1.f);
		}
	}

	void GunnerAttackState::SpawnLaser(MonsterStateContext& context)
	{
		if (context.stateMachine == nullptr)
			return;

		GameObject& owner = context.stateMachine->GetOwner();
		Scene* scene = owner.GetScene();
		TransformComponent* ownerTransform = owner.GetTransform();
		if (scene == nullptr || ownerTransform == nullptr)
			return;

		GunnerEffectComponent* effectComponent = GetEffectComponent(context);
		GM_ASSERT_RETURN(effectComponent, "Gunner Laser Effect를 생성하려면 GunnerEffectComponent가 필요합니다.");
		Vector3 start = ownerTransform->GetPosition();
		if (_attackType == AttackType::Sky)
			start = effectComponent->GetLaserSocketPosition();
		else
		{
			start += _lockedAimDirection * 0.2f;
			start.y += 1.f;
		}

		const float laserLength = std::max(_lockedTargetDistance + GunnerLaserExtraLength, GunnerLaserMinimumLength);
		const float visualLength = std::max(_lockedTargetDistance, GunnerLaserVisualMinimumLength);
		effectComponent->StopLaserGuide();
		GM_ASSERT_RETURN(effectComponent->SpawnLaser(start, _lockedAimDirection, visualLength), "Gunner Laser Effect 생성에 실패했습니다.");

		TemporaryBoxHitBoxDesc desc{};
		const Vector3 laserUp = std::abs(_lockedAimDirection.Dot(Vector3::Up)) > 0.99f ? Vector3::Right : Vector3::Up;
		desc.world = Math::CreateLookAtLH(start, start + _lockedAimDirection, laserUp).Invert();
		desc.colliderId = L"Attack";
		desc.localCenter = Vector3{ 0.f, 0.f, laserLength * 0.5f };
		desc.size = Vector3{ GunnerLaserWidth, GunnerLaserHeight, laserLength };
		desc.collisionLayer = HiFiRushCollisionLayer::MonsterAttack;
		desc.collisionMask = HiFiRushCollisionLayer::Player;
		desc.damageInfo.amount = _damage;
		desc.damageInfo.hitReactionType = HitReactionType::StrongKnockback;
		desc.damageInfo.worldKnockbackDirection = _lockedAimDirection;
		desc.lifetime = GunnerLaserLifetime;
		GM_ASSERT_RETURN(scene->SpawnGameObject<TemporaryHitBoxObject>(desc), "Gunner Laser GameObject 생성에 실패했습니다.");
	}

	void GunnerAttackState::UpdateLaserGuide(MonsterStateContext& context)
	{
		if (_hasSpawnedLaser)
			return;

		GunnerEffectComponent* effectComponent = GetEffectComponent(context);
		if (effectComponent == nullptr || context.combatComponent == nullptr || context.combatComponent->HasTarget() == false)
			return;

		GameObject& owner = context.stateMachine->GetOwner();
		GameObject* target = context.combatComponent->GetTarget();
		if (target == nullptr)
			return;

		if (_attackType == AttackType::Ground)
		{
			const float halfWidth = GetLaserGuideHalfWidth(context);
			if (halfWidth <= 0.f)
			{
				effectComponent->StopLaserGuide();
				return;
			}
			const Vector3 start = owner.GetTransform()->GetPosition();
			const Vector3 targetPosition = _isAimLocked ? start + _lockedAimDirection * _lockedTargetDistance : target->GetTransform()->GetPosition();
			effectComponent->UpdateGroundLaserGuide(start, targetPosition, halfWidth);
			return;
		}

		const float halfWidth = GetLaserGuideHalfWidth(context);
		if (halfWidth <= 0.f)
		{
			effectComponent->StopLaserGuide();
			return;
		}
		const Vector3 start = effectComponent->GetLaserSocketPosition();
		Vector3 targetPosition = _isAimLocked ? start + _lockedAimDirection * _lockedTargetDistance : target->GetTransform()->GetPosition() + Vector3{ 0.f, 1.5f, 0.f };
		effectComponent->UpdateSkyLaserGuide(start, targetPosition);
	}

	float GunnerAttackState::GetAnimationBeat(const MonsterStateContext& context) const
	{
		const std::shared_ptr<SkeletalAnimationClip> clip = context.animatorComponent->GetCurrentClip();
		if (clip == nullptr || clip->GetTicksPerSecond() <= 0.f)
			return 0.f;
		return context.animatorComponent->GetPlayTime() / (GunnerAnimationTicksPerBeat / clip->GetTicksPerSecond());
	}

	float GunnerAttackState::GetLaserGuideHalfWidth(const MonsterStateContext& context) const
	{
		const float animationBeat = GetAnimationBeat(context);
		if (_attackPhase == AttackPhase::Ready)
			return animationBeat >= 1.f ? GunnerGroundReadyGuideNarrowHalfWidth : GunnerGroundReadyGuideHalfWidth;
		const float initialHalfWidth = _attackType == AttackType::Ground ? GunnerGroundAttackGuideHalfWidth : GunnerSkyAttackGuideHalfWidth;
		return animationBeat <= 1.f ? initialHalfWidth : std::max(0.f, initialHalfWidth - (animationBeat - 1.f) * GunnerGuideNarrowSpeedPerBeat);
	}

	GunnerEffectComponent* GunnerAttackState::GetEffectComponent(const MonsterStateContext& context) const
	{
		return context.stateMachine->GetOwner().GetComponent<GunnerEffectComponent>();
	}

	// GunnerDamageState /////////////////////////////////////////////////////////////////////////
	void GunnerDamageState::Enter(MonsterStateContext& context)
	{
		SetRootMotionEnabled(context, true);
		PlayAnimation(context, GetGunnerAnimationClipName(GunnerAnimationId::DamageLow), false);
	}

	void GunnerDamageState::Tick(MonsterStateContext& context, float)
	{
		if (IsAnimationCompleted(context))
			context.stateMachine->ChangeState(MonsterStateId::Idle);
	}

	void GunnerDamageState::Exit(MonsterStateContext& context)
	{
		SetRootMotionEnabled(context, false);
	}

	// GunnerDeadState /////////////////////////////////////////////////////////////////////////
	void GunnerDeadState::Enter(MonsterStateContext& context)
	{
		if (context.combatComponent != nullptr)
			context.combatComponent->SetCombatEnabled(false);

		SetRootMotionEnabled(context, true);
		PlayAnimation(context, GetGunnerAnimationClipName(GunnerAnimationId::Die), false);
		PlayRandomSound2D(HiFiRushSound::GunnerDeathVoices);
	}

	void GunnerDeadState::Tick(MonsterStateContext& context, float)
	{
		if (IsAnimationCompleted(context))
			context.stateMachine->CompleteDeathAnimation();
	}
}
