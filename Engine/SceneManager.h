#pragma once

#include <map>
#include <memory>
#include <string>

struct HDC__;
typedef struct HDC__* HDC;

namespace gm
{
	class Scene;

	class SceneManager
	{
	public:
		SceneManager();
		~SceneManager();

		template <typename T>
		Scene*			CreateScene(const std::wstring& sceneName)
		{
			std::unique_ptr<T> newScene = std::make_unique<T>();
			newScene->SetName(sceneName);
			newScene->Initialize();

			Scene* scenePtr = newScene.get();
			_sceneList.insert({ sceneName, std::move(newScene) });
			return scenePtr;
		}

		void			PlayScene(const std::wstring& sceneName);
		Scene*			GetActiveScene() { return _activeScene; }
		const Scene*	GetActiveScene() const { return _activeScene; }

	public:
		void			Initialize();
		void			Update();
		void			LateUpdate();
		void			Render(HDC hDC);
		void			EndFrame();

	private:
		std::map<const std::wstring, std::unique_ptr<Scene>>	_sceneList;
		Scene*													_activeScene = nullptr;
	};
}


