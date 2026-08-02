#include "Scene.h"
#include "GameObject.h"
#include "CameraManager.h"
#include "Application.h"
#include "TimeSystem.h"
#include "TickManager.h"
#include <algorithm>

namespace gm
{
	Scene::Scene() : _cameraManager(std::make_unique<CameraManager>()), _tickManager(std::make_unique<TickManager>()){}

	Scene::~Scene() = default;

	void Scene::Enter()
	{
		if (_isInitialized == false)
			Initialize();

		OnEnter();
	}

	void Scene::Exit()
	{
		OnExit();

		if (_isUnloadOnExit)
			Unload();
	}

	void Scene::Initialize()
	{
		if (_isInitialized)
			return;

		OnInitialize();
		InitializePendingGameObjects();

		_isInitialized = true;
	}

	void Scene::Unload()
	{
		if (_isInitialized == false)
			return;

		OnUnload();
		_gameObjectSlots.clear();
		_freeGameObjectSlotIndices.clear();
		_pendingInitializeGameObjects.clear();
		_tickManager = std::make_unique<TickManager>();
		_cameraManager = std::make_unique<CameraManager>();
		_isInitialized = false;
	}

	void Scene::Tick(TickGroup group, float deltaTime)
	{
		_tickManager->Tick(group, deltaTime);

		// GameLogic Tick 시점에 Scene과 GameObject에 대해 OnTick 호출.
		if (group == TickGroup::GameLogic)
		{
			OnTick(deltaTime);

			for (auto& slot : _gameObjectSlots)
			{
				if (slot.gameObject == nullptr)
					continue;

				slot.gameObject->Tick(deltaTime);
			}
		}

		// Camera Tick 시점에 CameraManager를 갱신합니다.
		if (group == TickGroup::Camera)
			_cameraManager->Tick(deltaTime);
	}

	void Scene::Render()
	{
		OnRender();

		for (auto& slot : _gameObjectSlots)
		{
			if (slot.gameObject == nullptr)
				continue;

			slot.gameObject->Render();
		}
	}

	void Scene::EndFrame()
	{
		InitializePendingGameObjects();
		RemovePendingDestroyGameObjects();
	}

	void Scene::InitializePendingGameObjects()
	{
		if (_pendingInitializeGameObjects.empty())
			return;

		for (GameObject* gameObject : _pendingInitializeGameObjects)
		{
			if (gameObject->IsPendingDestroy())
				continue;

			gameObject->Initialize();
			_tickManager->RegisterGameObject(*gameObject);
		}

		_pendingInitializeGameObjects.clear();
	}

	void Scene::RemovePendingDestroyGameObjects()
	{
		for (uint32 i = 0; i < _gameObjectSlots.size(); ++i)
		{
			GameObjectSlot& slot = _gameObjectSlots[i];
			if (slot.gameObject == nullptr || slot.gameObject->IsPendingDestroy() == false)
				continue;

			_tickManager->UnregisterGameObject(*slot.gameObject);
			slot.gameObject.reset();
			++slot.generation;
			_freeGameObjectSlotIndices.push_back(i);
		}
	}

	void Scene::NotifyComponentAdded(Component& component)
	{
		_tickManager->Register(component);
	}

	uint32 Scene::AddGameObjectToSlot(std::unique_ptr<GameObject> gameObject)
	{
		if (_freeGameObjectSlotIndices.empty())
		{
			GameObjectSlot slot{};
			slot.gameObject = std::move(gameObject);
			_gameObjectSlots.push_back(std::move(slot));
			return static_cast<uint32>(_gameObjectSlots.size() - 1);
		}

		const uint32 slotIndex = _freeGameObjectSlotIndices.back();
		_freeGameObjectSlotIndices.pop_back();
		_gameObjectSlots[slotIndex].gameObject = std::move(gameObject);
		return slotIndex;
	}

	GameObject* Scene::FindGameObject(GameObjectHandle handle)
	{
		if (IsValid(handle) == false)
			return nullptr;

		return _gameObjectSlots[handle.index].gameObject.get();
	}

	const GameObject* Scene::FindGameObject(GameObjectHandle handle) const
	{
		if (IsValid(handle) == false)
			return nullptr;

		return _gameObjectSlots[handle.index].gameObject.get();
	}

	bool Scene::IsValid(GameObjectHandle handle) const
	{
		if (handle.IsValid() == false || handle.index >= _gameObjectSlots.size())
			return false;

		const GameObjectSlot& slot = _gameObjectSlots[handle.index];
		return slot.gameObject != nullptr && slot.generation == handle.generation && slot.gameObject->IsPendingDestroy() == false;
	}
}
