#pragma once

#include "ChiState.h"
#include "Event.h"

namespace gm
{
	struct AnimationNotifyEvent;
	class HitBoxComponent;
	class SkeletalAnimatorComponent;

	class ChiAttackState : public ChiState
	{
	public:
		ChiAttackState(ChiStateId stateId, ChiAnimationClipId animationClipId);

		virtual void Enter(ChiStateContext& context) override;
		virtual void Exit(ChiStateContext& context) override;

	protected:
		virtual bool UsesAutoTargeting() const { return true; }

		void	RestoreBasePlayRateAfterImpact(ChiStateContext& context);
		void	BufferMouseInput(ChiStateContext& context, float bufferStartBeat, bool allowWeak = true, bool allowStrong = true);
		bool	IsActionCancelAllowed(const ChiStateContext& context) const;
		bool	TryCancelAttack(ChiStateContext& context);
		bool	HasBufferedWeakInput() const { return _bufferedRhythmInput && _bufferedRhythmInput->type == RhythmInputType::WeakAttack; }
		bool	HasBufferedStrongInput() const { return _bufferedRhythmInput && _bufferedRhythmInput->type == RhythmInputType::StrongAttack; }
		void	ChangeStateWithBufferedInput(ChiStateContext& context, ChiStateId nextStateId) const;
		float	GetAnimationBeat(const ChiStateContext& context) const;

	private:
		void HandleTemporaryHitBoxNotify(ChiStateContext& context, const AnimationNotifyEvent& event);

		std::optional<RhythmJudgeResult>	_bufferedRhythmInput;
		EventConnection					_temporaryHitBoxNotifyConnection{};

		float	_animationSecondsPerBeat = 0.f;
		float	_basePlayRate = 1.f;
		float	_syncPlayRate = 1.f;
		float	_impactMarkerBeat = 0.f;
		bool	_hasRestoredBasePlayRate = true;
	};

	class ChiWeaponHitBoxAttackState : public ChiAttackState
	{
	public:
		ChiWeaponHitBoxAttackState(ChiStateId stateId, ChiAnimationClipId animationClipId, HitBoxComponent* weaponHitBox, int32 damage, float rehitInterval = 0.f);

		virtual void Enter(ChiStateContext& context) override;
		virtual void Exit(ChiStateContext& context) override;

	private:
		void HandleAnimationNotify(const AnimationNotifyEvent& event);

		HitBoxComponent*	_weaponHitBox = nullptr;
		EventConnection		_notifyConnection{};
		int32				_damage = 0;
		float				_rehitInterval = 0.f;
	};

	/// WeakAttack //////////////////////////////////////////////////////////////////////////////
	class ChiWeakAttackState : public ChiAttackState
	{
	public:
		ChiWeakAttackState(ChiStateId stateId, ChiAnimationClipId animationClipId, ChiStateId nextWeakAttackState = ChiStateId::None, ChiStateId nextStrongAttackState = ChiStateId::None);

		virtual void Tick(ChiStateContext& context, float deltaTime) override;

	private:
		ChiStateId _nextWeakAttackState = ChiStateId::None;
		ChiStateId _nextStrongAttackState = ChiStateId::None;
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

	class ChiWeakDashAttackState final : public ChiAttackState
	{
	public:
		ChiWeakDashAttackState();
		virtual void Tick(ChiStateContext& context, float deltaTime) override;
	};

	/// StrongAttack //////////////////////////////////////////////////////////////////////////////
	class ChiStrongAttackState : public ChiAttackState
	{
	public:
		ChiStrongAttackState(ChiStateId stateId, ChiAnimationClipId animationClipId, ChiStateId nextWeakAttackState = ChiStateId::None, ChiStateId nextStrongAttackState = ChiStateId::None, ChiStateId autoNextState = ChiStateId::None);

		virtual void Tick(ChiStateContext& context, float deltaTime) override;

	private:
		ChiStateId _nextWeakAttackState = ChiStateId::None;
		ChiStateId _nextStrongAttackState = ChiStateId::None;
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

	class ChiStrongDashAttackState final : public ChiAttackState
	{
	public:
		ChiStrongDashAttackState();
		virtual void Tick(ChiStateContext& context, float deltaTime) override;
	};

	/// BranchAttack //////////////////////////////////////////////////////////////////////////////
	class ChiBranchAttackState : public ChiAttackState
	{
	public:
		ChiBranchAttackState(ChiStateId stateId, ChiAnimationClipId animationClipId, ChiStateId nextAttackState = ChiStateId::None);

		virtual void Tick(ChiStateContext& context, float deltaTime) override;

	private:
		ChiStateId _nextAttackState = ChiStateId::None;
	};

	class ChiStrongToWeak1AttackState final : public ChiBranchAttackState
	{
	public:
		ChiStrongToWeak1AttackState();
	};

	class ChiStrongToWeak2AttackState final : public ChiWeaponHitBoxAttackState
	{
	public:
		explicit ChiStrongToWeak2AttackState(HitBoxComponent* weaponHitBox);
		virtual void Enter(ChiStateContext& context) override;
		virtual void Tick(ChiStateContext& context, float deltaTime) override;

	private:
		bool _hasCheckedBeatHit = false;
	};

	class ChiStrongToWeakBeatHitAttackState final : public ChiAttackState
	{
	public:
		ChiStrongToWeakBeatHitAttackState();
		virtual void Tick(ChiStateContext& context, float deltaTime) override;
	};

	class ChiWeakToStrong1AttackState final : public ChiBranchAttackState
	{
	public:
		ChiWeakToStrong1AttackState();
	};

	class ChiWeakToStrong2AttackState final : public ChiWeaponHitBoxAttackState
	{
	public:
		explicit ChiWeakToStrong2AttackState(HitBoxComponent* weaponHitBox);
		virtual void Tick(ChiStateContext& context, float deltaTime) override;
	};

	class ChiDelayedWeak1AttackState final : public ChiBranchAttackState
	{
	public:
		ChiDelayedWeak1AttackState();
	};

	class ChiDelayedWeak2AttackState final : public ChiAttackState
	{
	public:
		ChiDelayedWeak2AttackState();
		virtual void Tick(ChiStateContext& context, float deltaTime) override;
	};

	/// AirAttack //////////////////////////////////////////////////////////////////////////////
	class ChiAirAttackState : public ChiAttackState
	{
	public:
		ChiAirAttackState(ChiStateId stateId, ChiAnimationClipId animationClipId, ChiStateId nextAirAttackState = ChiStateId::None);

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
	class ChiStump0AttackState final : public ChiAttackState
	{
	public:
		ChiStump0AttackState();
		virtual void Tick(ChiStateContext& context, float deltaTime) override;
	};

	class ChiStump1AttackState final : public ChiAttackState
	{
	public:
		ChiStump1AttackState();
		virtual void Enter(ChiStateContext& context) override;
		virtual void Tick(ChiStateContext& context, float deltaTime) override;
		virtual void OnGroundContact(ChiStateContext& context) override;
	};

	class ChiStump2AttackState final : public ChiAttackState
	{
	public:
		ChiStump2AttackState();
		virtual void Tick(ChiStateContext& context, float deltaTime) override;
	};
}
