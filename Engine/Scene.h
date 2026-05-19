#pragma once

#include "EngineCore.h"
#include "Entity.h"
#include "TickGroup.h"
#include <type_traits>
#include <utility>
#include <vector>

namespace gm
{
	class GameObject;
	class CameraManager;
	class Component;
	class TickManager;

	class Scene : public Entity
	{
	friend class SceneManager;
	friend class GameObject;

	public:
		Scene();
		virtual ~Scene();

		template<typename T, typename... Args>
		T* SpawnGameObject(Args&&... args)
		{
			return SpawnGameObjectInternal<T>(std::forward<Args>(args)...);
		}

		template<typename T, typename... Args>
		T* SpawnGameObject(const Vector2& position, Args&&... args)
		{
			return SpawnGameObjectInternal<T>(position, std::forward<Args>(args)...);
		}

		void			Enter();
		void			Exit();
		void			Initialize();
		void			Unload();
		void			Tick(TickGroup group, float deltaTime);
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

		CameraManager*			GetCameraManager() { return _cameraManager.get(); }
		const CameraManager*	GetCameraManager() const { return _cameraManager.get(); }

	protected:
		virtual void	OnInitialize() {}
		virtual void	OnUnload() {}
		virtual void	OnTick(float deltaTime) {}
		virtual void	OnRender() {}

		virtual void	OnEnter() {};
		virtual void	OnExit() {};

	private:
		template<typename T, typename... Args>
		T* SpawnGameObjectInternal(Args&&... args)
		{
			static_assert(std::is_base_of_v<GameObject, T>);

			auto gameobject = std::make_unique<T>(std::forward<Args>(args)...);
			T* ptr = gameobject.get();
			ptr->SetScene(this);

			_gameObjectList.push_back(std::move(gameobject));
			_pendingInitializeGameObjects.push_back(ptr);

			return ptr;
		}

		void InitializePendingGameObjects();
		void RemovePendingDestroyGameObjects();
		void RegisterGameObjectComponents(GameObject& gameObject);
		void NotifyComponentAdded(Component& component);
		
	private:
		std::vector<std::unique_ptr<GameObject>>	_gameObjectList{};
		std::vector<GameObject*>					_pendingInitializeGameObjects{};
		std::unique_ptr<CameraManager>				_cameraManager = nullptr;
		std::unique_ptr<TickManager>				_tickManager = nullptr;
		bool										_isUnloadOnExit = true;
		bool										_isInitialized = false;
	};
}


