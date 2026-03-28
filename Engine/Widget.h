#pragma once

#include <memory>
#include <type_traits>
#include <utility>
#include <vector>
#include "Vector2.h"

struct HDC__;
typedef struct HDC__* HDC;

namespace gm
{
	class Widget
	{
	public:
		virtual ~Widget() = default;

		template <typename T, typename... Args>
		T* AddChild(Args&&... args)
		{
			static_assert(std::is_base_of_v<Widget, T>, "T는 반드시 Widget의 자식 클래스여야 합니다.");

			auto child = std::make_unique<T>(std::forward<Args>(args)...);
			T* raw = child.get();
			raw->_parent = this;

			_childList.push_back(std::move(child));
			return raw;
		}

		void					SetPosition(const math::Vector2& position) { _position = position; }
		const math::Vector2&	GetPosition() const { return _position; }

		void					SetSize(const math::Vector2& size) { _size = size; }
		const math::Vector2&	GetSize() const { return _size; }

		void					SetVisible(bool isVisible) { _isVisible = isVisible; }
		bool					IsVisible() const { return _isVisible; }

		void					Update();
		void					Render(HDC hDC, const math::Vector2& parentPosition = math::Vector2::Zero());

	protected:
		virtual void			OnUpdate() {}
		virtual void			OnRender(HDC hDC, const math::Vector2& absolutePosition) {}

	private:
		bool 									_isVisible = true;

		Widget*									_parent = nullptr;
		std::vector<std::unique_ptr<Widget>>	_childList{};

		math::Vector2							_position{};
		math::Vector2							_size{};
	};
}
