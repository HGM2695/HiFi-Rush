#pragma once

#include "ChiAnimationTypes.h"
#include "ChiStateContext.h"
#include "ChiStateTypes.h"
#include "MathTypes.h"

namespace gm
{
	class ChiState
	{
	public:
		virtual ~ChiState() = default;

		virtual ChiStateId GetStateId() const = 0;
		virtual void Enter(ChiStateContext& context) {}
		virtual void Tick(ChiStateContext& context, float deltaTime) {}
		virtual void Exit(ChiStateContext& context) {}

	protected:
		void	PlayAnimation(ChiStateContext& context, ChiAnimationId animationId, bool isLoop) const;
		void	ReturnToIdleOrRun(ChiStateContext& context) const;
		bool	IsAnimationCompleted(const ChiStateContext& context) const;
	};

	/// Clip //////////////////////////////////////////////////////////////////////////////
	class ChiClipState : public ChiState
	{
	public:
		ChiClipState(ChiStateId stateId, ChiAnimationId animationId, bool isLoop = false);

		virtual ChiStateId GetStateId() const override { return _stateId; }
		virtual void Enter(ChiStateContext& context) override;
		virtual void Tick(ChiStateContext& context, float deltaTime) override;
		virtual void Exit(ChiStateContext& context) override;

	protected:
		ChiAnimationId GetAnimationId() const { return _animationId; }

	private:
		ChiStateId		_stateId = ChiStateId::None;
		ChiAnimationId	_animationId = ChiAnimationId::Idle;
		bool			_isLoop = false;

		bool			_prevMoveEnabled = true;
		bool			_disabledMoveOnEnter = false;

		bool			_prevRootMotionEnabled = false;
		bool			_enabledRootMotionOnEnter = false;
	};

	/// Idle //////////////////////////////////////////////////////////////////////////////
	class ChiIdleState final : public ChiState
	{
	public:
		virtual ChiStateId GetStateId() const override { return ChiStateId::Idle; }
		virtual void Enter(ChiStateContext& context) override;
		virtual void Tick(ChiStateContext& context, float deltaTime) override;
	};

	/// Run //////////////////////////////////////////////////////////////////////////////
	class ChiRunState final : public ChiState
	{
	public:
		virtual ChiStateId GetStateId() const override { return ChiStateId::Run; }
		virtual void Enter(ChiStateContext& context) override;
		virtual void Tick(ChiStateContext& context, float deltaTime) override;
	};

	/// Dash //////////////////////////////////////////////////////////////////////////////
	enum class ChiDashDirection
	{
		Front,
		Back,
		Left,
		Right,
		InputOrFront,
	};

	class ChiDashState : public ChiClipState
	{
	public:
		ChiDashState(ChiStateId stateId, ChiAnimationId animationId, ChiDashDirection direction, float dashSpeed, bool rotateToDashDirection, ChiStateId nextDashState = ChiStateId::None);

		virtual void Enter(ChiStateContext& context) override;
		virtual void Tick(ChiStateContext& context, float deltaTime) override;
		virtual void Exit(ChiStateContext& context) override;

	protected:
		bool TryChangeDashAttack(ChiStateContext& context) const;
		Vector3 GetDashDirection(ChiStateContext& context) const;
		const Vector3& GetCachedDirection() const { return _cachedDirection; }
		float GetDashSpeed() const { return _dashSpeed; }

	private:
		ChiDashDirection	_direction = ChiDashDirection::Front;
		ChiStateId			_nextDashState = ChiStateId::None;
		Vector3				_cachedDirection{};
		float				_dashSpeed = 0.f;
		bool				_rotateToDashDirection = false;
		bool				_prevMoveEnabled = true;
	};

	class ChiDashFrontState final : public ChiDashState
	{
	public:
		ChiDashFrontState();
	};

	class ChiDashBackState final : public ChiDashState
	{
	public:
		ChiDashBackState();
	};

	class ChiDashLeftState final : public ChiDashState
	{
	public:
		ChiDashLeftState();
	};

	class ChiDashRightState final : public ChiDashState
	{
	public:
		ChiDashRightState();
	};

	class ChiDashDoubleState final : public ChiDashState
	{
	public:
		ChiDashDoubleState();
	};

