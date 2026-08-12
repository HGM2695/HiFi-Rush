#pragma once

#include "EngineCore.h"
#include "GameObjectHandle.h"
#include <type_traits>
#include <utility>
#include <vector>
#include "Component.h"

namespace gm
{
	class Collider2DComponent;
	class Collider3DComponent;
	class Rigidbody2DComponent;
	class Rigidbody3DComponent;
	class Scene;
	class TransformComponent;
	class WeakGameObjectPtr;

	enum class GameObjectLifeState
	{
		Active,
		PendingDestroy,
	};

	class GameObject
	{
	public:
		GameObject();
		GameObject(const Vector3& position);
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

			if (_isInitialized)
			{
				raw->Initialize();
				NotifyComponentAdded(*raw);
			}

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

		const TransformComponent*				GetTransform() const;
		TransformComponent*						GetTransform();
		const Rigidbody2DComponent*				GetRigidbody2D() const { return _rigidbody2D; }
		Rigidbody2DComponent*					GetRigidbody2D() { return _rigidbody2D; }
		const Rigidbody3DComponent*				GetRigidbody3D() const { return _rigidbody3D; }
		Rigidbody3DComponent*					GetRigidbody3D() { return _rigidbody3D; }
		const std::vector<Collider2DComponent*>& GetColliders2D() const { return _colliders2D; }
		const std::vector<Collider3DComponent*>& GetColliders3D() const { return _colliders3D; }

		void			Initialize();
		void			Tick(float deltaTime);
		void			Render();

		void			Destroy();
		bool			IsInitialized() const { return _isInitialized; }
		bool			IsPendingDestroy() const { return _lifeState == GameObjectLifeState::PendingDestroy; }

		void			SetRender(bool isRender) { _isRender = isRender; }
		bool			IsRenderEnabled() const { return _isRender; }

		Scene*				GetScene() const { return _scene; }
		WeakGameObjectPtr	GetWeakPtr() const;

		template <typename TFunc>
		void ForEachComponent(TFunc&& func)
		{
			for (auto& component : _componentList)
				func(*component);
		}

		template <typename TFunc>
		void ForEachComponent(TFunc&& func) const
		{
			for (const auto& component : _componentList)
				func(*component);
		}

	protected:
		virtual void	OnInitialize() {}
		virtual void	OnTick(float deltaTime) {}
		virtual void	OnRender() {}

	private:
		friend class Scene;

		void			SetScene(Scene* scene) { _scene = scene; }
		void			SetHandle(GameObjectHandle handle) { _handle = handle; }
		bool			RegisterComponent(Component* component);
		void			NotifyComponentAdded(Component& component);

	private:
		std::vector<std::unique_ptr<Component>> _componentList{};
		Scene*									_scene = nullptr;
		GameObjectHandle						_handle{};

		TransformComponent*						_transform = nullptr;

		Rigidbody2DComponent*					_rigidbody2D = nullptr;
		Rigidbody3DComponent*					_rigidbody3D = nullptr;
		std::vector<Collider2DComponent*>		_colliders2D{};
		std::vector<Collider3DComponent*>		_colliders3D{};

		GameObjectLifeState						_lifeState = GameObjectLifeState::Active;
		bool									_isRender = true;
		bool									_isInitialized = false;
	};
}

namespace gm
{
	template <>
	inline TransformComponent* GameObject::GetComponent<TransformComponent>() { return _transform; }

	template <>
	inline const TransformComponent* GameObject::GetComponent<TransformComponent>() const { return _transform; }

	template <>
	inline Rigidbody2DComponent* GameObject::GetComponent<Rigidbody2DComponent>() { return _rigidbody2D; }

	template <>
	inline const Rigidbody2DComponent* GameObject::GetComponent<Rigidbody2DComponent>() const { return _rigidbody2D; }

	template <>
	inline Rigidbody3DComponent* GameObject::GetComponent<Rigidbody3DComponent>() { return _rigidbody3D; }

	template <>
	inline const Rigidbody3DComponent* GameObject::GetComponent<Rigidbody3DComponent>() const { return _rigidbody3D; }
}
