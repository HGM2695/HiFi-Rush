#include "SceneManager.h"
#include "Scene.h"

namespace gm
{
	SceneManager::SceneManager() = default;
	SceneManager::~SceneManager() = default;

	void SceneManager::BeginFrame()
	{
		CheckSceneChange();
	}

	void SceneManager::Tick(TickGroup group, float deltaTime)
	{
		GM_ASSERT_RETURN(_activeScene, "활성 Scene이 없습니다.");

		_activeScene->Tick(group, deltaTime);
	}

	void SceneManager::Render()
	{
		GM_ASSERT_RETURN(_activeScene, "활성 Scene이 없습니다.");

		_activeScene->Render();
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

		if (loadingSceneName.empty())
		{
			_pendingSceneName.clear();
			_nextSceneName = pendingSceneName;

			return;
		}

		auto loadingSceneIter = _sceneList.find(loadingSceneName);
		GM_ASSERT_RETURN(loadingSceneIter != _sceneList.end(), "%ls 은 존재하지 않습니다.", loadingSceneName);

		_pendingSceneName = pendingSceneName;
		_nextSceneName = loadingSceneName;
	}

	void SceneManager::CheckSceneChange()
	{
		if (_nextSceneName.empty())
			return;

		if (_activeScene)
			_activeScene->Exit();

		_activeScene = _sceneList[_nextSceneName].get();
		_activeScene->Enter();

		_nextSceneName.clear();
	}
}
