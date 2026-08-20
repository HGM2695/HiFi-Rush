#pragma once

#include "QamilState.h"

#include <optional>

namespace gm
{
	class QamilInactiveState final : public QamilState
	{
	public:
		QamilStateId GetStateId() const override { return QamilStateId::Inactive; }
		void Enter(QamilStateContext& context) override;
	};

	class QamilIdleState final : public QamilState
	{
	public:
		QamilStateId GetStateId() const override { return QamilStateId::Idle; }
		void Enter(QamilStateContext& context) override;
		void Tick(QamilStateContext& context, float deltaTime) override;

	private:
		std::optional<float> _nextActionBeat{};
	};

	class QamilDeadState final : public QamilState
	{
	public:
		QamilStateId GetStateId() const override { return QamilStateId::Dead; }
		void Enter(QamilStateContext& context) override;
	};
}
