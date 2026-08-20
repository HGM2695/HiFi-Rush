#pragma once

#include "Component.h"
#include "Event.h"
#include "QamilStateContext.h"
#include "QamilStateTypes.h"

#include <memory>
#include <unordered_map>

namespace gm
{
	class QamilState;
	struct HealthChangedEvent;
	struct HitEvent;

	class QamilStateMachineComponent final : public Component
	{
	public:
		QamilStateMachineComponent();
		~QamilStateMachineComponent() override;

		TickGroup GetTickGroup() const override { return TickGroup::GameLogic; }

		bool			StartCombat();
		bool			StopCombat();
		bool			ChangeState(QamilStateId nextStateId, bool restart = false);
		QamilStateId	SelectNextAttackState();
		QamilStateId	SelectNextWideAttackState();

		QamilStateId	GetCurrentStateId() const { return _currentStateId; }
		QamilPhase		GetCurrentPhase() const { return _currentPhase; }
		bool			IsCombatActive() const { return _isCombatActive; }

		EventPublisher<QamilStateMachineComponent, QamilPhaseChangedEvent>	OnPhaseChanged;
		EventPublisher<QamilStateMachineComponent, QamilDefeatedEvent>		OnDefeated;

	protected:
		void OnInitialize() override;
		void OnTick(float deltaTime) override;

	private:
		bool		RegisterState(std::unique_ptr<QamilState> state);
		bool		RegisterStates();
		bool		ResolveTarget();
		QamilState* FindState(QamilStateId stateId) const;
		QamilPhase CalculatePhase(int32 health, int32 maxHealth) const;
		void		HandleHealthChanged(const HealthChangedEvent& event);
		void		HandleDeath(const HitEvent& event);

		std::unordered_map<QamilStateId, std::unique_ptr<QamilState>>	_states{};
		QamilStateContext												_context{};
		QamilStateId													_currentStateId = QamilStateId::None;
		QamilStateId													_previousAttackStateId = QamilStateId::None;
		QamilPhase														_currentPhase = QamilPhase::Phase1;
		EventConnection													_healthChangedConnection{};
		EventConnection													_deathConnection{};
		bool															_isCombatActive = false;
		uint32														_wideAttackIndex = 0;
	};
}
