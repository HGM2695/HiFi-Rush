#pragma once

#include "Component.h"
#include "Event.h"

namespace gm
{
	inline constexpr wchar_t QamilPhase2TriggerId[] = L"Qamil.Phase2";
	inline constexpr wchar_t QamilPhase3TriggerId[] = L"Qamil.Phase3";

	class QamilStateMachineComponent;
	class TriggerSystem;
	struct QamilPhaseChangedEvent;

	class QamilPhaseTriggerComponent final : public Component
	{
	protected:
		void OnInitialize() override;

	private:
		void HandlePhaseChanged(const QamilPhaseChangedEvent& event);

		QamilStateMachineComponent*	_stateMachine = nullptr;
		TriggerSystem*				_triggerSystem = nullptr;
		EventConnection				_phaseChangedConnection{};
	};
}
