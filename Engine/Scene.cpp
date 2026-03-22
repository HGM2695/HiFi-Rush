#include "Scene.h"
#include "GameObject.h"
#include <algorithm>

namespace gm
{
	Scene::Scene() = default;
	Scene::~Scene() = default;

	void Scene::Initialize()
	{
		OnInitialize();

		for (auto& gameObject : _gameObjectList)
			gameObject->Initialize();
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
	}

	void Scene::Render(HDC hDC)
	{
		OnRender(hDC);

		for (auto& gameObject : _gameObjectList)
			gameObject->Render(hDC);
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
