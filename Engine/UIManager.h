#pragma once

#include <memory>
#include <type_traits>
#include <utility>
#include <vector>
#include "UserWidget.h"

struct HDC__;
typedef struct HDC__* HDC;

namespace gm
{
	class UIManager
	{
	public:
		template <typename T, typename... Args>
		T* AddWidget(Args&&... args)
		{
			static_assert(std::is_base_of_v<UserWidget, T>, "T는 반드시 UserWidget의 자식 클래스여야 합니다.");

			auto widget = std::make_unique<T>(std::forward<Args>(args)...);
			T* raw = widget.get();
			raw->Initialize();

			_widgetList.push_back(std::move(widget));
			return raw;
		}

		void Update();
		void Render(HDC hDC);
		void ClearViewportWidgets();

	private:
		std::vector<std::unique_ptr<UserWidget>> _widgetList{};
	};
}
