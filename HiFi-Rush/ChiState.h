#pragma once

#include "AnimationTypes.h"
#include "ChiAnimationTypes.h"
#include "ChiStateContext.h"
#include "ChiStateTypes.h"
#include "MathTypes.h"

#include <optional>

namespace gm
{
	struct NavigationGroundLostEvent;

	class ChiState
	{
	public:
		ChiState(ChiStateId stateId, ChiAnimationClipId animationClipId, bool isLoop = false);
		ChiState(ChiStateId stateId, ChiAnimationClipId animationClipId, const AnimationPlayOption& playOption);
		virtual ~ChiState() = default;

		ChiStateId GetStateId() const { return _stateId; }
		virtual void Enter(ChiStateContext& context);
		virtual void Tick(ChiStateContext& context, float deltaTime) {}
		virtual void OnGroundContact(ChiStateContext& context);
		virtual void OnGroundLost(ChiStateContext& context, const NavigationGroundLostEvent& event);
		virtual void Exit(ChiStateContext& context) {}

	protected:
		void	PlayAnimation(ChiStateContext& context, ChiAnimationClipId animationClipId, const AnimationPlayOption& playOption) const;
		bool	IsMoveInputPressed(const ChiStateContext& context) const;
		void	ReturnToIdleOrRun(ChiStateContext& context) const;
		bool	IsAnimationCompleted(const ChiStateContext& context) const;
		void	ChangeDashStateByInput(ChiStateContext& context, const RhythmJudgeResult* judgeResult = nullptr) const;
		bool	TryChangeGroundAction(ChiStateContext& context) const;
		bool	TryChangeAirDashOrStump(ChiStateContext& context) const;
		bool	TryChangeAirAction(ChiStateContext& context, bool canDoubleJump, std::optional<float> weakAttackStartBeat = std::nullopt) const;
		bool	TryChangeHibiki(ChiStateContext& context) const;
		void	InitializeBeatTiming(ChiStateContext& context, float blendDuration);
		float	GetStateElapsedBeat(const ChiStateContext& context) const;
		float	GetElapsedBeatAfterBlend(const ChiStateContext& context) const;
		bool	IsBlendCompleted(const ChiStateContext& context) const;
		ChiAnimationClipId GetAnimationClipId() const { return _animationClipId; }

	private:
		ChiStateId			_stateId = ChiStateId::None;
		ChiAnimationClipId	_animationClipId = ChiAnimationClipId::Idle;
		AnimationPlayOption _playOption{};
		float				_stateStartBeat = 0.f;
		float				_blendEndBeat = 0.f;
	};
}
