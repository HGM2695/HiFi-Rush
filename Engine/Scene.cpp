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
		_gameObjectList.clear();
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

			for (auto& gameObject : _gameObjectList)
				gameObject->Tick(deltaTime);
		}

		// GameLogic Tick 시점에 Scene과 GameObject에 대해 OnTick 호출.
		if (group == TickGroup::Camera)
			_cameraManager->Tick(deltaTime);
	}

	void Scene::Render()
	{
		OnRender();

		for (auto& gameObject : _gameObjectList)
			gameObject->Render();
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
			RegisterGameObjectComponents(*gameObject);
		}

		_pendingInitializeGameObjects.clear();
	}

	void Scene::RemovePendingDestroyGameObjects()
	{
		for (auto& gameObject : _gameObjectList)
		{
			if (gameObject->IsPendingDestroy())
				_tickManager->UnregisterGameObject(*gameObject);
		}

		std::erase_if(_gameObjectList,
			[](const std::unique_ptr<GameObject>& gameObject)
			{
				return gameObject->IsPendingDestroy();
			});
	}

	void Scene::RegisterGameObjectComponents(GameObject& gameObject)
	{
		_tickManager->RegisterGameObject(gameObject);
	}

	void Scene::NotifyComponentAdded(Component& component)
	{
		_tickManager->Register(component);
	}
}
