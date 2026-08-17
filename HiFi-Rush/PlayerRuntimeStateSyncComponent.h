#pragma once

#include "Component.h"
#include "Event.h"

namespace gm
{
	class HealthComponent;
	class ReverbComponent;
	struct HealthChangedEvent;
	struct ReverbChangedEvent;
	struct PlayerRuntimeState;

	class PlayerRuntimeStateSyncComponent final : public Component
	{
	public:
		explicit PlayerRuntimeStateSyncComponent(PlayerRuntimeState& runtimeState);

	protected:
		void OnInitialize() override;

	private:
		void HandleHealthChanged(const HealthChangedEvent& event);
		void HandleReverbChanged(const ReverbChangedEvent& event);

		PlayerRuntimeState&		_runtimeState;
		HealthComponent*		_healthComponent = nullptr;
		ReverbComponent*		_reverbComponent = nullptr;
		EventConnection			_healthChangedConnection{};
		EventConnection			_reverbChangedConnection{};
	};
}
