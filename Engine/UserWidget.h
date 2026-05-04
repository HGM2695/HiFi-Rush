#pragma once

#include "EngineCore.h"
namespace gm
{
	class Widget;

	class UserWidget
	{
	public:
		UserWidget();
		virtual ~UserWidget();

		void			Initialize();
		void			Update();
		void			Render(HDC hDC, const Vector2& rootPosition = Vector2{});

		void			SetVisible(bool isVisible) { _isVisible = isVisible; }
		bool			IsVisible() const { return _isVisible; }
		void			ToggleVisibility() { _isVisible = !_isVisible; }

		Widget*			GetRootWidget() { return _rootWidget.get(); }
		const Widget*	GetRootWidget() const { return _rootWidget.get(); }

	protected:
		virtual std::unique_ptr<Widget>		BuildWidgetTree() = 0;
		virtual void						NativeConstruct() {}
		virtual void						NativeTick() {}

	private:
		std::unique_ptr<Widget>		_rootWidget{};
		bool						_isInitialized = false;
		bool						_isVisible = true;
	};
}
