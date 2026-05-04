#pragma once

#include <memory>
#include <type_traits>
#include <utility>
#include "Component.h"

namespace gm
{
	class Transform;
	class UserWidget;

	class WidgetComponent : public Component
	{
	public:
		WidgetComponent() = default;
		~WidgetComponent() override = default;

		template <typename T, typename... Args>
		T* CreateUserWidget(Args&&... args)
		{
			static_assert(std::is_base_of_v<UserWidget, T>, "T는 반드시 UserWidget의 자식 클래스여야 합니다.");

			auto widget = std::make_unique<T>(std::forward<Args>(args)...);
			T* raw = widget.get();
			raw->Initialize();

			_widget = std::move(widget);
			return raw;
		}

		void					SetWorldOffset(const Vector2& worldOffset) { _worldOffset = worldOffset; }
		const Vector2&			GetWorldOffset() const { return _worldOffset; }

		void					SetWidgetVisible(bool isVisible);
		bool					IsWidgetVisible() const;
		void					ToggleWidgetVisibility();

		UserWidget*				GetWidget() { return _widget.get(); }
		const UserWidget*		GetWidget() const { return _widget.get(); }

	protected:
		void					OnInitialize() override;
		void					OnUpdate() override;
		void					OnRender(HDC hDC) override;

	private:
		Transform*					_ownerTransform = nullptr;
		std::unique_ptr<UserWidget> _widget{};
		Vector2						_worldOffset{};
	};
}
