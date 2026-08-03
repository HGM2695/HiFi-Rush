#pragma once

#include "ChiState.h"

namespace gm
{
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
		virtual void OnGroundContact(ChiStateContext& context, const NavigationGroundContactEvent& event) override;
	};

	class ChiStump2AttackState final : public ChiClipState
	{
	public:
		ChiStump2AttackState();
		virtual void Tick(ChiStateContext& context, float deltaTime) override;
	};
}
