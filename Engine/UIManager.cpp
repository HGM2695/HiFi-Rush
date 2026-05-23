#include "UIManager.h"
#include "Application.h"
#include "UserWidget.h"

namespace gm
{
	void UIManager::Tick(float deltaTime)
	{
		for (const auto& widget : _userWidgetList)
			widget->Tick(deltaTime);
	}

	void UIManager::Render()
	{
		const WidgetGeometry viewportGeometry
		{
			Vector2{},
			Vector2(static_cast<float>(APPLICATION.GetWidth()), static_cast<float>(APPLICATION.GetHeight()))
		};

		for (const auto& widget : _userWidgetList)
			widget->Render(viewportGeometry);
	}

	void UIManager::ClearViewportWidgets()
	{
		_userWidgetList.clear();
	}
}
