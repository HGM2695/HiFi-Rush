#pragma once

#include "../Engine/SceneManager.h"
#include "../Engine/Application.h"
#include "MainScene.h"
#include "TitleScene.h"
#include "CommonLoadingScene.h"

namespace gm
{
	void SetupScenes()
	{
		SceneManager& sceneManager = APPLICATION.GetSceneManager();
		sceneManager.CreateScene<TitleScene>(L"TitleScene");
		sceneManager.CreateScene<MainScene>(L"MainScene");
		sceneManager.CreateScene<CommonLoadingScene>(L"CommonLoadingScene");

		sceneManager.RequestSceneChange(L"TitleScene", L"CommonLoadingScene");
	}
}