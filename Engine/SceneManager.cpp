#include "SceneManager.h"

namespace gm
{
	SceneManager::SceneManager() = default;
	SceneManager::~SceneManager() = default;

	void SceneManager::Update()
	{
		CheckSceneChange();

		GM_ASSERT_RETURN(_activeScene, "활성 Scene이 없습니다.");

		_activeScene->Update();
	}

	void SceneManager::LateUpdate()
	{
		GM_ASSERT_RETURN(_activeScene, "활성 Scene이 없습니다.");

		_activeScene->LateUpdate();
	}

	void SceneManager::Render(HDC hDC)
	{
		GM_ASSERT_RETURN(_activeScene, "활성 Scene이 없습니다.");

		_activeScene->Render(hDC);
	}

	void SceneManager::EndFrame()
	{
		GM_ASSERT_RETURN(_activeScene, "활성 Scene이 없습니다.");

		_activeScene->EndFrame();
	}

	void SceneManager::RequestSceneChange(const std::wstring& pendingSceneName, const std::wstring& loadingSceneName)
	{
		auto sceneIter = _sceneList.find(pendingSceneName);
		GM_ASSERT_RETURN(sceneIter != _sceneList.end(), "%ls 은 존재하지 않습니다.", pendingSceneName);

		if (loadingSceneName.empty() == false)
		{
			auto loadingSceneIter = _sceneList.find(loadingSceneName);
			GM_ASSERT_RETURN(loadingSceneIter != _sceneList.end(), "%ls 은 존재하지 않습니다.", loadingSceneName);

			_pendingSceneName = pendingSceneName;
			_nextSceneName = loadingSceneName;
			return;
		}

		_pendingSceneName.clear();
		_nextSceneName = pendingSceneName;
	}

	void SceneManager::CheckSceneChange()
	{
		if (_nextSceneName.empty())
			return;

		if (_activeScene)
			_activeScene->OnExit();

		_activeScene = _sceneList[_nextSceneName].get();
		_activeScene->OnEnter();

		_nextSceneName.clear();
	}
}
