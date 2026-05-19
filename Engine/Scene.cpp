#include "Scene.h"
#include "GameObject.h"
#include "CameraManager.h"
#include "Application.h"
#include "TimeSystem.h"
#include <algorithm>

namespace gm
{
	Scene::Scene() : _cameraManager(std::make_unique<CameraManager>()){ }

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

		for (auto& gameObject : _gameObjectList)
			gameObject->Initialize();

		_isInitialized = true;
	}

	void Scene::Unload()
	{
		if (_isInitialized == false)
			return;

		OnUnload();
		_gameObjectList.clear();
		_cameraManager = std::make_unique<CameraManager>();
		_isInitialized = false;
	}

	void Scene::Update()
	{
		OnUpdate();

		for (auto& gameObject : _gameObjectList)
			gameObject->Update();
	}

	void Scene::LateUpdate()
	{
		OnLateUpdate();

		for (auto& gameObject : _gameObjectList)
			gameObject->LateUpdate();

		_cameraManager->Update(APPLICATION.GetTimeSystem().GetDeltaTime());
	}

	void Scene::Render()
	{
		OnRender();

		for (auto& gameObject : _gameObjectList)
			gameObject->Render();
	}

	void Scene::EndFrame()
	{
		RemovePendingDestroyGameObjects();
	}

	void Scene::AddGameObject(std::unique_ptr<GameObject> gameObject)
	{
		if (gameObject)
			_gameObjectList.push_back(std::move(gameObject));
	}

	void Scene::RemovePendingDestroyGameObjects()
	{
		_gameObjectList.erase(
			std::remove_if(_gameObjectList.begin(), _gameObjectList.end(),
				[](const std::unique_ptr<GameObject>& gameObject)
				{
					return gameObject->IsPendingDestroy();
				}),
			_gameObjectList.end());
	}
}
