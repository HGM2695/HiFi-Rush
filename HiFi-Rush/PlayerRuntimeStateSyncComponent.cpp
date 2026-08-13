#include "PlayerRuntimeStateSyncComponent.h"

#include "CombatTypes.h"
#include "GameObject.h"
#include "HealthComponent.h"
#include "PlayerRuntimeState.h"

namespace gm
{
	PlayerRuntimeStateSyncComponent::PlayerRuntimeStateSyncComponent(PlayerRuntimeState& runtimeState)
		: _runtimeState(runtimeState)
	{}

	void PlayerRuntimeStateSyncComponent::OnInitialize()
	{
		GM_ASSERT_RETURN(_runtimeState.maxHealth > 0, "Player Runtime State의 Max Health는 0보다 커야 합니다.");
		GM_ASSERT_RETURN(_runtimeState.currentHealth >= 0 && _runtimeState.currentHealth <= _runtimeState.maxHealth, "Player Runtime State의 Current Health가 유효하지 않습니다. current=%d, max=%d", _runtimeState.currentHealth, _runtimeState.maxHealth);

		_healthComponent = GetOwner().GetComponent<HealthComponent>();
		GM_ASSERT_RETURN(_healthComponent, "PlayerRuntimeStateSyncComponent에는 HealthComponent가 필요합니다.");

		_healthComponent->SetHealth(_runtimeState.currentHealth);
		_healthComponent->OnHealthChanged.Subscribe(_healthChangedConnection,
			[this](const HealthChangedEvent& event)
			{
				HandleHealthChanged(event);
			});
	}

	void PlayerRuntimeStateSyncComponent::HandleHealthChanged(const HealthChangedEvent& event)
	{
		_runtimeState.maxHealth = event.maxHealth;
		_runtimeState.currentHealth = event.currentHealth;
	}
}
