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
		NativeConstruct();
		_isInitialized = true;
	}

	void UserWidget::Tick(float deltaTime)
	{
		if (_isInitialized == false || _isVisible == false)
			return;

		NativeTick(deltaTime);

		if (_rootWidget)
			_rootWidget->Tick(deltaTime);
	}

	void UserWidget::Render(const Vector2& rootPosition)
	{
		if (_isInitialized == false || _isVisible == false || _rootWidget == nullptr)
			return;

		_rootWidget->Render(rootPosition);
	}
}
