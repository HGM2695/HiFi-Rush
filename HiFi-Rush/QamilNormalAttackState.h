#pragma once

#include "QamilAttackAim.h"
#include "QamilState.h"

namespace gm
{
	struct AnimationNotifyEvent;
	class Collider3DComponent;

	class QamilNormalAttackState final : public QamilState
	{
	public:
		QamilStateId GetStateId() const override { return QamilStateId::NormalAttack; }
		void Enter(QamilStateContext& context) override;
		void Tick(QamilStateContext& context, float deltaTime) override;
		void Exit(QamilStateContext& context) override;

	private:
		enum class Phase
		{
			Attack,
			Wait,
			Restore,
		};

		void SelectAttack(QamilStateContext& context);
		void BeginAttack(QamilStateContext& context);
		void BeginWait(QamilStateContext& context);
		void BeginRestore(QamilStateContext& context);
		void UpdateAimCorrection(QamilStateContext& context);
		void LockAimCorrection(QamilStateContext& context);
		void UpdateHandCollisionBlocking();
		void SetHandCollisionBlocking(bool isBlocking);
		void HandleAnimationNotify(QamilStateContext& context, const AnimationNotifyEvent& event);
		bool SpawnHitBox(QamilStateContext& context);

		EventConnection _notifyConnection{};
		QamilAnimationId _attackAnimationId = QamilAnimationId::NormalLeftNear;
		QamilAnimationId _waitAnimationId = QamilAnimationId::NormalLeftNearWait;
		QamilAnimationId _restoreAnimationId = QamilAnimationId::NormalLeftNearRestore;
		QamilAttackAim _attackAim{};
		Vector3 _attackReferencePosition{};
		Vector3 _targetAimOffset{};
		Vector3 _restoreStartAimOffset{};
		Collider3DComponent* _handCollider = nullptr;
		float _aimStartTime = 0.f;
		float _hitTime = 0.f;
		Phase _phase = Phase::Attack;
		uint32 _waitCycleCount = 0;
		uint32 _handCollisionDelayFrames = 0;
		bool _isAimLocked = false;
	};
}
