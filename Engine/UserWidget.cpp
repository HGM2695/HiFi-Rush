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

	void UserWidget::Update()
	{
		if (_isInitialized == false || _isVisible == false)
			return;

		NativeTick();

		if (_rootWidget)
			_rootWidget->Update();
	}

	void UserWidget::Render(const Vector2& rootPosition)
	{
		if (_isInitialized == false || _isVisible == false || _rootWidget == nullptr)
			return;

		_rootWidget->Render(rootPosition);
	}
}