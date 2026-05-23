#include "UserWidget.h"
#include "Widget.h"

namespace gm
{
	UserWidget::UserWidget() = default;
	UserWidget::~UserWidget() = default;

	void UserWidget::Initialize()
	{
		if (_isInitialized)
			return;

		_rootWidget = BuildWidgetTree();
		GM_ASSERT_RETURN(_rootWidget, "Root Widget이 없습니다.");

		OnInitialize();
		_rootWidget->Initialize();
		_isInitialized = true;
	}

	void UserWidget::Tick(float deltaTime)
	{
		if (_isInitialized == false || _isVisible == false)
			return;

		OnTick(deltaTime);

		if (_rootWidget)
			_rootWidget->Tick(deltaTime);
	}

	void UserWidget::Render(const WidgetGeometry& rootGeometry)
	{
		if (_isInitialized == false || _isVisible == false || _rootWidget == nullptr)
			return;

		_rootWidget->Render(rootGeometry);
	}
}
