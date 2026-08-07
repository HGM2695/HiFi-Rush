#include "UIManager.h"
#include "Application.h"
#include "UserWidget.h"

namespace gm
{
	void UIManager::Tick(float deltaTime)
	{
		for (const auto& widget : _viewportWidgetList)
			widget->Tick(deltaTime);

#if GM_ENABLE_DEBUG_TOOLS
		for (const auto& widget : _debugWidgetList)
			widget->Tick(deltaTime);
#endif
	}

	void UIManager::Render()
	{
		const WidgetGeometry viewportGeometry
		{
			Vector2{},
			Vector2(static_cast<float>(APPLICATION.GetWidth()), static_cast<float>(APPLICATION.GetHeight()))
		};

		for (const auto& widget : _viewportWidgetList)
			widget->Render(viewportGeometry);

#if GM_ENABLE_DEBUG_TOOLS
		if (_areDebugWidgetsVisible)
		{
			for (const auto& widget : _debugWidgetList)
				widget->Render(viewportGeometry);
		}
#endif
	}

	void UIManager::ClearViewportWidgets()
	{
		_viewportWidgetList.clear();
	}

	void UIManager::ClearDebugWidgets()
	{
#if GM_ENABLE_DEBUG_TOOLS
		_debugWidgetList.clear();
#endif
	}

	void UIManager::ClearAllWidgets()
	{
		ClearViewportWidgets();
		ClearDebugWidgets();
	}
}
