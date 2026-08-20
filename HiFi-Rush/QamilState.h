#pragma once

#include "QamilAnimationTypes.h"
#include "QamilStateContext.h"
#include "QamilStateTypes.h"

namespace gm
{
	inline constexpr float QamilAnimationTicksPerBeat = 15.f;
	inline constexpr float QamilDefaultBlendDuration = 0.15f;

	class QamilState
	{
	public:
		virtual ~QamilState() = default;

		virtual QamilStateId GetStateId() const = 0;
		virtual void Enter(QamilStateContext& context) {}
		virtual void Tick(QamilStateContext& context, float deltaTime) {}
		virtual void Exit(QamilStateContext& context) {}

	protected:
		bool PlayAnimation(QamilStateContext& context, QamilAnimationId animationId, bool isLoop, float blendDuration = QamilDefaultBlendDuration) const;
		bool PlayBeatSyncedAnimation(QamilStateContext& context, QamilAnimationId animationId, bool isLoop, float blendDuration = QamilDefaultBlendDuration) const;
		bool IsAnimationCompleted(const QamilStateContext& context) const;
		float GetAnimationBeat(const QamilStateContext& context) const;
		float GetBasePlayRate(const QamilStateContext& context) const;
		Vector3 GetCurrentPlatformPosition(const QamilStateContext& context) const;
		bool IsTargetInCurrentPlatformArea(const QamilStateContext& context) const;
		bool SelectClockwiseDirection(const QamilStateContext& context) const;
	};
}
