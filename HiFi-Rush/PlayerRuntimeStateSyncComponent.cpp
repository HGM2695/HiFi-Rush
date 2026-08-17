#include "PlayerRuntimeStateSyncComponent.h"

#include "CombatTypes.h"
#include "GameObject.h"
#include "HealthComponent.h"
#include "PlayerRuntimeState.h"
#include "ReverbComponent.h"

namespace gm
{
	PlayerRuntimeStateSyncComponent::PlayerRuntimeStateSyncComponent(PlayerRuntimeState& runtimeState)
		: _runtimeState(runtimeState)
	{}

	void PlayerRuntimeStateSyncComponent::OnInitialize()
	{
		GM_ASSERT_RETURN(_runtimeState.maxHealth > 0, "Player Runtime State의 Max Health는 0보다 커야 합니다.");
		GM_ASSERT_RETURN(_runtimeState.currentHealth >= 0 && _runtimeState.currentHealth <= _runtimeState.maxHealth, "Player Runtime State의 Current Health가 유효하지 않습니다. current=%d, max=%d", _runtimeState.currentHealth, _runtimeState.maxHealth);
		GM_ASSERT_RETURN(_runtimeState.maxReverb > 0.f, "Player Runtime State의 Max Reverb는 0보다 커야 합니다.");
		GM_ASSERT_RETURN(_runtimeState.currentReverb >= 0.f && _runtimeState.currentReverb <= _runtimeState.maxReverb, "Player Runtime State의 Current Reverb가 유효하지 않습니다. current=%f, max=%f", _runtimeState.currentReverb, _runtimeState.maxReverb);

		_healthComponent = GetOwner().GetComponent<HealthComponent>();
		GM_ASSERT_RETURN(_healthComponent, "PlayerRuntimeStateSyncComponent에는 HealthComponent가 필요합니다.");
		_reverbComponent = GetOwner().GetComponent<ReverbComponent>();
		GM_ASSERT_RETURN(_reverbComponent, "PlayerRuntimeStateSyncComponent에는 ReverbComponent가 필요합니다.");

		_healthComponent->SetHealth(_runtimeState.currentHealth);
		_reverbComponent->SetReverb(_runtimeState.currentReverb);
		_healthComponent->OnHealthChanged.Subscribe(_healthChangedConnection,
			[this](const HealthChangedEvent& event)
			{
				HandleHealthChanged(event);
			});
		_reverbComponent->OnReverbChanged.Subscribe(_reverbChangedConnection,
			[this](const ReverbChangedEvent& event)
			{
				HandleReverbChanged(event);
			});
	}

	void PlayerRuntimeStateSyncComponent::HandleHealthChanged(const HealthChangedEvent& event)
	{
		_runtimeState.maxHealth = event.maxHealth;
		_runtimeState.currentHealth = event.currentHealth;
	}

	void PlayerRuntimeStateSyncComponent::HandleReverbChanged(const ReverbChangedEvent& event)
	{
		_runtimeState.maxReverb = event.maxReverb;
		_runtimeState.currentReverb = event.currentReverb;
	}
}
