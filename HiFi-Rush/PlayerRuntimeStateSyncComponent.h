#pragma once

#include "Component.h"
#include "Event.h"

namespace gm
{
	class HealthComponent;
	struct HealthChangedEvent;
	struct PlayerRuntimeState;

	class PlayerRuntimeStateSyncComponent final : public Component
	{
	public:
		explicit PlayerRuntimeStateSyncComponent(PlayerRuntimeState& runtimeState);

	protected:
		void OnInitialize() override;

	private:
		void HandleHealthChanged(const HealthChangedEvent& event);

		PlayerRuntimeState&		_runtimeState;
		HealthComponent*		_healthComponent = nullptr;
		EventConnection			_healthChangedConnection{};
	};
}
