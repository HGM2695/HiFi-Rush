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
	struct ChiStateContext;
	class ChiState;
	class ChiMoveComponent;
	class SkeletalAnimatorComponent;
	struct NavigationGroundContactEvent;
	struct NavigationGroundLostEvent;

	class ChiStateMachineComponent : public Component
	{
	public:
		ChiStateMachineComponent();
		virtual ~ChiStateMachineComponent();

		virtual TickGroup GetTickGroup() const override { return TickGroup::GameLogic; }

		void					ChangeState(ChiStateId nextStateId);
		ChiStateId				GetCurrentStateId() const { return _currentStateId; }

	protected:
		virtual void			OnInitialize() override;
		virtual void			OnTick(float deltaTime) override;

	private:
		void					RegisterAnimationClips();
		void					RegisterStates();
		void					OnGroundContact(const NavigationGroundContactEvent& event);
		void					OnGroundLost(const NavigationGroundLostEvent& event);
		ChiState*				FindState(ChiStateId stateId) const;

	private:
		std::unordered_map<ChiStateId, std::unique_ptr<ChiState>> _states;

		ChiStateId					_currentStateId = ChiStateId::None;
		ChiStateContext				_context{};
		EventConnection				_groundContactConnection{};
		EventConnection				_groundLostConnection{};

		SkeletalAnimatorComponent*	_animatorComponent = nullptr;
		ChiMoveComponent*			_moveComponent = nullptr;
	};
}
