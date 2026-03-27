#include "SceneManager.h"
#include "Application.h"
#include "Time.h"
#include "Scene.h"
#include "GMAssert.h"
#include "PhysicsSystem2D.h"

namespace gm
{
	SceneManager::SceneManager()
		: _activeScene(nullptr), _physicsSystem2D(std::make_unique<PhysicsSystem2D>())
	{
	}

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
		GM_ASSERT_RETURN(_activeScene, "활성 Scene이 없습니다.");

		_activeScene->Update();
	}

	void SceneManager::PhysicsUpdate()
	{
		GM_ASSERT_RETURN(_activeScene, "활성 Scene이 없습니다.");

		const float deltaTime = APPLICATION.GetTime().GetDeltaTime();

		switch (_physicsMode)
		{
		case PhysicsMode::None:
			return;
		case PhysicsMode::Physics2D:
			_physicsSystem2D->Simulate(*_activeScene, deltaTime);
			return;
		case PhysicsMode::Physics3D:
			return;
		}
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
}