	class ChiDashTripleState final : public ChiDashState
	{
	public:
		ChiDashTripleState();
	};

	class ChiDashSkyState final : public ChiDashState
	{
	public:
		ChiDashSkyState();
		virtual void Tick(ChiStateContext& context, float deltaTime) override;
	};

	class ChiDashSkyFallState final : public ChiClipState
	{
	public:
		ChiDashSkyFallState();
		virtual void Tick(ChiStateContext& context, float deltaTime) override;
	};

	/// Jump //////////////////////////////////////////////////////////////////////////////
	class ChiJumpUpState final : public ChiClipState
	{
	public:
		ChiJumpUpState();
		virtual void Tick(ChiStateContext& context, float deltaTime) override;
	};

	class ChiJumpDownState final : public ChiClipState
	{
	public:
		ChiJumpDownState();
		virtual void Tick(ChiStateContext& context, float deltaTime) override;
	};

	class ChiJumpLandingState final : public ChiClipState
	{
	public:
		ChiJumpLandingState();
		virtual void Tick(ChiStateContext& context, float deltaTime) override;
	};

	class ChiJumpDoubleUpState final : public ChiClipState
	{
	public:
		ChiJumpDoubleUpState();
		virtual void Tick(ChiStateContext& context, float deltaTime) override;
	};

	class ChiJumpDoubleDownState final : public ChiClipState
	{
	public:
		ChiJumpDoubleDownState();
		virtual void Tick(ChiStateContext& context, float deltaTime) override;
	};

	/// WeakAttack //////////////////////////////////////////////////////////////////////////////
	class ChiWeakAttackState : public ChiClipState
	{
	public:
		ChiWeakAttackState(ChiStateId stateId, ChiAnimationId animationId, ChiStateId nextWeakAttackState = ChiStateId::None, ChiStateId rightAttackState = ChiStateId::None);

		virtual void Tick(ChiStateContext& context, float deltaTime) override;

	private:
		ChiStateId _nextWeakAttackState = ChiStateId::None;
		ChiStateId _rightAttackState = ChiStateId::None;
	};

	class ChiWeak0AttackState final : public ChiWeakAttackState
	{
	public:
		ChiWeak0AttackState();
	};

	class ChiWeak1AttackState final : public ChiWeakAttackState
	{
	public:
		ChiWeak1AttackState();
	};

	class ChiWeak2AttackState final : public ChiWeakAttackState
	{
	public:
		ChiWeak2AttackState();
	};

	class ChiWeak3AttackState final : public ChiWeakAttackState
	{
	public:
		ChiWeak3AttackState();
	};

	class ChiWeakDashAttackState final : public ChiClipState
	{
	public:
		ChiWeakDashAttackState();
		virtual void Tick(ChiStateContext& context, float deltaTime) override;
	};

	/// StrongAttack //////////////////////////////////////////////////////////////////////////////
	class ChiStrongAttackState : public ChiClipState
	{
	public:
		ChiStrongAttackState(ChiStateId stateId, ChiAnimationId animationId, ChiStateId nextStrongAttackState = ChiStateId::None, ChiStateId leftAttackState = ChiStateId::None, ChiStateId autoNextState = ChiStateId::None);

		virtual void Tick(ChiStateContext& context, float deltaTime) override;

	private:
		ChiStateId _nextStrongAttackState = ChiStateId::None;
		ChiStateId _leftAttackState = ChiStateId::None;
		ChiStateId _autoNextState = ChiStateId::None;
	};

	class ChiStrong0_0AttackState final : public ChiStrongAttackState
	{
	public:
		ChiStrong0_0AttackState();
	};

	class ChiStrong0_1AttackState final : public ChiStrongAttackState
	{
	public:
		ChiStrong0_1AttackState();
	};

	class ChiStrong1AttackState final : public ChiStrongAttackState
	{
	public:
		ChiStrong1AttackState();
	};

	class ChiStrong2AttackState final : public ChiStrongAttackState
	{
	public:
		ChiStrong2AttackState();
	};

	class ChiStrongDashAttackState final : public ChiClipState
	{
	public:
		ChiStrongDashAttackState();
		virtual void Tick(ChiStateContext& context, float deltaTime) override;
	};

