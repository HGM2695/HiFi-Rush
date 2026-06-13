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
		void	ChangeDashStateByInput(ChiStateContext& context) const;
		bool	TryChangeAirAction(ChiStateContext& context, bool canDoubleJump) const;
		bool	TryChangeHibiki(ChiStateContext& context) const;
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
}
