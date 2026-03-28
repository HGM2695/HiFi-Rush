#include "UIManager.h"
#include "UserWidget.h"

namespace gm
{
	void UIManager::Update()
	{
		for (const auto& widget : _widgetList)
			widget->Update();
	}

	void UIManager::Render(HDC hDC)
	{
		for (const auto& widget : _widgetList)
			widget->Render(hDC);
	}

	void UIManager::ClearViewportWidgets()
	{
		_widgetList.clear();
	}
}
