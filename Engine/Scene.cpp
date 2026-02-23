#include "Scene.h"
#include "GameObject.h"

namespace gm
{
	Scene::Scene() = default;
	Scene::~Scene() = default;

	void Scene::Initialize()
	{
		OnInitialize();
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
}