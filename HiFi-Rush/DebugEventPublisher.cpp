#include "DebugEventPublisher.h"

#if GM_ENABLE_DEBUG_TOOLS

#include "Application.h"
#include "DebugInputHandler.h"
#include "IDebugRenderer.h"
#include "Renderer.h"
#include "UIManager.h"

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

		if (DebugInputHandler::IsTriggered(DebugInputType, KeyCode::F11))
		{
			IDebugRenderer& debugRenderer = APPLICATION.GetDebugRenderer();
			const bool isEnabled = debugRenderer.IsEnabled() == false;
			debugRenderer.SetEnabled(isEnabled);
			APPLICATION.GetUIManager().SetDebugWidgetsVisible(isEnabled);
		}

		if (DebugInputHandler::IsTriggered(DebugInputType, KeyCode::B))
		{
			Renderer& renderer = APPLICATION.GetRenderer();
			const Input& input = APPLICATION.GetInput();
			const bool isControlPressed = input.IsKeyRepeat(KeyCode::LeftCtrl) || input.IsKeyRepeat(KeyCode::RightCtrl);

			if (isControlPressed)
			{
				const bool isEnabled = renderer.IsFrustumCullingEnabled() == false;
				renderer.SetFrustumCullingEnabled(isEnabled);
			}
			else
			{
				const bool isEnabled = renderer.IsBoundingVolumeDebugDrawEnabled() == false;
				renderer.SetBoundingVolumeDebugDrawEnabled(isEnabled);
			}
		}
	}
}

#endif
