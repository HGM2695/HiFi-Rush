#pragma once

#include <vector>
#include <memory>
#include "Entity.h"
#include "Vector2.h"

struct HDC__;
typedef struct HDC__* HDC;

namespace gm
{
	class GameObject;

	class Scene : public Entity
	{
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

		virtual void	OnEnter() {};
		virtual void	OnExit() {};

		void			AddGameObject(std::unique_ptr<GameObject> gameObject);

	protected:
		virtual void	OnInitialize() {}
		virtual void	OnUpdate() {}
		virtual void	OnLateUpdate() {}
		virtual void	OnRender(HDC hDC) {}

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
		std::vector<std::unique_ptr<GameObject>> _gameObjectList{};
	};
}


