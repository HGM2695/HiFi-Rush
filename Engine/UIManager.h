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
			static_assert(std::is_base_of_v<UserWidget, T>, "T는 반드시 UserWidget의 자식 클래스여야 합니다.");

			auto widget = std::make_unique<T>(std::forward<Args>(args)...);
			T* raw = widget.get();
			raw->Initialize();

			_userWidgetList.push_back(std::move(widget));
			return raw;
		}

		void Tick(float deltaTime);
		void Render();
		void ClearViewportWidgets();

	private:
		std::vector<std::unique_ptr<UserWidget>> _userWidgetList{};
	};
}
