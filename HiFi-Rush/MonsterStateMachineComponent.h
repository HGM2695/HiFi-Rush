#pragma once

#include "Component.h"
#include "Event.h"
#include "MonsterStateContext.h"
#include "MonsterStateTypes.h"

#include <memory>
#include <unordered_map>

namespace gm
{
	class MonsterState;
	struct HitEvent;

	class MonsterStateMachineComponent : public Component
	{
	public:
		MonsterStateMachineComponent();
		~MonsterStateMachineComponent() override;

		TickGroup GetTickGroup() const override { return TickGroup::GameLogic; }

		bool			RegisterState(std::unique_ptr<MonsterState> state);
		bool			HasState(MonsterStateId stateId) const;
		bool			ChangeState(MonsterStateId nextStateId, bool restart = false);
		void			SetInitialState(MonsterStateId stateId) { _initialStateId = stateId; }
		MonsterStateId	GetInitialStateId() const { return _initialStateId; }
		MonsterStateId	GetCurrentStateId() const { return _currentStateId; }

	protected:
		void			OnInitialize() override;
		void			OnTick(float deltaTime) override;

	private:
		void			OnDamaged(const HitEvent& event);
		MonsterState*	FindState(MonsterStateId stateId) const;

	private:
		std::unordered_map<MonsterStateId, std::unique_ptr<MonsterState>> _states{};

		MonsterStateContext	_context{};
		MonsterStateId		_initialStateId = MonsterStateId::None;
		MonsterStateId		_currentStateId = MonsterStateId::None;
		EventConnection		_damagedConnection{};
	};
}
