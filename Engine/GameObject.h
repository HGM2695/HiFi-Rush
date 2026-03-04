#pragma once

#include <vector>
#include <memory>
#include "Component.h"

struct HDC__;
typedef struct HDC__* HDC;

namespace gm
{
	class GameObject
	{
	public:
		GameObject();
		virtual ~GameObject();

		template <typename T>
		T* AddComponent()
		{
			static_assert(std::is_base_of_v<Component, T>, "T는 반드시 Component의 자식 클래스이어야 합니다.");

			std::unique_ptr<T> comp = std::make_unique<T>();
			T* raw = comp.get();
			raw->SetOwner(this);

			_ComponentList.push_back(std::move(comp));
			return raw;
		}

		template <typename T>
		T* GetComponent()
		{
			static_assert(std::is_base_of_v<Component, T>, "T는 반드시 Component의 자식 클래스이어야 합니다.");

			for (auto& c : _ComponentList)
			{
				if (auto casted = dynamic_cast<T*>(c.get()))
					return casted;
			}

			return nullptr;
		}

		template <typename T>
		const T* GetComponent() const
		{
			static_assert(std::is_base_of_v<Component, T>, "T는 반드시 Component의 자식 클래스이어야 합니다.");

			for (const auto& c : _ComponentList)
			{
				if (auto casted = dynamic_cast<const T*>(c.get()))
					return casted;
			}

			return nullptr;
		}

		void	Initialize();
		void	Update();
		void	LateUpdate();
		void	Render(HDC hDC);

	protected:
		virtual void	OnInitialize() {}
		virtual void	OnUpdate() {}
		virtual void	OnLateUpdate() {}
		virtual void	OnRender(HDC hDC) {}

	private:
		std::vector<std::unique_ptr<Component>> _ComponentList{};
	};
}


