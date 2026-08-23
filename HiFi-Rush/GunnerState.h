#pragma once

#include "Event.h"
#include "MonsterState.h"

namespace gm
{
	struct AnimationNotifyEvent;
	class GunnerEffectComponent;
	class SkeletalAnimatorComponent;

	// GunnerIdleState /////////////////////////////////////////////////////////////////////////
	class GunnerIdleState final : public MonsterState
	{
	public:
		MonsterStateId GetStateId() const override { return MonsterStateId::Idle; }
		void Enter(MonsterStateContext& context) override;
		void Tick(MonsterStateContext& context, float deltaTime) override;
	};

	// GunnerMoveState /////////////////////////////////////////////////////////////////////////
	class GunnerMoveState final : public MonsterState
	{
	public:
		GunnerMoveState(float attackRangeMin, float attackRangeMax);

		MonsterStateId GetStateId() const override { return MonsterStateId::Move; }
		void Enter(MonsterStateContext& context) override;
		void Tick(MonsterStateContext& context, float deltaTime) override;
		void Exit(MonsterStateContext& context) override;

	private:
		enum class MoveType
		{
			None,
			WalkFront,
			WalkBack,
			WalkLeft,
			WalkRight,
			DashFront,
			DashBack,
		};

		bool IsInAttackRange(float distance) const;
		bool CanAttack(const MonsterStateContext& context, float distance) const;
		void SelectMove(MonsterStateContext& context, float distance);
		void SetMove(MonsterStateContext& context, MoveType moveType);
		bool IsDash() const;

		float		_attackRangeMin = 0.f;
		float		_attackRangeMax = 0.f;
		MoveType	_moveType = MoveType::None;
		int64		_lastFootstepBeatIndex = -1;
		size_t		_footstepIndex = 0;
	};

	// GunnerAttackState /////////////////////////////////////////////////////////////////////////
	class GunnerAttackState final : public MonsterState
	{
	public:
		explicit GunnerAttackState(int32 damage);

		MonsterStateId GetStateId() const override { return MonsterStateId::Attack; }
		void Enter(MonsterStateContext& context) override;
		void Tick(MonsterStateContext& context, float deltaTime) override;
		void Exit(MonsterStateContext& context) override;

	private:
		enum class AttackType
		{
			Ground,
			Sky,
		};

		enum class AttackPhase
		{
			Ready,
			Shoot,
			Landing,
		};

		void BeginShoot(MonsterStateContext& context);
		void BeginLanding(MonsterStateContext& context);
		void LockAim(MonsterStateContext& context);
		void HandleAnimationNotify(MonsterStateContext& context, SkeletalAnimatorComponent& animator, const AnimationNotifyEvent& event);
		void SpawnLaser(MonsterStateContext& context);
		void UpdateLaserGuide(MonsterStateContext& context);
		float GetAnimationBeat(const MonsterStateContext& context) const;
		float GetLaserGuideHalfWidth(const MonsterStateContext& context) const;
		GunnerEffectComponent* GetEffectComponent(const MonsterStateContext& context) const;

		EventConnection		_notifyConnection{};
		int32				_damage = 0;
		AttackType			_attackType = AttackType::Ground;
		AttackPhase			_attackPhase = AttackPhase::Ready;
		Vector3				_lockedAimDirection{ 0.f, 0.f, 1.f };
		float				_lockedTargetDistance = 0.f;
		bool				_previousUseGravity = true;
		bool				_isAimLocked = false;
		bool				_hasSpawnedLaser = false;
		bool				_hasStartedLaserGuide = false;
		bool				_hasPlayedShootSound = false;
		bool				_overrodeSkyMovement = false;
	};

	// GunnerDamageState /////////////////////////////////////////////////////////////////////////
	class GunnerDamageState final : public MonsterState
	{
	public:
		MonsterStateId GetStateId() const override { return MonsterStateId::Damage; }
		void Enter(MonsterStateContext& context) override;
		void Tick(MonsterStateContext& context, float deltaTime) override;
		void Exit(MonsterStateContext& context) override;
	};

	// GunnerDeadState /////////////////////////////////////////////////////////////////////////
	class GunnerDeadState final : public MonsterState
	{
	public:
		MonsterStateId GetStateId() const override { return MonsterStateId::Dead; }
		void Enter(MonsterStateContext& context) override;
		void Tick(MonsterStateContext& context, float deltaTime) override;
	};
}
