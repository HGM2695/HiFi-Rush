#pragma once

#include "EngineCore.h"
#include <type_traits>
#include <utility>
#include <vector>

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

		void					SetPosition(const Vector2& position) { _position = position; }
		const Vector2&			GetPosition() const { return _position; }

		void					SetSize(const Vector2& size) { _size = size; }
		const Vector2&			GetSize() const { return _size; }

		void					SetVisible(bool isVisible) { _isVisible = isVisible; }
		bool					IsVisible() const { return _isVisible; }

		void					Tick(float deltaTime);
		void					Render(const Vector2& parentPosition = Vector2{});

	protected:
		virtual void			OnTick(float deltaTime) {}
		virtual void			OnRender(const Vector2& absolutePosition) {}

	private:
		bool 									_isVisible = true;

		Widget*									_parent = nullptr;
		std::vector<std::unique_ptr<Widget>>	_childList{};

		Vector2									_position{};
		Vector2									_size{};
	};
}
