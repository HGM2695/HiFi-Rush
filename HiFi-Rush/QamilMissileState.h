#pragma once

#include "QamilState.h"

#include <vector>

namespace gm
{
	class QamilMissileState final : public QamilState
	{
	public:
		QamilStateId GetStateId() const override { return QamilStateId::Missile; }
		void Enter(QamilStateContext& context) override;
		void Tick(QamilStateContext& context, float deltaTime) override;
		void Exit(QamilStateContext& context) override;

	private:
		bool PlaySequenceAnimation(QamilStateContext& context, QamilAnimationId animationId, float playRateScale = 1.f);
		bool SpawnMissile(QamilStateContext& context, uint32 missileIndex);
		Vector3 CreateWarningPosition(const Vector3& arenaCenter, uint32 missileIndex) const;
		bool GetSocketPosition(const QamilStateContext& context, const wchar_t* socketName, Vector3& outPosition) const;

		std::vector<WeakGameObjectPtr> _missiles{};
		QamilAnimationId _currentAnimationId = QamilAnimationId::MissileReady;
	};
}
