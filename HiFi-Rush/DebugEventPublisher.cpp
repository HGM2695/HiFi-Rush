#include "DebugEventPublisher.h"
#include "DebugInputHandler.h"

namespace gm
{
	namespace
	{
		constexpr wchar_t DebugInputType[] = L"HiFiRush.DebugEvent";
	}

	void DebugEventPublisher::Initialize()
	{
		DebugInputHandler::RegisterDebugType(DebugInputType, true);
	}

	void DebugEventPublisher::Tick()
	{
		if (DebugInputHandler::IsTriggered(DebugInputType, KeyCode::F4))
		{
			DebugEvent event{};
			event.type = DebugEventType::Activate;
			OnDebugEvent.Publish(event);
		}
		else if (DebugInputHandler::IsTriggered(DebugInputType, KeyCode::F9))
		{
			DebugEvent event{};
			event.type = DebugEventType::Reset;
			OnDebugEvent.Publish(event);
		}
	}
}
