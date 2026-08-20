#pragma once

#include "QamilAttackAim.h"
#include "QamilState.h"

namespace gm
{
	class Collider3DComponent;

	class QamilLaserState final : public QamilState
	{
	public:
		QamilStateId GetStateId() const override { return QamilStateId::Laser; }
		void Enter(QamilStateContext& context) override;
		void Tick(QamilStateContext& context, float deltaTime) override;
		void Exit(QamilStateContext& context) override;

	private:
		enum class Phase
		{
			Attack,
			Rebound,
			Restore,
		};

		bool ResolveHandColliders(QamilStateContext& context);
		bool SpawnLaserHitBox(QamilStateContext& context, uint32 laserIndex) const;
		void SetReboundCollision(bool isEnabled);

		Collider3DComponent* _leftHandCollider = nullptr;
		Collider3DComponent* _rightHandCollider = nullptr;
		float _leftHandBaseRadius = 0.f;
		float _rightHandBaseRadius = 0.f;
		Phase _phase = Phase::Attack;
		uint32 _nextLaserIndex = 0;
		uint32 _reboundCycleCount = 0;
		bool _isReboundCollisionEnabled = false;
	};

	class QamilChainState final : public QamilState
	{
	public:
		QamilStateId GetStateId() const override { return QamilStateId::Chain; }
		void Enter(QamilStateContext& context) override;
		void Tick(QamilStateContext& context, float deltaTime) override;
		void Exit(QamilStateContext& context) override;

	private:
		enum class Phase
		{
			Opening,
			ReadyTransition,
			Strike,
			ReturnToReady,
			FinisherWait,
			FinisherToMooyaho,
			Restore,
		};

		bool SelectNextAttack(QamilStateContext& context);
		bool BeginReadyTransition(QamilStateContext& context);
		bool BeginStrike(QamilStateContext& context);
		bool BeginReturnToReady(QamilStateContext& context);
		bool BeginFinisherWait(QamilStateContext& context);
		bool BeginMooyaho(QamilStateContext& context);
		bool SpawnHitBox(QamilStateContext& context);
		void UpdateAimCorrection(QamilStateContext& context);
		void UpdateHandCollisionBlocking(QamilStateContext& context);
		void SetHandCollisionBlocking(bool isBlocking);

		QamilAttackAim _attackAim{};
		Vector3 _transitionStartAimOffset{};
		Vector3 _targetAimOffset{};
		Collider3DComponent* _blockingHandCollider = nullptr;
		Phase _phase = Phase::Opening;
		uint32 _attackSettingIndex = 0;
		uint32 _attackCount = 0;
		uint32 _waitCycleCount = 0;
		uint32 _handCollisionDelayFrames = 0;
		bool _hasSpawnedFinisherHitBox = false;
	};
}
