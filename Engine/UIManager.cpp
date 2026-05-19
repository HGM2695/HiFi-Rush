#include "UIManager.h"
#include "UserWidget.h"

namespace gm
{
	void UIManager::Tick(float deltaTime)
	{
		for (const auto& widget : _widgetList)
			widget->Tick(deltaTime);
	}

	void UIManager::Render()
	{
		for (const auto& widget : _widgetList)
			widget->Render();
	}

	void UIManager::ClearViewportWidgets()
	{
		_widgetList.clear();
	}
}
