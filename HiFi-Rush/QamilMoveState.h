#pragma once

#include "QamilState.h"

namespace gm
{
	class QamilMoveState final : public QamilState
	{
	public:
		QamilStateId GetStateId() const override { return QamilStateId::Move; }
		void Enter(QamilStateContext& context) override;
		void Tick(QamilStateContext& context, float deltaTime) override;

	private:
		enum class Phase
		{
			Start,
			Move,
			End,
		};

		void BeginMove(QamilStateContext& context);
		void BeginEnd(QamilStateContext& context);

		Phase _phase = Phase::Start;
		float _startRotationY = 0.f;
		float _targetRotationY = 0.f;
		bool _isClockwise = false;
	};
}
