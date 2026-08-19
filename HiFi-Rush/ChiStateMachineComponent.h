#pragma once

#include "ChiAnimationTypes.h"
#include "ChiStateContext.h"
#include "ChiStateTypes.h"
#include "Component.h"
#include "Event.h"

#include <memory>
#include <unordered_map>

namespace gm
{
	struct HitEvent;
	struct ChiStateContext;
	class ChiState;
	class ChiMoveComponent;
	class HealthComponent;
	class HitBoxComponent;
	class PlayerTargetingComponent;
	class ReverbComponent;
	class SkeletalAnimatorComponent;
	struct NavigationGroundLostEvent;

	class ChiStateMachineComponent : public Component
	{
	public:
		explicit ChiStateMachineComponent(HitBoxComponent* weaponHitBox = nullptr);
		virtual ~ChiStateMachineComponent();

		virtual TickGroup GetTickGroup() const override { return TickGroup::GameLogic; }

		void					ChangeState(ChiStateId nextStateId);
		void					ChangeState(ChiStateId nextStateId, float blendDuration);
		void					ChangeState(ChiStateId nextStateId, const RhythmJudgeResult& rhythmInput);
		void					ChangeState(ChiStateId nextStateId, float blendDuration, const RhythmJudgeResult& rhythmInput);
		ChiStateId				GetCurrentStateId() const { return _currentStateId; }
		void					SetInputEnabled(bool enabled) { _inputEnabled = enabled; }
		bool					IsInputEnabled() const { return _inputEnabled; }

		EventPublisher<ChiStateMachineComponent, RhythmJudgeResult> OnRhythmActionStarted;

	protected:
		virtual void			OnInitialize() override;
		virtual void			OnTick(float deltaTime) override;

	private:
		void					RegisterAnimationClips();
		void					RegisterStates();
		void					OnGroundContact();
		void					OnGroundLost(const NavigationGroundLostEvent& event);
		void					OnDamaged(const HitEvent& event);
		RhythmJudgeResult		JudgeRhythmInput(RhythmInputType inputType);
		bool					ChangeStateInternal(ChiStateId nextStateId);
		void					ResetTransitionOptions();
		ChiState*				FindState(ChiStateId stateId) const;

	private:
		std::unordered_map<ChiStateId, std::unique_ptr<ChiState>> _states;

		ChiStateId					_currentStateId = ChiStateId::None;
		ChiStateContext				_context{};
		EventConnection				_groundContactConnection{};
		EventConnection				_groundLostConnection{};
		EventConnection				_damagedConnection{};
		EventConnection				_weaponHitConnection{};

		SkeletalAnimatorComponent*		_animatorComponent = nullptr;
		ChiMoveComponent*				_moveComponent = nullptr;
		PlayerTargetingComponent*		_targetingComponent = nullptr;
		HealthComponent*				_healthComponent = nullptr;
		ReverbComponent*				_reverbComponent = nullptr;
		HitBoxComponent*				_weaponHitBox = nullptr;
		bool							_inputEnabled = true;
	};
}
