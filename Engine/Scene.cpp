#include "Scene.h"
#include "GameObject.h"

namespace gm
{
	Scene::Scene() = default;
	Scene::~Scene() = default;

	void Scene::Initialize()
	{

	}

	void Scene::Update()
	{
		for (auto& gameObject : _gameObjectList)
			gameObject->Update();
	}

	void Scene::LateUpdate()
	{
		for (auto& gameObject : _gameObjectList)
			gameObject->LateUpdate();
	}

	void Scene::Render(HDC hDC)
	{
		for (auto& gameObject : _gameObjectList)
			gameObject->Render(hDC);
	}
}