#pragma once

#include "EngineCore.h"
#include "Widget.h"

namespace gm
{
	// Widget을 조합하여 사용자 정의 UI를 만들어내는 단위입니다.
	class UserWidget
	{
	public:
		UserWidget();
		virtual ~UserWidget();

		void			Initialize();
		void			Tick(float deltaTime);
		void			Render(const WidgetGeometry& rootGeometry = WidgetGeometry{});

		void			SetVisible(bool isVisible) { _isVisible = isVisible; }
		bool			IsVisible() const { return _isVisible; }
		void			ToggleVisibility() { _isVisible = !_isVisible; }

		bool			IsInitialized() const { return _isInitialized; }

		Widget*			GetRootWidget() { return _rootWidget.get(); }
		const Widget*	GetRootWidget() const { return _rootWidget.get(); }

		template <typename T, typename... Args>
		std::unique_ptr<T> CreateRootWidget(Args&&... args)
		{
			static_assert(std::is_base_of_v<Widget, T>, "T는 반드시 Widget의 자식 클래스여야 합니다.");

			return std::make_unique<T>(std::forward<Args>(args)...);
		}

		template <typename T, typename... Args>
		std::unique_ptr<T> CreateNamedRootWidget(const std::wstring& name, Args&&... args)
		{
			static_assert(std::is_base_of_v<Widget, T>, "T는 반드시 Widget의 자식 클래스여야 합니다.");

			auto root = std::make_unique<T>(std::forward<Args>(args)...);
			root->SetName(name);
			return root;
		}

		template <typename T>
		T* FindWidget(const std::wstring& name)
		{
			if (_rootWidget == nullptr)
				return nullptr;

			return _rootWidget->FindWidget<T>(name);
		}

		template <typename T>
		const T* FindWidget(const std::wstring& name) const
		{
			if (_rootWidget == nullptr)
				return nullptr;

			return _rootWidget->FindWidget<T>(name);
		}

	protected:
		virtual std::unique_ptr<Widget>		BuildWidgetTree() = 0;
		virtual void						OnInitialize() {}
		virtual void						OnTick(float deltaTime) {}

	private:
		std::unique_ptr<Widget>		_rootWidget{};
		bool						_isInitialized = false;
		bool						_isVisible = true;
	};
}
