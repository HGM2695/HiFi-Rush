#pragma once

#include "Event.h"
#include "MonsterState.h"

namespace gm
{
	struct AnimationNotifyEvent;
	class HitBoxComponent;
	class SkeletalAnimatorComponent;

	/// SwordIdleState //////////////////////////////////////////////////////////////////////////////
	class SwordIdleState final : public MonsterState
	{
	public:
		MonsterStateId	GetStateId() const override { return MonsterStateId::Idle; }
		void			Enter(MonsterStateContext& context) override;
		void			Tick(MonsterStateContext& context, float deltaTime) override;
	};

	/// SwordMoveState //////////////////////////////////////////////////////////////////////////////
	class SwordMoveState final : public MonsterState
	{
	public:
		SwordMoveState(float attackRangeMin, float attackRangeMax);

		MonsterStateId	GetStateId() const override { return MonsterStateId::Move; }
		void			Enter(MonsterStateContext& context) override;
		void			Tick(MonsterStateContext& context, float deltaTime) override;
		void			Exit(MonsterStateContext& context) override;

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

		bool	IsInAttackRange(float distance) const;
		bool	CanAttack(const MonsterStateContext& context, float distance) const;
		void	SelectMove(MonsterStateContext& context, float distance);
		void	SetMove(MonsterStateContext& context, MoveType moveType);
		bool	IsDash() const;

	private:
		float		_attackRangeMin = 0.f;
		float		_attackRangeMax = 0.f;
		MoveType	_moveType = MoveType::None;
	};

	/// SwordAttackState //////////////////////////////////////////////////////////////////////////////
	class SwordAttackState final : public MonsterState
	{
	public:
		explicit SwordAttackState(HitBoxComponent& hitBox);

		MonsterStateId	GetStateId() const override { return MonsterStateId::Attack; }
		void			Enter(MonsterStateContext& context) override;
		void			Tick(MonsterStateContext& context, float deltaTime) override;
		void			Exit(MonsterStateContext& context) override;

	private:
		void HandleAnimationNotify(SkeletalAnimatorComponent& animator, const AnimationNotifyEvent& event);

	private:
		HitBoxComponent&	_hitBox;
		EventConnection		_notifyConnection{};
	};

	/// SwordDamageState //////////////////////////////////////////////////////////////////////////////
	class SwordDamageState final : public MonsterState
	{
	public:
		MonsterStateId	GetStateId() const override { return MonsterStateId::Damage; }
		void			Enter(MonsterStateContext& context) override;
		void			Tick(MonsterStateContext& context, float deltaTime) override;
		void			Exit(MonsterStateContext& context) override;
	};

	/// SwordDeadState //////////////////////////////////////////////////////////////////////////////
	class SwordDeadState final : public MonsterState
	{
	public:
		MonsterStateId	GetStateId() const override { return MonsterStateId::Dead; }
		void			Enter(MonsterStateContext& context) override;
		void			Tick(MonsterStateContext& context, float deltaTime) override;
	};
}
