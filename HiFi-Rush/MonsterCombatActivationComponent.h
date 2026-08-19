#pragma once

#include "Component.h"
#include "TriggerBinding.h"

#include <string>
#include <vector>

namespace gm
{
	class BeatSystem;
	class Collider3DComponent;
	class MonsterCombatComponent;
	class MonsterStateMachineComponent;

	struct MonsterCombatActivationDesc
	{
		std::wstring	triggerId{};
		float			beatOffset = 0.f;
	};

	class MonsterCombatActivationComponent final : public Component
	{
	public:
		MonsterCombatActivationComponent(const BeatSystem& beatSystem, MonsterCombatActivationDesc desc);

	protected:
		void OnInitialize() override;
		void OnTick(float deltaTime) override;

	private:
		struct ColliderState
		{
			Collider3DComponent*	collider = nullptr;
			bool					wasEnabled = false;
		};

	private:
		enum class ActivationState
		{
			Inactive,
			Scheduled,
			Active,
		};

	private:
		void Schedule(float activationBeat);
		void ResetActivation();
		void ActivateCombat();
		void SetCombatActive(bool isActive);

	private:
		const BeatSystem&				_beatSystem;
		MonsterCombatActivationDesc	_desc{};
		TriggerBinding					_triggerBinding{};
		MonsterCombatComponent*			_combatComponent = nullptr;
		MonsterStateMachineComponent*	_stateMachine = nullptr;
		std::vector<ColliderState>		_colliderStates{};
		float							_activationBeat = 0.f;
		ActivationState					_state = ActivationState::Inactive;
	};
}
