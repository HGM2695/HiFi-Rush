#pragma once

#if GM_ENABLE_DEBUG_TOOLS

#include "Event.h"

namespace gm
{
	enum class DebugEventType
	{
		Activate,
		Reset,
	};

	struct DebugEvent : EventType
	{
		DebugEventType type = DebugEventType::Activate;
	};

	class DebugEventPublisher
	{
	public:
		void Initialize();
		void Tick();

		EventPublisher<DebugEventPublisher, DebugEvent> OnDebugEvent;
	};
}

#endif
