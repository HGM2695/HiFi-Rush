#pragma once

#include <map>
#include <memory>
#include <string>

struct HDC__;
typedef struct HDC__* HDC;

namespace gm
{
	class PhysicsSystem2D;
	class Scene;

	enum class PhysicsMode
	{
		None,
		Physics2D,
		Physics3D,
	};

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
		void			SetPhysicsMode(PhysicsMode physicsMode) { _physicsMode = physicsMode; }
		PhysicsMode		GetPhysicsMode() const { return _physicsMode; }

	public:
		void			Initialize();
		void			Update();
		void			PhysicsUpdate();
		void			LateUpdate();
		void			Render(HDC hDC);
		void			EndFrame();

	private:
		std::map<const std::wstring, std::unique_ptr<Scene>>	_sceneList;
		Scene*													_activeScene;
		std::unique_ptr<PhysicsSystem2D>						_physicsSystem2D;
		PhysicsMode												_physicsMode = PhysicsMode::None;
	};
}


