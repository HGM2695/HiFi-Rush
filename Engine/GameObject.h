#pragma once

#include "EngineCore.h"
#include <memory>
#include <vector>
#include "Component.h"

namespace gm
{
	class Collider2D;
	class Rigidbody2D;
	class Transform;

	enum class GameObjectLifeState
	{
		Active,
		PendingDestroy,
	};

	class GameObject
	{
	public:
		GameObject();
		GameObject(const Vector2& position);
		virtual ~GameObject();

		template <typename T, typename... Args>
		T* AddComponent(Args&&... args)
		{
			static_assert(std::is_base_of_v<Component, T>, "T는 반드시 Component의 자식 클래스이어야 합니다.");

			std::unique_ptr<T> comp = std::make_unique<T>(std::forward<Args>(args)...);
			T* raw = comp.get();
			raw->SetOwner(this);

			if (RegisterComponent(raw) == false)
				return nullptr;

			_componentList.push_back(std::move(comp));
			return raw;
		}

		template <typename T>
		T* GetComponent()
		{
			static_assert(std::is_base_of_v<Component, T>, "T는 반드시 Component의 자식 클래스이어야 합니다.");

			for (auto& c : _componentList)
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

			for (const auto& c : _componentList)
			{
				if (auto casted = dynamic_cast<const T*>(c.get()))
					return casted;
			}

			return nullptr;
		}

		const Transform*				GetTransform() const;
		Transform*						GetTransform();
		const Rigidbody2D*				GetRigidbody2D() const { return _rigidbody2D; }
		Rigidbody2D*					GetRigidbody2D() { return _rigidbody2D; }
		const std::vector<Collider2D*>& GetColliders2D() const { return _colliders2D; }

		void			Initialize();
		void			Update();
		void			LateUpdate();
		void			Render(HDC hDC);

		void			Destroy();
		bool			IsPendingDestroy() const { return _lifeState == GameObjectLifeState::PendingDestroy; }

		void			SetRender(bool isRender) { _isRender = isRender; }
		bool			IsRenderEnabled() const { return _isRender; }

	protected:
		virtual void	OnInitialize() {}
		virtual void	OnUpdate() {}
		virtual void	OnLateUpdate() {}
		virtual void	OnRender(HDC hDC) {}

	private:
		bool			RegisterComponent(Component* component);

	private:
		std::vector<std::unique_ptr<Component>> _componentList{};

		Transform*								_transform = nullptr;

		Rigidbody2D*							_rigidbody2D = nullptr;
		std::vector<Collider2D*>				_colliders2D{};

		GameObjectLifeState						_lifeState = GameObjectLifeState::Active;
		bool									_isRender = true;
	};
}

namespace gm
{
	template <>
	inline Transform* GameObject::GetComponent<Transform>() { return _transform; }

	template <>
	inline const Transform* GameObject::GetComponent<Transform>() const { return _transform; }

	template <>
	inline Rigidbody2D* GameObject::GetComponent<Rigidbody2D>() { return _rigidbody2D; }

	template <>
	inline const Rigidbody2D* GameObject::GetComponent<Rigidbody2D>() const { return _rigidbody2D; }
}
