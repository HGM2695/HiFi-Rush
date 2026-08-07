#pragma once

#include "Component.h"
#include "WeakGameObjectPtr.h"
#if GM_ENABLE_DEBUG_TOOLS
#include "Event.h"
#endif

#include <vector>

namespace gm
{
	class BeatSystem;
	class IBeatTriggerAction;

	class BeatTriggerSequenceComponent : public Component
	{
	public:
		explicit BeatTriggerSequenceComponent(const BeatSystem& beatSystem);

		void AddAction(float beatOffset, const WeakGameObjectPtr& actionOwner, IBeatTriggerAction& action);
		void Activate();
		void Reset();

	protected:
		void OnInitialize() override;

	private:
		struct SequenceEntry
		{
			WeakGameObjectPtr	actionOwner{};
			IBeatTriggerAction*	action = nullptr;
			float				beatOffset = 0.f;
		};

	private:
		const BeatSystem&			_beatSystem;
		std::vector<SequenceEntry>	_entries{};
#if GM_ENABLE_DEBUG_TOOLS
		EventConnection				_debugEventConnection{};
#endif
		bool						_isActivated = false;
	};
}
