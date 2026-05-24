#pragma once

#include "Component.h"
#include <type_traits>
#include <utility>

namespace gm
{
	class TransformComponent;
	class UserWidget;

	class WidgetComponent : public Component
	{
	public:
		WidgetComponent() = default;
		~WidgetComponent() override = default;

		virtual TickGroup	GetTickGroup() const { return TickGroup::Attachment; }

		template <typename T, typename... Args>
		T* SetUserWidget(Args&&... args)
		{
			static_assert(std::is_base_of_v<UserWidget, T>, "T는 반드시 UserWidget의 자식 클래스여야 합니다.");

			auto widget = std::make_unique<T>(std::forward<Args>(args)...);
			T* raw = widget.get();			

			_widget = std::move(widget);
			return raw;
		}

		void					SetWidgetVisible(bool isVisible);
		bool					IsWidgetVisible() const;
		void					ToggleWidgetVisibility();

		UserWidget*				GetUserWidget() { return _widget.get(); }
		const UserWidget*		GetUserWidget() const { return _widget.get(); }

	protected:
		void					OnInitialize() override;
		void					OnTick(float deltaTime) override;
		void					OnRender() override;

	private:
		TransformComponent*					_ownerTransform = nullptr;
		std::unique_ptr<UserWidget> _widget{};
	};
}
