#pragma once

#include "EngineCore.h"
#include "WidgetTween.h"
#include <type_traits>
#include <utility>
#include <vector>

namespace gm
{
	struct WidgetGeometry
	{
		Vector2 center{};
		Vector2 size{};
	};

	enum class WidgetSizeRule
	{
		Fixed,
		FillParent
	};

	// UI tree를 구성하는 기본 요소입니다.
	class Widget
	{
	public:
		Widget();
		virtual ~Widget();

		template <typename T, typename... Args>
		T* AddChild(Args&&... args)
		{
			return AddChildInternal<T>(std::forward<Args>(args)...);
		}

		template <typename T, typename... Args>
		T* AddNamedChild(const std::wstring& name, Args&&... args)
		{
			T* child = AddChildInternal<T>(std::forward<Args>(args)...);
			child->SetName(name);
			return child;
		}

		void				Initialize();
		bool				IsInitialized() const { return _isInitialized; }

		void				SetName(const std::wstring& name) { _name = name; }
		const std::wstring&	GetName() const { return _name; }

		template <typename T>
		T* FindWidget(const std::wstring& name)
		{
			static_assert(std::is_base_of_v<Widget, T>, "T는 반드시 Widget의 자식 클래스여야 합니다.");

			if (_name == name)
				return dynamic_cast<T*>(this);

			for (const auto& child : _childList)
			{
				T* found = child->FindWidget<T>(name);
				if (found)
					return found;
			}

			return nullptr;
		}

		template <typename T>
		const T* FindWidget(const std::wstring& name) const
		{
			static_assert(std::is_base_of_v<Widget, T>, "T는 반드시 Widget의 자식 클래스여야 합니다.");

			if (_name == name)
				return dynamic_cast<const T*>(this);

			for (const auto& child : _childList)
			{
				const T* found = child->FindWidget<T>(name);
				if (found)
					return found;
			}

			return nullptr;
		}

		void				SetPosition(const Vector2& position) { _position = position; }
		const Vector2&		GetPosition() const { return _position; }

		void				SetSize(const Vector2& size) { _size = size; }
		const Vector2&		GetSize() const { return _size; }

		template <typename T, typename... Args>
		T* AddTween(Args&&... args)
		{
			static_assert(std::is_base_of_v<WidgetTween, T>, "T는 반드시 WidgetTween의 자식 클래스여야 합니다.");

			auto tween = std::make_unique<T>(std::forward<Args>(args)...);
			T* raw = tween.get();
			raw->Start(*this);
			_tweens.push_back(std::move(tween));
			return raw;
		}

		void				SetSizeRule(WidgetSizeRule sizeRule) { _sizeRule = sizeRule; }
		WidgetSizeRule		GetSizeRule() const { return _sizeRule; }

		void				SetVisible(bool isVisible) { _isVisible = isVisible; }
		bool				IsVisible() const { return _isVisible; }
		void				ToggleVisibility() { _isVisible = !_isVisible; }

		void				Tick(float deltaTime);
		void				Render(const WidgetGeometry& parentGeometry = WidgetGeometry{});

	protected:
		virtual void		OnInitialize() {}
		virtual void		OnTick(float deltaTime) {}
		virtual void		OnRender(const WidgetGeometry& geometry) {}

	private:
		Vector2				ResolveSize(const WidgetGeometry& parentGeometry) const;

		template <typename T, typename... Args>
		T* AddChildInternal(Args&&... args)
		{
			static_assert(std::is_base_of_v<Widget, T>, "T는 반드시 Widget의 자식 클래스여야 합니다.");

			auto child = std::make_unique<T>(std::forward<Args>(args)...);
			T* raw = child.get();
			raw->_parent = this;

			_childList.push_back(std::move(child));

			if (_isInitialized)
				raw->Initialize();

			return raw;
		}

	private:
		bool 									_isVisible = true;
		bool									_isInitialized = false;

		Widget*									_parent = nullptr;
		std::vector<std::unique_ptr<Widget>>	_childList{};
		std::vector<std::unique_ptr<WidgetTween>> _tweens{};

		std::wstring							_name = L"Widget";
		Vector2									_position{};
		Vector2									_size{};
		WidgetSizeRule							_sizeRule = WidgetSizeRule::Fixed;
	};
}
