#pragma once

#include "EffectInstance.h"
#include "QamilState.h"

namespace gm
{
	struct AnimationNotifyEvent;

	class QamilStumpState final : public QamilState
	{
	public:
		QamilStateId GetStateId() const override { return QamilStateId::Stump; }
		void Enter(QamilStateContext& context) override;
		void Tick(QamilStateContext& context, float deltaTime) override;
		void Exit(QamilStateContext& context) override;

	private:
		void HandleAnimationNotify(QamilStateContext& context, const AnimationNotifyEvent& event);
		bool SpawnHitBox(QamilStateContext& context) const;
		bool SpawnEffect(QamilStateContext& context) const;
		bool PulseFloorShake(QamilStateContext& context) const;

		EventConnection _notifyConnection{};
	};

	class QamilSweepState final : public QamilState
	{
	public:
		QamilStateId GetStateId() const override { return QamilStateId::Sweep; }
		void Enter(QamilStateContext& context) override;
		void Tick(QamilStateContext& context, float deltaTime) override;
		void Exit(QamilStateContext& context) override;

	private:
		void HandleAnimationNotify(QamilStateContext& context, const AnimationNotifyEvent& event);
		bool SpawnWarning(QamilStateContext& context);
		bool SpawnEffect(QamilStateContext& context) const;
		void UpdateWarning(const QamilStateContext& context);
		bool SpawnHitBox(QamilStateContext& context) const;

		EventConnection _notifyConnection{};
		EffectInstance _warningEffect{};
		Vector3 _attackCenter{};
		bool _isClockwise = false;
	};
}
