#pragma once

#include "Component.h"
#include "Event.h"

#include <memory>
#include <string>
#include <vector>

namespace gm
{
	class HurtBoxComponent;
	class TriggerSequenceSystem;
	struct HitEvent;

	class HitTriggeredEnvironmentComponent final : public Component
	{
	public:
		explicit HitTriggeredEnvironmentComponent(const std::wstring& sequenceId);

	protected:
		void OnInitialize() override;

	private:
		void HandleHurt(const HitEvent& event);

		std::wstring					_sequenceId{};
		TriggerSequenceSystem*			_triggerSequenceSystem = nullptr;
		std::vector<std::unique_ptr<EventConnection>> _hurtConnections{};
	};
}