	/// BranchAttack //////////////////////////////////////////////////////////////////////////////
	class ChiBranchAttackState : public ChiClipState
	{
	public:
		ChiBranchAttackState(ChiStateId stateId, ChiAnimationId animationId, ChiStateId leftAttackState = ChiStateId::None, ChiStateId rightAttackState = ChiStateId::None, ChiStateId autoNextState = ChiStateId::None);

		virtual void Tick(ChiStateContext& context, float deltaTime) override;

	private:
		ChiStateId _leftAttackState = ChiStateId::None;
		ChiStateId _rightAttackState = ChiStateId::None;
		ChiStateId _autoNextState = ChiStateId::None;
	};

	class ChiStrongToWeak1AttackState final : public ChiBranchAttackState
	{
	public:
		ChiStrongToWeak1AttackState();
	};

	class ChiStrongToWeak2AttackState final : public ChiBranchAttackState
	{
	public:
		ChiStrongToWeak2AttackState();
	};

	class ChiWeakToStrong1AttackState final : public ChiBranchAttackState
	{
	public:
		ChiWeakToStrong1AttackState();
	};

	class ChiWeakToStrong2AttackState final : public ChiBranchAttackState
	{
	public:
		ChiWeakToStrong2AttackState();
	};

	class ChiDelayedWeak1AttackState final : public ChiBranchAttackState
	{
	public:
		ChiDelayedWeak1AttackState();
	};

	class ChiDelayedWeak2AttackState final : public ChiBranchAttackState
	{
	public:
		ChiDelayedWeak2AttackState();
	};

	/// AirAttack //////////////////////////////////////////////////////////////////////////////
	class ChiAirAttackState : public ChiClipState
	{
	public:
		ChiAirAttackState(ChiStateId stateId, ChiAnimationId animationId, ChiStateId nextAirAttackState = ChiStateId::None);

		virtual void Tick(ChiStateContext& context, float deltaTime) override;

	private:
		ChiStateId _nextAirAttackState = ChiStateId::None;
	};

	class ChiSky0AttackState final : public ChiAirAttackState
	{
	public:
		ChiSky0AttackState();
	};

	class ChiSky1AttackState final : public ChiAirAttackState
	{
	public:
		ChiSky1AttackState();
	};

	class ChiSky2AttackState final : public ChiAirAttackState
	{
	public:
		ChiSky2AttackState();
	};

	class ChiSky3AttackState final : public ChiAirAttackState
	{
	public:
		ChiSky3AttackState();
	};

	/// StumpAttack //////////////////////////////////////////////////////////////////////////////
	class ChiStump0AttackState final : public ChiClipState
	{
	public:
		ChiStump0AttackState();
		virtual void Tick(ChiStateContext& context, float deltaTime) override;
	};

	class ChiStump1AttackState final : public ChiClipState
	{
	public:
		ChiStump1AttackState();
		virtual void Tick(ChiStateContext& context, float deltaTime) override;
	};

	class ChiStump2AttackState final : public ChiClipState
	{
	public:
		ChiStump2AttackState();
		virtual void Tick(ChiStateContext& context, float deltaTime) override;
	};

	/// Damage //////////////////////////////////////////////////////////////////////////////
	class ChiDamageState : public ChiClipState
	{
	public:
		ChiDamageState(ChiStateId stateId, ChiAnimationId animationId, bool returnToBaseMotion);

		virtual void Tick(ChiStateContext& context, float deltaTime) override;

	private:
		bool _returnToBaseMotion = true;
	};

	class ChiWeakKnockbackDamageState final : public ChiDamageState
	{
	public:
		ChiWeakKnockbackDamageState();
	};

	class ChiStrongKnockbackDamageState final : public ChiDamageState
	{
	public:
		ChiStrongKnockbackDamageState();
	};

	class ChiDeadDamageState final : public ChiDamageState
	{
	public:
		ChiDeadDamageState();
	};

	/// Hibiki //////////////////////////////////////////////////////////////////////////////
	class ChiHibikiReadyState final : public ChiClipState
	{
	public:
		ChiHibikiReadyState();
		virtual void Tick(ChiStateContext& context, float deltaTime) override;
	};

	class ChiHibikiAttackState final : public ChiClipState
	{
	public:
		ChiHibikiAttackState();
		virtual void Tick(ChiStateContext& context, float deltaTime) override;
	};
}
