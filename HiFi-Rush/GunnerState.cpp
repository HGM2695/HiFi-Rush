#include "GunnerState.h"

#include "BeatMath.h"
#include "BeatSystem.h"
#include "CharacterMovementComponent.h"
#include "GameObject.h"
#include "GunnerAnimationTypes.h"
#include "HiFiRushAnimationNotifyNames.h"
#include "HiFiRushCollisionLayers.h"
#include "MathUtil.h"
#include "MonsterCombatComponent.h"
#include "MonsterStateMachineComponent.h"
#include "Random.h"
#include "Rigidbody3DComponent.h"
#include "Scene.h"
#include "SkeletalAnimationClip.h"
#include "SkeletalAnimatorComponent.h"
#include "TemporaryBoxHitBoxObject.h"
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
		_isFacingLocked = false;

		if (context.combatComponent == nullptr || context.combatComponent->TryStartAttack() == false)
		{
			context.stateMachine->ChangeState(MonsterStateId::Move);
			return;
		}

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
	}

	void GunnerAttackState::Tick(MonsterStateContext& context, float deltaTime)
	{
		if (context.animatorComponent == nullptr)
			return;

		if (_attackPhase != AttackPhase::Landing && _isFacingLocked == false)
			FaceTarget(context, deltaTime);

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
		_isFacingLocked = false;
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
		const GunnerAnimationId animationId = _attackType == AttackType::Ground ? GunnerAnimationId::GroundAttackShoot : GunnerAnimationId::SkyAttackShoot;
		PlayAnimation(context, GetGunnerAnimationClipName(animationId), false);

		const std::shared_ptr<SkeletalAnimationClip> clip = context.animatorComponent->GetCurrentClip();
		GM_ASSERT_RETURN(clip, "Gunner Shoot Animation Clip이 없습니다.");
		const AnimationNotifyEvent* hitStartNotify = clip->FindNotify(HiFiRushAnimationNotifyNames::HitStart);
		SkeletalAnimatorComponent* animator = context.animatorComponent;
		animator->GetNotifyEvent().Subscribe(_notifyConnection,
			[this, &context, animator](const AnimationNotifyEvent& event)
			{
				HandleAnimationNotify(context, *animator, event);
			});

		if (context.beatSystem != nullptr && context.beatSystem->HasPlaybackTime())
			animator->SetPlayRate(BeatMath::CalcAnimationPlayRate(context.beatSystem->GetCurrentBeat(), context.beatSystem->GetSecondsPerBeat(), hitStartNotify->time));
	}

	void GunnerAttackState::BeginLanding(MonsterStateContext& context)
	{
		_notifyConnection.Disconnect();
		_attackPhase = AttackPhase::Landing;
		if (context.animatorComponent != nullptr)
			context.animatorComponent->SetPlayRate(1.f);
		const GunnerAnimationId animationId = _attackType == AttackType::Ground ? GunnerAnimationId::GroundAttackLanding : GunnerAnimationId::SkyAttackLanding;
		PlayAnimation(context, GetGunnerAnimationClipName(animationId), false);
	}

	void GunnerAttackState::HandleAnimationNotify(MonsterStateContext& context, SkeletalAnimatorComponent& animator, const AnimationNotifyEvent& event)
	{
		if (event.name == HiFiRushAnimationNotifyNames::HitStart)
		{
			_isFacingLocked = true;
			SpawnLaser(context);
			animator.SetPlayRate(1.f);
		}
	}

	void GunnerAttackState::SpawnLaser(MonsterStateContext& context)
	{
		if (context.stateMachine == nullptr || context.combatComponent == nullptr)
			return;

		GameObject& owner = context.stateMachine->GetOwner();
		Scene* scene = owner.GetScene();
		TransformComponent* ownerTransform = owner.GetTransform();
		if (scene == nullptr || ownerTransform == nullptr)
			return;

		Vector3 start = ownerTransform->GetPosition();
		Vector3 direction = context.combatComponent->GetTargetDirection();
		float targetDistance = context.combatComponent->GetTargetDistance();
		GameObject* target = context.combatComponent->GetTarget();
		if (_attackType == AttackType::Sky && target != nullptr && target->GetTransform() != nullptr)
		{
			start.y += 1.5f;
			Vector3 targetPosition = target->GetTransform()->GetPosition();
			targetPosition.y += 0.9f;
			const Vector3 offset = targetPosition - start;
			targetDistance = offset.Length();
			if (targetDistance > 0.000001f)
				direction = offset / targetDistance;
		}
		else
		{
			start.y += 0.75f;
		}

		const float laserLength = std::max(targetDistance + GunnerLaserExtraLength, GunnerLaserMinimumLength);

		TemporaryBoxHitBoxDesc desc{};
		desc.world = Math::CreateLookAtLH(start, start + direction, Vector3::Up).Invert();
		desc.colliderId = L"Attack";
		desc.localCenter = Vector3{ 0.f, 0.f, laserLength * 0.5f };
		desc.size = Vector3{ GunnerLaserWidth, GunnerLaserHeight, laserLength };
		desc.collisionLayer = HiFiRushCollisionLayer::MonsterAttack;
		desc.collisionMask = HiFiRushCollisionLayer::Player;
		desc.damageInfo.amount = _damage;
		desc.damageInfo.hitReactionType = HitReactionType::StrongKnockback;
		desc.lifetime = GunnerLaserLifetime;
		GM_ASSERT_RETURN(scene->SpawnGameObject<TemporaryHitBoxObject>(desc), "Gunner Laser GameObject 생성에 실패했습니다.");
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
	}

	void GunnerDeadState::Tick(MonsterStateContext& context, float)
	{
		if (IsAnimationCompleted(context))
			context.stateMachine->CompleteDeathAnimation();
	}
}
