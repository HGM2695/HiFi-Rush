#pragma once

#include "EngineCore.h"
#include <vector>
#include <memory>
#include "Entity.h"
#include "Vector2.h"

namespace gm
{
	class GameObject;

	class Scene : public Entity
	{
	friend class SceneManager;

	public:
		Scene();
		virtual ~Scene();

		template<typename T, typename... Args>
		T* Instantiate(Args&&... args)
		{
			return CreateGameObject<T>(std::forward<Args>(args)...);
		}

		template<typename T, typename... Args>
		T* Instantiate(const math::Vector2& position, Args&&... args)
		{
			return CreateGameObject<T>(position, std::forward<Args>(args)...);
		}

		void			Initialize();
		void			Update();
		void			LateUpdate();
		void			Render(HDC hDC);
		void			EndFrame();

		template <typename TFunc>
		void ForEachGameObject(TFunc&& func)
		{
			for (auto& gameObject : _gameObjectList)
				func(*gameObject);
		}

		template <typename TFunc>
		void ForEachGameObject(TFunc&& func) const
		{
			for (const auto& gameObject : _gameObjectList)
				func(static_cast<const GameObject&>(*gameObject));
		}

		template <typename TFunc>
		void ForEachAliveGameObject(TFunc&& func)
		{
			for (auto& gameObject : _gameObjectList)
			{
				if (gameObject->IsPendingDestroy())
					continue;

				func(*gameObject);
			}
		}

		template <typename TFunc>
		void ForEachAliveGameObject(TFunc&& func) const
		{
			for (const auto& gameObject : _gameObjectList)
			{
				if (gameObject->IsPendingDestroy())
					continue;

				func(*gameObject);
			}
		}

		void			AddGameObject(std::unique_ptr<GameObject> gameObject);

	protected:
		virtual void	OnInitialize() {}
		virtual void	OnUpdate() {}
		virtual void	OnLateUpdate() {}
		virtual void	OnRender(HDC hDC) {}

		virtual void	OnEnter() {};
		virtual void	OnExit() {};

	private:
		template<typename T, typename... Args>
		T* CreateGameObject(Args&&... args)
		{
			static_assert(std::is_base_of_v<GameObject, T>);

			auto gameobject = std::make_unique<T>(std::forward<Args>(args)...);
			T* ptr = gameobject.get();

			_gameObjectList.push_back(std::move(gameobject));

			return ptr;
		}

		void			RemovePendingDestroyGameObjects();
		
	private:
		std::vector<std::unique_ptr<GameObject>>	_gameObjectList{};
	};
}


