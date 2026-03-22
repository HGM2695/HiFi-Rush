#include "SceneManager.h"
#include "Scene.h"
#include "GMAssert.h"

namespace gm
{
    SceneManager::SceneManager() = default;
    SceneManager::~SceneManager() = default;

	void SceneManager::PlayScene(const std::wstring& sceneName)
	{
		auto sceneIter = _sceneList.find(sceneName);
		GM_ASSERT_RETURN(sceneIter != _sceneList.end(), "입력한 Scene은 존재하지 않습니다.");

		if (_activeScene)
			_activeScene->OnExit();

		_activeScene = sceneIter->second.get();
		_activeScene->OnEnter();
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

	void SceneManager::EndFrame()
	{
		_activeScene->EndFrame();
	}
}
