#pragma once

#include <map>
#include <memory>
#include <string>
#include "Scene.h"

struct HDC__;
typedef struct HDC__* HDC;

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
			std::unique_ptr<T> newScene = std::make_unique<T>();
			newScene->SetName(sceneName);
			newScene->Initialize();

			Scene* scenePtr = newScene.get();
			_sceneList.insert({ sceneName, std::move(newScene) });
			return scenePtr;
		}

		void			Initialize();
		void			Update();
		void			LateUpdate();
		void			Render(HDC hDC);

	private:
		std::map<const std::wstring, std::unique_ptr<Scene>> _sceneList;
		Scene* _activeScene;
	};
}


