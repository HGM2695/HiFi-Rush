#include "DebugEventPublisher.h"

#if GM_ENABLE_DEBUG_TOOLS

#include "Application.h"
#include "DebugInputHandler.h"
#include "IDebugRenderer.h"
#include "NavMeshSystem.h"
#include "PhysicsSystem.h"
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

		if (DebugInputHandler::IsTriggered(DebugInputType, KeyCode::I))
		{
			Renderer& renderer = APPLICATION.GetRenderer();
			const Input& input = APPLICATION.GetInput();
			const bool isControlPressed = input.IsKeyRepeat(KeyCode::LeftCtrl) || input.IsKeyRepeat(KeyCode::RightCtrl);

			if (isControlPressed)
			{
				const bool isEnabled = renderer.IsStaticMeshInstancingEnabled() == false;
				renderer.SetStaticMeshInstancingEnabled(isEnabled);
			}
		}

		if (DebugInputHandler::IsTriggered(DebugInputType, KeyCode::C))
		{
			PhysicsSystem& physicsSystem = APPLICATION.GetPhysicsSystem();
			const bool isEnabled = physicsSystem.IsCollider3DDebugDrawEnabled();
			physicsSystem.SetCollider3DDebugDrawEnabled(!isEnabled);
		}

		if (DebugInputHandler::IsTriggered(DebugInputType, KeyCode::N))
		{
			NavMeshSystem& navMeshSystem = APPLICATION.GetPhysicsSystem().GetNavMeshSystem();
			const bool isEnabled = navMeshSystem.IsDebugDrawEnabled();
			navMeshSystem.SetDebugDrawEnabled(!isEnabled);
		}

	}
}

#endif
