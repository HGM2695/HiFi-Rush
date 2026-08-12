#pragma once

#include "EngineCore.h"
#include "Entity.h"
#include "GameObjectHandle.h"
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
	class WeakGameObjectPtr;

	class Scene : public Entity
	{
	friend class SceneManager;
	friend class GameObject;
	friend class WeakGameObjectPtr;

	public:
		Scene();
		virtual ~Scene();

		template<typename T, typename... Args>
		T* SpawnGameObject(Args&&... args)
		{
			return SpawnGameObjectInternal<T>(std::forward<Args>(args)...);
		}

		template<typename T, typename... Args>
		T* SpawnGameObject(const Vector3& position, Args&&... args)
		{
			return SpawnGameObjectInternal<T>(position, std::forward<Args>(args)...);
		}

		template <typename TFunc>
		void ForEachGameObjectIncludingPending(TFunc&& func)
		{
			for (auto& slot : _gameObjectSlots)
			{
				if (slot.gameObject == nullptr)
					continue;

				func(*slot.gameObject);
			}
		}

		template <typename TFunc>
		void ForEachGameObjectIncludingPending(TFunc&& func) const
		{
			for (const auto& slot : _gameObjectSlots)
			{
				if (slot.gameObject == nullptr)
					continue;

				func(static_cast<const GameObject&>(*slot.gameObject));
			}
		}

		template <typename TFunc>
		void ForEachGameObject(TFunc&& func)
		{
			for (auto& slot : _gameObjectSlots)
			{
				if (slot.gameObject == nullptr || slot.gameObject->IsPendingDestroy())
					continue;

				func(*slot.gameObject);
			}
		}

		template <typename TFunc>
		void ForEachGameObject(TFunc&& func) const
		{
			for (const auto& slot : _gameObjectSlots)
			{
				if (slot.gameObject == nullptr || slot.gameObject->IsPendingDestroy())
					continue;

				func(*slot.gameObject);
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
			const uint32 slotIndex = AddGameObjectToSlot(std::move(gameobject));
			ptr->SetHandle(GameObjectHandle{ slotIndex, _gameObjectSlots[slotIndex].generation });

			_pendingInitializeGameObjects.push_back(ptr);

			return ptr;
		}

		struct GameObjectSlot
		{
			std::unique_ptr<GameObject> gameObject;
			uint32						generation = 0;
		};

		void			Enter();
		void			Exit();
		void			Initialize();
		void			Unload();
		void			Tick(TickGroup group, float deltaTime);
		void			Render();
		void			EndFrame();

		void				InitializePendingGameObjects();
		void				RemovePendingDestroyGameObjects();
		void				NotifyComponentAdded(Component& component);
		uint32				AddGameObjectToSlot(std::unique_ptr<GameObject> gameObject);
		GameObject*			FindGameObject(GameObjectHandle handle);
		const GameObject*	FindGameObject(GameObjectHandle handle) const;
		bool				IsValid(GameObjectHandle handle) const;
		
	private:
		std::vector<GameObjectSlot>					_gameObjectSlots{};
		std::vector<uint32>							_freeGameObjectSlotIndices{};
		std::vector<GameObject*>					_pendingInitializeGameObjects{};
		std::unique_ptr<CameraManager>				_cameraManager = nullptr;
		std::unique_ptr<TickManager>				_tickManager = nullptr;
		bool										_isUnloadOnExit = true;
		bool										_isInitialized = false;
	};
}


