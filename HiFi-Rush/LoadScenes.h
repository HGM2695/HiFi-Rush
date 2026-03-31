#pragma once

#include "../Engine/SceneManager.h"
#include "../Engine/Application.h"
#include "MainScene.h"
#include "TitleScene.h"

namespace gm
{
	void SetupScenes()
	{
		SceneManager& sceneManager = APPLICATION.GetSceneManager();
		sceneManager.CreateScene<TitleScene>(L"TitleScene");
		sceneManager.CreateScene<MainScene>(L"MainScene");

		sceneManager.RequestSceneChange(L"TitleScene");
	}
}