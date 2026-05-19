#pragma once

#include "EngineCore.h"
#include "Scene.h"
#include <type_traits>
#include <unordered_map>
#include <utility>

namespace gm
{
	class SceneManager
	{
	public:
		SceneManager();
		~SceneManager();

		template <typename T>
		Scene*			CreateScene(const std::wstring& sceneName)
		{
			static_assert(std::is_base_of_v<Scene, T>, "T는 반드시 Scene의 자식 클래스이어야 합니다.");
			
			auto iter = _sceneList.find(sceneName);
			if (iter != _sceneList.end())
				return nullptr;

			std::unique_ptr<T> newScene = std::make_unique<T>();
			newScene->SetName(sceneName);

			Scene* scenePtr = newScene.get();
			_sceneList.insert({ sceneName, std::move(newScene) });
			return scenePtr;
		}

		void				Update();
		void				LateUpdate();
		void				Render();
		void				EndFrame();

		void				RequestSceneChange(const std::wstring& pendingSceneName, const std::wstring& loadingSceneName = L"");
		Scene*				GetActiveScene() { return _activeScene; }
		const Scene*		GetActiveScene() const { return _activeScene; }
		const std::wstring& GetPendingSceneName() { return _pendingSceneName; }

	private:
		void				CheckSceneChange();

	private:
		std::unordered_map<std::wstring, std::unique_ptr<Scene>>	_sceneList;
		Scene*														_activeScene = nullptr;
		std::wstring												_pendingSceneName{};
		std::wstring												_nextSceneName{};
	};
}


