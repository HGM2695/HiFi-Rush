#pragma once

#include "EngineCore.h"
#include "UserWidget.h"
#include <type_traits>
#include <utility>
#include <vector>

namespace gm
{
	class UIManager
	{
	public:
		template <typename T, typename... Args>
		T* AddUserWidget(Args&&... args)
		{
			auto widget = CreateUserWidget<T>(std::forward<Args>(args)...);
			T* raw = widget.get();

			_viewportWidgetList.push_back(std::move(widget));
			return raw;
		}

		template <typename T, typename... Args>
		T* AddDebugUserWidget(Args&&... args)
		{
#if GM_ENABLE_DEBUG_TOOLS
			auto widget = CreateUserWidget<T>(std::forward<Args>(args)...);
			T* raw = widget.get();

			_debugWidgetList.push_back(std::move(widget));
			return raw;
#else
			return nullptr;
#endif
		}

		void Tick(float deltaTime);
		void Render();
		void ClearViewportWidgets();
		void ClearDebugWidgets();
		void ClearAllWidgets();

#if GM_ENABLE_DEBUG_TOOLS
		void SetDebugWidgetsVisible(bool isVisible) { _areDebugWidgetsVisible = isVisible; }
		bool AreDebugWidgetsVisible() const { return _areDebugWidgetsVisible; }
#endif

	private:
		template <typename T, typename... Args>
		std::unique_ptr<T> CreateUserWidget(Args&&... args)
		{
			static_assert(std::is_base_of_v<UserWidget, T>, "T는 반드시 UserWidget의 자식 클래스여야 합니다.");

			auto widget = std::make_unique<T>(std::forward<Args>(args)...);
			widget->Initialize();

			return widget;
		}

	private:
		std::vector<std::unique_ptr<UserWidget>> _viewportWidgetList{};

#if GM_ENABLE_DEBUG_TOOLS
		std::vector<std::unique_ptr<UserWidget>> _debugWidgetList{};
		bool _areDebugWidgetsVisible = true;
#endif
	};
}
