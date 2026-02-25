#include "SceneManager.h"
#include "Scene.h"

namespace gm
{
    SceneManager::SceneManager() = default;
    SceneManager::~SceneManager() = default;

	void SceneManager::PlayScene(const std::wstring& sceneName)
	{
		auto sceneIter = _sceneList.find(sceneName);
		if (sceneIter != _sceneList.end())
			_activeScene = sceneIter->second.get();
	}

	void SceneManager::Initialize()
	{
		
	}

	void SceneManager::Update()
	{
		_activeScene->Update();
	}

	void SceneManager::LateUpdate()
	{
		_activeScene->LateUpdate();
	}
	
	void SceneManager::Render(HDC hDC)
	{
		_activeScene->Render(hDC);
	}
}