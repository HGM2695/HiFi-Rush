#pragma once

#include "EngineCore.h"
#include <type_traits>
#include <utility>
#include <vector>
#include "Entity.h"

namespace gm
{
	class GameObject;
	class CameraManager;

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
		T* Instantiate(const Vector2& position, Args&&... args)
		{
			return CreateGameObject<T>(position, std::forward<Args>(args)...);
		}

		void			Enter();
		void			Exit();
		void			Initialize();
		void			Unload();
		void			Update();
		void			LateUpdate();
		void			Render();
		void			EndFrame();

		template <typename TFunc>
		void ForEachGameObjectIncludingPending(TFunc&& func)
		{
			for (auto& gameObject : _gameObjectList)
				func(*gameObject);
		}

		template <typename TFunc>
		void ForEachGameObjectIncludingPending(TFunc&& func) const
		{
			for (const auto& gameObject : _gameObjectList)
				func(static_cast<const GameObject&>(*gameObject));
		}

		template <typename TFunc>
		void ForEachGameObject(TFunc&& func)
		{
			for (auto& gameObject : _gameObjectList)
			{
				if (gameObject->IsPendingDestroy())
					continue;

				func(*gameObject);
			}
		}

		template <typename TFunc>
		void ForEachGameObject(TFunc&& func) const
		{
			for (const auto& gameObject : _gameObjectList)
			{
				if (gameObject->IsPendingDestroy())
					continue;

				func(*gameObject);
			}
		}

		bool					IsInitialized() const { return _isInitialized; }
		void					SetUnloadOnExit(bool isUnloadOnExit) { _isUnloadOnExit = isUnloadOnExit; }
		bool					GetUnloadOnExit() const { return _isUnloadOnExit; }

		void					AddGameObject(std::unique_ptr<GameObject> gameObject);
		CameraManager*			GetCameraManager() { return _cameraManager.get(); }
		const CameraManager*	GetCameraManager() const { return _cameraManager.get(); }

	protected:
		virtual void	OnInitialize() {}
		virtual void	OnUnload() {}
		virtual void	OnUpdate() {}
		virtual void	OnLateUpdate() {}
		virtual void	OnRender() {}

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

		void RemovePendingDestroyGameObjects();
		
	private:
		std::vector<std::unique_ptr<GameObject>>	_gameObjectList{};
		std::unique_ptr<CameraManager>				_cameraManager = nullptr;
		bool										_isUnloadOnExit = true;
		bool										_isInitialized = false;
	};
}


